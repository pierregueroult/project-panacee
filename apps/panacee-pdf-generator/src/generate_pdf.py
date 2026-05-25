from pathlib import Path
from reportlab.lib.pagesizes import A4
from reportlab.pdfgen import canvas
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.units import mm, cm
from reportlab.platypus import Flowable

from load_data import *

# COULEURS
COLOR_GREEN  = colors.green        # Nb hôpitaux
COLOR_ORANGE = colors.orange       # Total lits
COLOR_RED    = colors.red          # Population
COLOR_BLUE   = colors.blue         # Lits/1000
COLOR_DARK   = colors.darkblue     # Header tableau
COLOR_LIGHT  = colors.lightgrey    # Ligne paire
COLOR_WHITE  = colors.white
COLOR_BORDER = colors.grey
# COLOR_GREEN  = colors.HexColor("#27AE60")   # Nb hôpitaux
# COLOR_ORANGE = colors.HexColor("#E67E22")   # Total lits
# COLOR_RED    = colors.HexColor("#C0392B")   # Population
# COLOR_BLUE   = colors.HexColor("#2980B9")   # Lits/1000
# COLOR_DARK   = colors.HexColor("#2C3E50")   # Header tableau
# COLOR_LIGHT  = colors.HexColor("#ECF0F1")   # Ligne paire
# COLOR_WHITE  = colors.white
# COLOR_BORDER = colors.HexColor("#BDC3C7")


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
    def __init__(self, hospital_count, total_beds, total_population, beds_per_1000, page_width):
        super().__init__()
        self.hospital_count   = hospital_count
        self.total_beds       = total_beds
        self.total_population = total_population
        self.beds_per_1000    = beds_per_1000
        self.page_width       = page_width
        self.card_h = 2.2 * cm
        self.gap    = 0.3 * cm

    def wrap(self, *args):
        return self.page_width, self.card_h + 0.2 * cm

    def draw(self):
        c = self.canv
        n = 4
        card_w = (self.page_width - (n - 1) * self.gap) / n

        cards = [
            (COLOR_GREEN,  "Hopitaux",        str(self.hospital_count)),
            (COLOR_ORANGE, "Total lits",      f"{self.total_beds:,}".replace(",", " ")),
            (COLOR_RED,    "Population",      f"{self.total_population:,}".replace(",", " ")),
            (COLOR_BLUE,   "Lits / 1 000 hab",str(self.beds_per_1000)),
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



# TABLEAU VILLES

def build_city_table(department_hospitals):
    """Retourne un objet Table ReportLab avec les villes et leurs lits."""
    table_data = [["Ville", "Nombre de lits"]]

    displayed = (
        department_hospitals[["city_name", "beds_count"]]
        .dropna()
        .sort_values("beds_count", ascending=False)
    )

    if displayed.empty:
        table_data.append(["Aucun hôpital", "-"])
    else:
        for _, row in displayed.iterrows():
            table_data.append([row["city_name"], int(row["beds_count"])])

    col_widths = [9 * cm, 5 * cm]
    table = Table(table_data, colWidths=col_widths)

    # Style du tableau
    n_rows = len(table_data)
    style = [
        # ── Header ──
        ("BACKGROUND",   (0, 0), (-1, 0),  COLOR_DARK),
        ("TEXTCOLOR",    (0, 0), (-1, 0),  COLOR_WHITE),
        ("FONTNAME",     (0, 0), (-1, 0),  "Helvetica-Bold"),
        ("FONTSIZE",     (0, 0), (-1, 0),  10),
        ("ALIGN",        (0, 0), (-1, 0),  "CENTER"),
        ("BOTTOMPADDING",(0, 0), (-1, 0),  8),
        ("TOPPADDING",   (0, 0), (-1, 0),  8),
        # ── Lignes de données ──
        ("FONTNAME",     (0, 1), (-1, -1), "Helvetica"),
        ("FONTSIZE",     (0, 1), (-1, -1), 9),
        ("ALIGN",        (1, 1), (1, -1),  "CENTER"),
        ("TOPPADDING",   (0, 1), (-1, -1), 5),
        ("BOTTOMPADDING",(0, 1), (-1, -1), 5),
        # ── Grille ──
        ("GRID",         (0, 0), (-1, -1), 0.5, COLOR_BORDER),
        ("ROWBACKGROUNDS",(0, 1), (-1, -1), [COLOR_WHITE, COLOR_LIGHT]),
    ]

    table.setStyle(TableStyle(style))
    return table



# LES ÉLÉMENTS DE LA PAGE

def build_department_elements(department_code, department_hospitals,
                               department_towns, department_stats,
                               styles, page_width):
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

    # TITRE 
    # Récupère le nom du département
    if not department_hospitals.empty:
        dept_name = department_hospitals["department_name"].dropna().iloc[0]
    else:
        dept_name = department_towns["department_name"].dropna().iloc[0]

    title_style = ParagraphStyle(
        "DeptTitle",
        fontName="Helvetica-Bold",
        fontSize=26,
        textColor=COLOR_DARK,
        spaceAfter=4,
    )
    elements.append(Paragraph(f"Département {department_code} - {dept_name}", title_style))
    elements.append(Spacer(1, 0.3 * cm))

    # LIGNE DE SÉPARATION
    elements.append(HRule(page_width, thickness=2, color=COLOR_DARK))
    elements.append(Spacer(1, 0.4 * cm))

    # STATISTIQUES 
    stats = department_stats.iloc[0]
    elements.append(StatCards(
        hospital_count   = int(stats["hospital_count"]),
        total_beds       = int(stats["total_beds"]),
        total_population = int(stats["total_population"]),
        beds_per_1000    = stats["beds_per_1000"],
        page_width       = page_width,
    ))
    elements.append(Spacer(1, 0.6 * cm))

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
    elements.append(build_city_table(department_hospitals))

    return elements



# GÉNÉRATION : UN SEUL DÉPARTEMENT

def generate_department_pdf(department_code: str) -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    department_towns, department_hospitals, department_stats = get_department_data(department_code)

    if department_stats.empty:
        raise ValueError(f"Aucune donnée trouvée pour le département {department_code}")

    pdf_path = OUTPUT_DIR / f"department_{department_code}.pdf"

    styles = getSampleStyleSheet()
    doc = SimpleDocTemplate(
        str(pdf_path),
        pagesize=A4,
        leftMargin=1.5 * cm,
        rightMargin=1.5 * cm,
        topMargin=1.5 * cm,
        bottomMargin=1.5 * cm,
    )

    page_width = A4[0] - 3 * cm   # largeur utile (marges gauche + droite = 3 cm)

    elements = build_department_elements(
        department_code, department_hospitals,
        department_towns, department_stats,
        styles, page_width
    )

    doc.build(elements)
    return pdf_path



# GÉNÉRATION : TOUS LES DÉPARTEMENTS EN UN PDF

def generate_all_departments_in_one_pdf() -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    towns, _, _ = load_all_data()

    department_codes = sorted(
        towns["department_code"].dropna().astype(str).unique(),
        key=lambda x: int(x)
    )

    pdf_path = OUTPUT_DIR / "all_departments.pdf"

    styles = getSampleStyleSheet()
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
        department_towns, department_hospitals, department_stats = get_department_data(department_code)

        if department_stats.empty:
            continue

        dept_elements = build_department_elements(
            department_code, department_hospitals,
            department_towns, department_stats,
            styles, page_width
        )
        elements.extend(dept_elements)

        # Saut de page entre chaque département (sauf le dernier)
        if index < len(department_codes) - 1:
            elements.append(PageBreak())

    doc.build(elements)
    return pdf_path