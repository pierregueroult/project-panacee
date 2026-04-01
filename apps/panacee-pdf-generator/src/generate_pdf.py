from pathlib import Path
from reportlab.lib.pagesizes import A4
from reportlab.pdfgen import canvas
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet

from load_data import *

OUTPUT_DIR = Path(__file__).resolve().parent.parent / "output"


def generate_department_pdf(department_code: str) -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    department_towns, department_hospitals, department_stats = get_department_data(department_code)

    if department_stats.empty:
        raise ValueError(f"Aucune donnée trouvée pour le département {department_code}")

    if not department_hospitals.empty:
        department_name = department_hospitals["department_name"].dropna().iloc[0]
    else:
        department_name = department_towns["department_name"].dropna().iloc[0]

    pdf_path = OUTPUT_DIR / f"department_{department_code}.pdf"

    styles = getSampleStyleSheet()
    doc = SimpleDocTemplate(str(pdf_path), pagesize=A4)

    elements = []

    title = Paragraph(f"<b>Département {department_code} - {department_name}</b>", styles["Title"])
    elements.append(title)
    elements.append(Spacer(1, 20))

    stats = department_stats.iloc[0]

    stats_text = f"""
    <b>Statistiques :</b><br/>
    Nombre d'hôpitaux : {stats['hospital_count']}<br/>
    Total de lits : {stats['total_beds']}<br/>
    Population totale : {stats['total_population']}<br/>
    Lits pour 1000 habitants : {stats['beds_per_1000']}
    """

    elements.append(Paragraph(stats_text, styles["Normal"]))
    elements.append(Spacer(1, 30))

    elements.append(Paragraph("<b>Villes avec hôpital</b>", styles["Heading2"]))
    elements.append(Spacer(1, 10))

    table_data = [["Ville", "Nombre de lits"]]

    displayed = (
        department_hospitals[["city_name", "beds_count"]]
        .dropna()
        .sort_values("beds_count", ascending=False)
    )

    for _, row in displayed.iterrows():
        table_data.append([row["city_name"], row["beds_count"]])

    table = Table(table_data)
    table.setStyle(
        TableStyle(
            [
                ("BACKGROUND", (0, 0), (-1, 0), colors.grey),
                ("GRID", (0, 0), (-1, -1), 1, colors.black),
            ]
        )
    )

    elements.append(table)

    doc.build(elements)

    return pdf_path


def generate_all_departments_in_one_pdf() -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    towns, _, _ = load_all_data()

    department_codes = sorted(
        towns["department_code"]
        .dropna()
        .astype(str)
        .unique(),
        key=lambda x: int(x)
    )

    pdf_path = OUTPUT_DIR / "all_departments.pdf"

    styles = getSampleStyleSheet()
    doc = SimpleDocTemplate(str(pdf_path), pagesize=A4)

    elements = []

    for index, department_code in enumerate(department_codes):
        department_towns, department_hospitals, department_stats = get_department_data(department_code)

        if department_stats.empty:
            continue

        if not department_hospitals.empty:
            department_name = department_hospitals["department_name"].dropna().iloc[0]
        else:
            department_name = department_towns["department_name"].dropna().iloc[0]

        title = Paragraph(
            f"<b>Département {department_code} - {department_name}</b>",
            styles["Title"]
        )
        elements.append(title)
        elements.append(Spacer(1, 20))

        stats = department_stats.iloc[0]

        stats_text = f"""
        <b>Statistiques :</b><br/>
        Nombre d'hôpitaux : {stats['hospital_count']}<br/>
        Total de lits : {stats['total_beds']}<br/>
        Population totale : {stats['total_population']}<br/>
        Lits pour 1000 habitants : {stats['beds_per_1000']}
        """

        elements.append(Paragraph(stats_text, styles["Normal"]))
        elements.append(Spacer(1, 20))

        elements.append(Paragraph("<b>Villes avec hôpital</b>", styles["Heading2"]))
        elements.append(Spacer(1, 10))

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
                table_data.append([row["city_name"], row["beds_count"]])

        table = Table(table_data)
        table.setStyle(
            TableStyle(
                [
                    ("BACKGROUND", (0, 0), (-1, 0), colors.grey),
                    ("GRID", (0, 0), (-1, -1), 1, colors.black),
                    ("FONTNAME", (0, 0), (-1, 0), "Helvetica-Bold"),
                ]
            )
        )

        elements.append(table)

        if index < len(department_codes) - 1:
            elements.append(PageBreak())

    doc.build(elements)

    return pdf_path