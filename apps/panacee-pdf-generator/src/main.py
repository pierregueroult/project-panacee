from generate_pdf import generate_all_departments_in_one_pdf


def main():
    pdf_path = generate_all_departments_in_one_pdf()
    print(f"PDF generated: {pdf_path}")


if __name__ == "__main__":
    main()
