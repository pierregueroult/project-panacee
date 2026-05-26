import math
from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.units import cm
from reportlab.platypus import (
    Flowable,
    PageBreak,
    Paragraph,
    SimpleDocTemplate,
    Spacer,
    Table,
    TableStyle,
)

from load_data import (
    OUTPUT_DIR,
    department_summary,
    get_department_data,
    load_communes_coords,
    load_department_borders,
    load_hospitals,
    load_towns_status,
)

COLOR_GREEN = colors.green
COLOR_ORANGE = colors.orange
COLOR_RED = colors.red
COLOR_BLUE = colors.blue
COLOR_DARK = colors.darkblue
COLOR_LIGHT = colors.lightgrey
COLOR_WHITE = colors.white
COLOR_BORDER = colors.grey


# LIGNE DE SÉPARATION
class HRule(Flowable):
    """Trait horizontal de séparation."""

    def __init__(self, width, thickness=1, color=COLOR_BORDER):
        super().__init__()
        self.width = width
        self.thickness = thickness
        self.color = color

    def draw(self):
        self.canv.setStrokeColor(self.color)
        self.canv.setLineWidth(self.thickness)
        self.canv.line(0, 0, self.width, 0)

    def wrap(self, *args):
        return self.width, self.thickness + 2


# STATISTIQUES


class StatCards(Flowable):
    """
    4 cartes colorées sur une ligne :
    [Nb hôpitaux | Total lits | Population | Lits/1000]
    """

    def __init__(self, hospital_count, total_beds, total_population, beds_per_1000):
        super().__init__()
        self.hospital_count = hospital_count
        self.total_beds = total_beds
        self.total_population = total_population
        self.beds_per_1000 = beds_per_1000
        self.card_h = 2.2 * cm
        self.gap = 0.3 * cm
        self.width = 0

    def wrap(self, avail_width, avail_height):
        self.width = avail_width
        return self.width, self.card_h + 0.2 * cm

    def draw(self):
        c = self.canv
        n = 4
        card_w = (self.width - (n - 1) * self.gap) / n

        cards = [
            (COLOR_GREEN, "Hopitaux", str(self.hospital_count)),
            (COLOR_ORANGE, "Total lits", f"{self.total_beds:,}".replace(",", " ")),
            (COLOR_RED, "Population", f"{self.total_population:,}".replace(",", " ")),
            (COLOR_BLUE, "Lits / 1 000 hab", str(self.beds_per_1000)),
        ]

        for i, (color, label, value) in enumerate(cards):
            x = i * (card_w + self.gap)
            y = 0

            # Fond de la carte
            c.setFillColor(color)
            c.roundRect(x, y, card_w, self.card_h, 6, fill=1, stroke=0)

            # Valeur (grande, blanche, en haut)
            c.setFillColor(COLOR_WHITE)
            c.setFont("Helvetica-Bold", 16)
            c.drawCentredString(x + card_w / 2, y + self.card_h * 0.52, value)

            # Label (petit, blanc, en bas)
            c.setFont("Helvetica", 8)
            c.drawCentredString(x + card_w / 2, y + self.card_h * 0.18, label)


# MINI-CARTE DÉPARTEMENT


class DepartmentMiniMap(Flowable):
    """
    Projection équirectangulaire des villes du département.
    Villes : petit point gris. Hôpitaux : disque rouge plus gros.
    """

    def __init__(self, towns_coords, hospitals_coords, width, border_rings=None,
                 height=6 * cm):
        super().__init__()
        self.towns = [
            (lat, lon)
            for lat, lon in towns_coords
            if lat is not None and lon is not None
        ]
        self.hospitals = [
            (lat, lon)
            for lat, lon in hospitals_coords
            if lat is not None and lon is not None
        ]
        self.border_rings = border_rings or []
        self.width = width
        self.height = height

    def wrap(self, avail_width, avail_height):
        self.width = avail_width
        return self.width, self.height

    def draw(self):
        c = self.canv

        c.setFillColor(COLOR_WHITE)
        c.setStrokeColor(COLOR_BORDER)
        c.setLineWidth(0.5)
        c.roundRect(0, 0, self.width, self.height, 4, fill=1, stroke=1)

        if not self.towns and not self.border_rings:
            return

        bbox_points = list(self.towns)
        for ring in self.border_rings:
            bbox_points.extend(ring)
        lats = [lat for lat, _ in bbox_points]
        lons = [lon for _, lon in bbox_points]
        lat_min, lat_max = min(lats), max(lats)
        lon_min, lon_max = min(lons), max(lons)

        pad_lat = (lat_max - lat_min) * 0.05 or 0.01
        pad_lon = (lon_max - lon_min) * 0.05 or 0.01
        lat_min -= pad_lat
        lat_max += pad_lat
        lon_min -= pad_lon
        lon_max += pad_lon

        mean_lat_rad = math.radians((lat_min + lat_max) / 2)
        cos_lat = math.cos(mean_lat_rad)

        margin = 0.4 * cm
        inner_w = self.width - 2 * margin
        inner_h = (
            self.height - 2 * margin - 0.5 * cm
        )  # garde de la place pour la légende

        span_x = (lon_max - lon_min) * cos_lat
        span_y = lat_max - lat_min
        if span_x <= 0 or span_y <= 0:
            return

        scale = min(inner_w / span_x, inner_h / span_y)
        draw_w = span_x * scale
        draw_h = span_y * scale
        offset_x = margin + (inner_w - draw_w) / 2
        offset_y = margin + 0.5 * cm + (inner_h - draw_h) / 2

        def project(lat, lon):
            x = offset_x + (lon - lon_min) * cos_lat * scale
            y = offset_y + (lat - lat_min) * scale
            return x, y

        # Contour du département
        if self.border_rings:
            c.setStrokeColor(COLOR_DARK)
            c.setFillColor(colors.HexColor("#f5f5f5"))
            c.setLineWidth(0.6)
            for ring in self.border_rings:
                if len(ring) < 2:
                    continue
                p = c.beginPath()
                lat0, lon0 = ring[0]
                x0, y0 = project(lat0, lon0)
                p.moveTo(x0, y0)
                for lat, lon in ring[1:]:
                    x, y = project(lat, lon)
                    p.lineTo(x, y)
                p.close()
                c.drawPath(p, stroke=1, fill=1)

        c.setFillColor(COLOR_BORDER)
        c.setStrokeColor(COLOR_BORDER)
        for lat, lon in self.towns:
            x, y = project(lat, lon)
            c.circle(x, y, 0.8, fill=1, stroke=0)

        c.setFillColor(COLOR_RED)
        c.setStrokeColor(colors.white)
        c.setLineWidth(0.3)
        for lat, lon in self.hospitals:
            x, y = project(lat, lon)
            c.circle(x, y, 2.0, fill=1, stroke=1)

        # Légende
        c.setFont("Helvetica", 7)
        c.setFillColor(COLOR_BORDER)
        c.circle(margin + 0.1 * cm, 0.25 * cm, 0.8, fill=1, stroke=0)
        c.setFillColor(colors.black)
        c.drawString(margin + 0.3 * cm, 0.18 * cm, "Commune")

        c.setFillColor(COLOR_RED)
        c.circle(margin + 2.2 * cm, 0.25 * cm, 2.0, fill=1, stroke=0)
        c.setFillColor(colors.black)
        c.drawString(margin + 2.4 * cm, 0.18 * cm, "Hôpital")


# TABLEAU VILLES


CITY_TABLE_SPLIT_THRESHOLD = 15


def _city_table_style():
    return TableStyle(
        [
            # ── Header ──
            ("BACKGROUND", (0, 0), (-1, 0), COLOR_DARK),
            ("TEXTCOLOR", (0, 0), (-1, 0), COLOR_WHITE),
            ("FONTNAME", (0, 0), (-1, 0), "Helvetica-Bold"),
            ("FONTSIZE", (0, 0), (-1, 0), 10),
            ("ALIGN", (0, 0), (-1, 0), "CENTER"),
            ("BOTTOMPADDING", (0, 0), (-1, 0), 8),
            ("TOPPADDING", (0, 0), (-1, 0), 8),
            # ── Lignes de données ──
            ("FONTNAME", (0, 1), (-1, -1), "Helvetica"),
            ("FONTSIZE", (0, 1), (-1, -1), 9),
            ("ALIGN", (1, 1), (1, -1), "CENTER"),
            ("TOPPADDING", (0, 1), (-1, -1), 5),
            ("BOTTOMPADDING", (0, 1), (-1, -1), 5),
            # ── Grille ──
            ("GRID", (0, 0), (-1, -1), 0.5, COLOR_BORDER),
            ("ROWBACKGROUNDS", (0, 1), (-1, -1), [COLOR_WHITE, COLOR_LIGHT]),
        ]
    )


def _make_city_subtable(rows, col_widths):
    data = [["Ville", "Nombre de lits"]] + rows
    table = Table(data, colWidths=col_widths, repeatRows=1)
    table.setStyle(_city_table_style())
    return table


def build_city_table(department_hospitals, page_width):
    """Retourne un Flowable : tableau simple, ou 2 sous-tableaux côte à côte
    si la liste est longue (pour éviter le débordement sur la page suivante)."""
    displayed = (
        department_hospitals[["name", "beds_count"]]
        .dropna()
        .sort_values("beds_count", ascending=False)
    )

    if displayed.empty:
        return _make_city_subtable([["Aucun hôpital", "-"]], [9 * cm, 5 * cm])

    rows = [[r["name"], int(r["beds_count"])] for _, r in displayed.iterrows()]

    if len(rows) <= CITY_TABLE_SPLIT_THRESHOLD:
        return _make_city_subtable(rows, [9 * cm, 5 * cm])

    # Split en 2 colonnes côte à côte
    gap = 0.4 * cm
    half_w = (page_width - gap) / 2
    name_w = half_w * 0.65
    beds_w = half_w - name_w

    mid = (len(rows) + 1) // 2
    left = _make_city_subtable(rows[:mid], [name_w, beds_w])
    right = _make_city_subtable(rows[mid:], [name_w, beds_w])

    outer = Table(
        [[left, right]],
        colWidths=[half_w, half_w],
        style=TableStyle(
            [
                ("VALIGN", (0, 0), (-1, -1), "TOP"),
                ("LEFTPADDING", (0, 0), (-1, -1), 0),
                ("RIGHTPADDING", (0, 0), (0, 0), gap),
                ("RIGHTPADDING", (1, 0), (1, 0), 0),
                ("TOPPADDING", (0, 0), (-1, -1), 0),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 0),
            ]
        ),
    )
    return outer


# LES ÉLÉMENTS DE LA PAGE


def build_department_elements(
    department_code,
    department_hospitals,
    department_towns,
    department_stats,
    page_width,
    border_rings=None,
):
    """
    Retourne la liste des Flowables pour un département.
    Étapes :
      1. Titre  (numéro + nom du département)
      2. Ligne de séparation
      3. 4 cartes de statistiques
      4. Sous-titre "Villes avec hôpital"
      5. Tableau des villes
    """
    elements = []

    hospitals_names = department_hospitals["department_name"].dropna()
    towns_names = department_towns["department_name"].dropna()
    if not hospitals_names.empty:
        dept_name = hospitals_names.iloc[0]
    elif not towns_names.empty:
        dept_name = towns_names.iloc[0]
    else:
        dept_name = department_code

    title_style = ParagraphStyle(
        "DeptTitle",
        fontName="Helvetica-Bold",
        fontSize=26,
        textColor=COLOR_DARK,
        spaceAfter=4,
    )
    elements.append(
        Paragraph(f"Département {department_code} - {dept_name}", title_style)
    )
    elements.append(Spacer(1, 0.3 * cm))

    # LIGNE DE SÉPARATION
    elements.append(HRule(page_width, thickness=2, color=COLOR_DARK))
    elements.append(Spacer(1, 0.4 * cm))

    # STATISTIQUES
    stats = department_stats.iloc[0]
    elements.append(
        StatCards(
            hospital_count=int(stats["hospital_count"]),
            total_beds=int(stats["total_beds"]),
            total_population=int(stats["total_population"]),
            beds_per_1000=stats["beds_per_1000"],
        )
    )
    elements.append(Spacer(1, 0.5 * cm))

    # MINI-CARTE
    if "lat" in department_towns.columns and "lon" in department_towns.columns:
        towns_coords = list(
            zip(department_towns["lat"].tolist(), department_towns["lon"].tolist())
        )
        hospitals_coords = list(
            zip(
                department_hospitals["lat"].tolist(),
                department_hospitals["lon"].tolist(),
            )
        )
        elements.append(
            DepartmentMiniMap(
                towns_coords, hospitals_coords, page_width,
                border_rings=border_rings,
            )
        )
        elements.append(Spacer(1, 0.5 * cm))

    # SOUS-TITRE TABLEAU
    subtitle_style = ParagraphStyle(
        "SubTitle",
        fontName="Helvetica-Bold",
        fontSize=13,
        textColor=COLOR_DARK,
        spaceAfter=6,
    )
    elements.append(Paragraph("Villes avec hôpital", subtitle_style))

    # TABLEAU DES VILLES
    elements.append(build_city_table(department_hospitals, page_width))

    return elements


# GÉNÉRATION : UN SEUL DÉPARTEMENT


def generate_department_pdf(department_code: str) -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    hospitals = load_hospitals()
    towns_status = load_towns_status()
    summary = department_summary(hospitals, towns_status)
    coords = load_communes_coords()
    borders = load_department_borders()

    department_towns, department_hospitals, department_stats = get_department_data(
        department_code, hospitals, towns_status, summary, coords
    )

    if department_stats.empty:
        raise ValueError(f"Aucune donnée trouvée pour le département {department_code}")

    pdf_path = OUTPUT_DIR / f"department_{department_code}.pdf"

    doc = SimpleDocTemplate(
        str(pdf_path),
        pagesize=A4,
        leftMargin=1.5 * cm,
        rightMargin=1.5 * cm,
        topMargin=1.5 * cm,
        bottomMargin=1.5 * cm,
    )

    page_width = A4[0] - 3 * cm

    elements = build_department_elements(
        department_code,
        department_hospitals,
        department_towns,
        department_stats,
        page_width,
        border_rings=borders.get(department_code.zfill(2)),
    )

    doc.build(elements)
    return pdf_path


# GÉNÉRATION : TOUS LES DÉPARTEMENTS EN UN PDF


def generate_all_departments_in_one_pdf() -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    hospitals = load_hospitals()
    towns_status = load_towns_status()
    summary = department_summary(hospitals, towns_status)
    coords = load_communes_coords()
    borders = load_department_borders()

    department_codes = sorted(
        towns_status["department_code"].dropna().astype(str).unique(),
        key=lambda x: int(x),
    )

    pdf_path = OUTPUT_DIR / "all_departments.pdf"

    doc = SimpleDocTemplate(
        str(pdf_path),
        pagesize=A4,
        leftMargin=1.5 * cm,
        rightMargin=1.5 * cm,
        topMargin=1.5 * cm,
        bottomMargin=1.5 * cm,
    )

    page_width = A4[0] - 3 * cm

    elements = []

    for index, department_code in enumerate(department_codes):
        department_towns, department_hospitals, department_stats = get_department_data(
            department_code, hospitals, towns_status, summary, coords
        )

        if department_stats.empty:
            continue

        dept_elements = build_department_elements(
            department_code,
            department_hospitals,
            department_towns,
            department_stats,
            page_width,
            border_rings=borders.get(department_code.zfill(2)),
        )
        elements.extend(dept_elements)

        if index < len(department_codes) - 1:
            elements.append(PageBreak())

    doc.build(elements)
    return pdf_path
