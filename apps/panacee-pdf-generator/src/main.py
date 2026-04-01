from generate_pdf import generate_department_pdf


def main():
    pdf_path = generate_department_pdf("75")
    print(f"PDF généré : {pdf_path}")


if __name__ == "__main__":
    main()