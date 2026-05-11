// CSV paths served from the project root by JExpress.java
const PATHS = {
  communes:  '/data/communes-france-metrople-2025.csv',
  hospitals: '/apps/panacee-genetics/src/results/hospitals.csv',
  fitness:   '/apps/panacee-genetics/src/results/fitness.csv',
};

// Color thresholds based on bed count
const COLOR_SCALE = [
  { max: 50,       color: '#60a5fa', label: '< 50 lits' },
  { max: 150,      color: '#34d399', label: '50 – 150 lits' },
  { max: 400,      color: '#fbbf24', label: '150 – 400 lits' },
  { max: 1000,     color: '#f97316', label: '400 – 1 000 lits' },
  { max: Infinity, color: '#ef4444', label: '> 1 000 lits (CHU)' },
];

// Hospital coverage radius in metres
const COVERAGE_RADIUS = 10000;

// Global references used across functions
let map;
let hospitalGroup;


// =============================================================================
// DATA LOADING
// =============================================================================

function loadCSV(url, hasHeader) {
  return new Promise(function (resolve, reject) {
    Papa.parse(url, {
      download: true,
      header: hasHeader,
      dynamicTyping: true,
      skipEmptyLines: true,
      complete: function (results) {
        resolve(results.data);
      },
      error: function (err) {
        reject(new Error('Erreur chargement CSV (' + url + ') : ' + err.message));
      },
    });
  });
}

// Builds a lookup map: insee code → commune data, from the communes CSV
// Columns: insee, name, regionCode, region, deptCode, dept, postalCode, population, lat, lng
function buildCommuneMap(rows) {
  const communeMap = new Map();

  for (const row of rows) {
    const insee = String(row[0]);
    communeMap.set(insee, {
      name:     row[1],
      region:   row[3],
      deptCode: String(row[4]).padStart(2, '0'),
      dept:     row[5],
      pop:      row[7],
      lat:      row[8],
      lng:      row[9],
    });
  }

  return communeMap;
}

// Joins hospitals with commune data to get GPS coordinates,
// and separately returns communes that have no hospital.
function processData(hospitalsRaw, communeMap) {
  const hospitalInseeSet = new Set();
  const hospitals = [];

  for (const row of hospitalsRaw) {
    const insee = String(row.insee);
    const commune = communeMap.get(insee);

    if (commune === undefined || !commune.lat || !commune.lng) {
      continue;
    }

    hospitalInseeSet.add(insee);
    hospitals.push({
      insee:    insee,
      beds:     row.beds_count,
      name:     commune.name,
      dept:     commune.dept,
      deptCode: commune.deptCode,
      region:   commune.region,
      pop:      commune.pop,
      lat:      commune.lat,
      lng:      commune.lng,
    });
  }

  // Communes without a hospital, stored as [lat, lng] pairs to save memory
  const communes = [];
  for (const [insee, commune] of communeMap) {
    if (!hospitalInseeSet.has(insee) && commune.lat && commune.lng) {
      communes.push([commune.lat, commune.lng]);
    }
  }

  return { hospitals, communes };
}


// =============================================================================
// SIDEBAR RENDERING
// =============================================================================

function getColor(beds) {
  for (const entry of COLOR_SCALE) {
    if (beds < entry.max) {
      return entry.color;
    }
  }
  return COLOR_SCALE[COLOR_SCALE.length - 1].color;
}

function formatNumber(n) {
  return Number(n).toLocaleString('fr-FR');
}

function renderStats(fitness, hospitals) {
  const totalBeds = hospitals.reduce((sum, h) => sum + h.beds, 0);

  const cards = [
    {
      icon:  '🏥',
      value: formatNumber(fitness.hospital_count),
      label: 'Hôpitaux placés',
      sub:   'dont ' + fitness.uhc_count + ' CHU',
    },
    {
      icon:  '🛏',
      value: formatNumber(totalBeds),
      label: 'Lits totaux',
      sub:   null,
    },
    {
      icon:  '📍',
      value: Number(fitness.distant_resident_percent).toFixed(2) + ' %',
      label: 'Résidents éloignés',
      sub:   formatNumber(fitness.distant_resident_count) + ' pers.',
    },
    {
      icon:  '🏘',
      value: Number(fitness.distant_town_percent).toFixed(2) + ' %',
      label: 'Communes éloignées',
      sub:   formatNumber(fitness.distant_town_count) + ' communes',
    },
  ];

  let html = '';
  for (const card of cards) {
    const subHtml = card.sub !== null ? `<div class="stat-sub">${card.sub}</div>` : '';
    html += `
      <div class="stat-card">
        <span class="stat-icon">${card.icon}</span>
        <div>
          <div class="stat-value">${card.value}</div>
          <div class="stat-label">${card.label}</div>
          ${subHtml}
        </div>
      </div>`;
  }

  document.getElementById('stats-grid').innerHTML = html;
}

function renderLegend() {
  let html = '';
  for (const entry of COLOR_SCALE) {
    html += `
      <div class="legend-row">
        <span class="legend-dot" style="background: ${entry.color}"></span>
        <span>${entry.label}</span>
      </div>`;
  }
  document.getElementById('legend').innerHTML = html;
}

function populateDeptFilter(hospitals) {
  // Collect unique departments present in the results
  const deptMap = new Map();
  for (const hospital of hospitals) {
    deptMap.set(hospital.deptCode, hospital.dept);
  }

  // Sort by department code
  const depts = Array.from(deptMap.entries());
  depts.sort((a, b) => a[0].localeCompare(b[0]));

  const select = document.getElementById('dept-filter');
  for (const [code, name] of depts) {
    const option = document.createElement('option');
    option.value = code;
    option.textContent = code + ' — ' + name;
    select.appendChild(option);
  }
}

function showDetail(hospital) {
  const populationText = hospital.pop ? formatNumber(hospital.pop) : '—';

  document.getElementById('detail-content').innerHTML = `
    <div class="detail-name">${hospital.name}</div>
    <div class="detail-loc">${hospital.dept} (${hospital.deptCode}) · ${hospital.region}</div>
    <div class="detail-row"><span>Lits</span><strong>${formatNumber(hospital.beds)}</strong></div>
    <div class="detail-row"><span>Population</span><strong>${populationText}</strong></div>
    <div class="detail-row"><span>Code INSEE</span><strong>${hospital.insee}</strong></div>
  `;
  document.getElementById('detail-section').style.display = '';
}


// =============================================================================
// LEAFLET MAP
// =============================================================================

function initMap(communes) {
  map = L.map('map', {
    center: [46.5, 2.3],
    zoom: 6,
    zoomControl: false,
  });

  L.tileLayer('https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png', {
    attribution: '© <a href="https://openstreetmap.org/copyright">OpenStreetMap</a> contributors © <a href="https://carto.com/attributions">CARTO</a>',
    subdomains: 'abcd',
    maxZoom: 19,
  }).addTo(map);

  L.control.zoom({ position: 'topright' }).addTo(map);

  // Canvas renderer is required to display ~33 000 points without slowing the browser
  const canvasRenderer = L.canvas({ padding: 0.5 });
  const communeLayer = L.layerGroup();

  for (const [lat, lng] of communes) {
    L.circleMarker([lat, lng], {
      renderer:    canvasRenderer,
      radius:      2,
      fillColor:   '#94a3b8',
      color:       'transparent',
      fillOpacity: 0.45,
      interactive: false,
    }).addTo(communeLayer);
  }

  communeLayer.addTo(map);

  // Hospital group will be populated by refreshHospitals()
  hospitalGroup = L.layerGroup().addTo(map);
}

function refreshHospitals(hospitals) {
  hospitalGroup.clearLayers();

  for (const hospital of hospitals) {
    const color = getColor(hospital.beds);

    const popupContent = `
      <div class="popup">
        <h3>${hospital.name}</h3>
        <div class="popup-dept">${hospital.dept} (${hospital.deptCode}) · ${hospital.region}</div>
        <div class="popup-row"><span>Lits :</span><strong>${formatNumber(hospital.beds)}</strong></div>
        <div class="popup-row"><span>Population :</span><strong>${hospital.pop ? formatNumber(hospital.pop) : '—'}</strong></div>
        <div class="popup-row"><span>INSEE :</span><code>${hospital.insee}</code></div>
      </div>`;

    // Geographic circle representing the 10 km coverage area
    L.circle([hospital.lat, hospital.lng], {
      radius:      COVERAGE_RADIUS,
      fillColor:   color,
      fillOpacity: 0.10,
      color:       color,
      weight:      1.2,
      opacity:     0.5,
    })
    .bindPopup(popupContent, { maxWidth: 240 })
    .on('click', () => showDetail(hospital))
    .addTo(hospitalGroup);

    // Center dot to precisely locate the commune
    L.circleMarker([hospital.lat, hospital.lng], {
      radius:      4,
      fillColor:   color,
      color:       'rgba(255,255,255,0.7)',
      weight:      1,
      fillOpacity: 1,
      interactive: false,
    }).addTo(hospitalGroup);
  }

  const label = hospitals.length > 1 ? 'hôpitaux' : 'hôpital';
  document.getElementById('hospital-count').textContent =
    formatNumber(hospitals.length) + ' ' + label;
}


// =============================================================================
// FILTERS
// =============================================================================

function wireFilters(allHospitals) {
  const deptSelect  = document.getElementById('dept-filter');
  const bedsSlider  = document.getElementById('beds-min');
  const bedsLabel   = document.getElementById('beds-min-val');
  const resetButton = document.getElementById('reset-btn');

  let selectedDept = '';
  let minBeds = 0;

  function applyFilters() {
    const filtered = allHospitals.filter(function (hospital) {
      const matchesDept = selectedDept === '' || hospital.deptCode === selectedDept;
      const matchesBeds = hospital.beds >= minBeds;
      return matchesDept && matchesBeds;
    });

    refreshHospitals(filtered);

    // Zoom the map to the selected department
    if (selectedDept !== '' && filtered.length > 0) {
      const coords = filtered.map(h => [h.lat, h.lng]);
      map.fitBounds(L.latLngBounds(coords), { padding: [60, 60] });
    }
  }

  deptSelect.addEventListener('change', function () {
    selectedDept = deptSelect.value;
    applyFilters();
  });

  bedsSlider.addEventListener('input', function () {
    minBeds = Number(bedsSlider.value);
    bedsLabel.textContent = minBeds === 0 ? '0' : minBeds + '+';
    applyFilters();
  });

  resetButton.addEventListener('click', function () {
    selectedDept = '';
    minBeds = 0;
    deptSelect.value = '';
    bedsSlider.value = 0;
    bedsLabel.textContent = '0';
    document.getElementById('detail-section').style.display = 'none';
    map.setView([46.5, 2.3], 6);
    applyFilters();
  });
}


// =============================================================================
// STARTUP
// =============================================================================

async function main() {
  try {
    const [communesRaw, hospitalsRaw, fitnessRaw] = await Promise.all([
      loadCSV(PATHS.communes,  false),
      loadCSV(PATHS.hospitals, true),
      loadCSV(PATHS.fitness,   true),
    ]);

    const communeMap = buildCommuneMap(communesRaw);
    const { hospitals, communes } = processData(hospitalsRaw, communeMap);
    const fitness = fitnessRaw[0];

    renderStats(fitness, hospitals);
    renderLegend();
    populateDeptFilter(hospitals);
    initMap(communes);
    refreshHospitals(hospitals);
    wireFilters(hospitals);

    document.getElementById('loading').style.display = 'none';
    document.getElementById('app').classList.remove('hidden');

  } catch (error) {
    console.error(error);
  }
}

main();
