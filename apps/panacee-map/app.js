// Paths are served as static files by JExpress from the project root.
// All three CSVs are written by the genetic algorithm before the map is opened.
const PATHS = {
  communes:  '/data/communes-france-metrople-2025.csv',
  hospitals: '/apps/panacee-genetics/src/results/hospitals.csv',
  fitness:   '/apps/panacee-genetics/src/results/fitness.csv',
};

// Bed-count thresholds that drive both marker colour and the sidebar legend.
// Tiers map loosely to French healthcare categories:
//   < 50   → small clinic / maison de santé
//   < 150  → local hospital (hôpital de proximité)
//   < 400  → general hospital (centre hospitalier)
//   < 1000 → regional hospital (CHR)
//   ≥ 1000 → university hospital (CHU)
const COLOR_SCALE = [
  { max: 50,       color: '#60a5fa', label: '< 50 lits' },
  { max: 150,      color: '#34d399', label: '50 – 150 lits' },
  { max: 400,      color: '#fbbf24', label: '150 – 400 lits' },
  { max: 1000,     color: '#f97316', label: '400 – 1 000 lits' },
  { max: Infinity, color: '#ef4444', label: '> 1 000 lits (CHU)' },
];

// Maximum distance (metres) at which a town is considered covered by a hospital.
// Must match the value used by the genetic algorithm's fitness function.
const COVERAGE_RADIUS = 10000;

// Kept module-level so wireFilters can call refreshHospitals without threading
// these objects through every function signature.
let map;
let hospitalGroup;


// =============================================================================
// DATA LOADING
// =============================================================================

// Fetches and parses a remote CSV file via PapaParse.
// hasHeader controls whether the first row is consumed as named keys.
function loadCSV(url, hasHeader) {
  return new Promise(function (resolve, reject) {
    Papa.parse(url, {
      download:      true,
      header:        hasHeader,
      dynamicTyping: true,
      skipEmptyLines: true,
      complete: function (results) { resolve(results.data); },
      error:    function (err)     { reject(new Error('CSV load failed (' + url + '): ' + err.message)); },
    });
  });
}

// Column indices for communes-france-metrople-2025.csv (no header row).
const COL_INSEE     = 0;
const COL_NAME      = 1;
const COL_REGION    = 3;
const COL_DEPT_CODE = 4;
const COL_DEPT      = 5;
const COL_POP       = 7;
const COL_LAT       = 8;
const COL_LNG       = 9;

// Indexes communes by INSEE code for O(1) lookups when joining with hospitals.
function buildCommuneMap(rows) {
  const communeMap = new Map();

  for (const row of rows) {
    communeMap.set(String(row[COL_INSEE]), {
      name:     row[COL_NAME],
      region:   row[COL_REGION],
      deptCode: String(row[COL_DEPT_CODE]).padStart(2, '0'),
      dept:     row[COL_DEPT],
      pop:      row[COL_POP],
      lat:      row[COL_LAT],
      lng:      row[COL_LNG],
    });
  }

  return communeMap;
}

// Joins the hospitals CSV with commune data to obtain GPS coordinates.
// Returns two separate lists:
//   hospitals — full objects used for markers, popups and filters
//   communes  — bare [lat, lng] pairs for background dots; the full commune
//               object is not needed so storing only coordinates saves memory
function processData(hospitalsRaw, communeMap) {
  const hospitalInseeSet = new Set();
  const hospitals = [];

  for (const row of hospitalsRaw) {
    const insee   = String(row.insee);
    const commune = communeMap.get(insee);

    // Skip rows whose INSEE code has no matching commune or no GPS coordinates.
    if (commune === undefined || !commune.lat || !commune.lng) continue;

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
    if (beds < entry.max) return entry.color;
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
  // Collect the unique departments that actually appear in the result set
  // so the dropdown only lists departments with at least one hospital.
  const deptMap = new Map();
  for (const hospital of hospitals) {
    deptMap.set(hospital.deptCode, hospital.dept);
  }

  const depts = Array.from(deptMap.entries());
  depts.sort((a, b) => a[0].localeCompare(b[0]));

  const select = document.getElementById('dept-filter');
  for (const [code, name] of depts) {
    const option = document.createElement('option');
    option.value       = code;
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
    center:      [46.5, 2.3],
    zoom:        6,
    zoomControl: false,
  });

  L.tileLayer('https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png', {
    attribution: '© <a href="https://openstreetmap.org/copyright">OpenStreetMap</a> contributors © <a href="https://carto.com/attributions">CARTO</a>',
    subdomains:  'abcd',
    maxZoom:     19,
  }).addTo(map);

  L.control.zoom({ position: 'topright' }).addTo(map);

  // A shared Canvas renderer lets Leaflet draw ~33 000 commune dots in a single
  // <canvas> element. The SVG renderer (Leaflet default) creates one DOM node
  // per marker, which would freeze the browser at this scale.
  const canvasRenderer = L.canvas({ padding: 0.5 });
  const communeLayer   = L.layerGroup();

  for (const [lat, lng] of communes) {
    L.circleMarker([lat, lng], {
      renderer:    canvasRenderer,
      radius:      2,
      fillColor:   '#94a3b8',
      color:       'transparent',
      fillOpacity: 0.45,
      interactive: false,  // background context only — no hover or click needed
    }).addTo(communeLayer);
  }

  communeLayer.addTo(map);

  // Hospital group is populated by refreshHospitals() after initMap returns.
  hospitalGroup = L.layerGroup().addTo(map);
}

// Clears and redraws all hospital markers for the given subset.
// Called on startup with the full list and on every filter change.
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

    // Translucent circle visualises the 10 km coverage zone around each hospital.
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

    // Solid centre dot to pinpoint the commune precisely within its coverage circle.
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

// Wires the department dropdown and minimum-beds slider to refreshHospitals.
// Filter state lives in the closure so no module-level variables are needed.
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

    // Auto-zoom to the selected department when one is chosen,
    // but leave the national view untouched when showing all departments.
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
    minBeds      = 0;
    deptSelect.value  = '';
    bedsSlider.value  = 0;
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
    // Load all three CSVs in parallel — they are independent of each other.
    const [communesRaw, hospitalsRaw, fitnessRaw] = await Promise.all([
      loadCSV(PATHS.communes,  false),
      loadCSV(PATHS.hospitals, true),
      loadCSV(PATHS.fitness,   true),
    ]);

    const communeMap              = buildCommuneMap(communesRaw);
    const { hospitals, communes } = processData(hospitalsRaw, communeMap);
    const fitness                 = fitnessRaw[0];

    renderStats(fitness, hospitals);
    renderLegend();
    populateDeptFilter(hospitals);
    initMap(communes);
    refreshHospitals(hospitals);
    wireFilters(hospitals);

    document.getElementById('loading').style.display = 'none';
    document.getElementById('app').classList.remove('hidden');

    // Leaflet computes the container size at init time, but #app was display:none
    // so it got zero dimensions. Force a size recalculation now that it is visible.
    map.invalidateSize();

  } catch (error) {
    console.error(error);
  }
}

main();
