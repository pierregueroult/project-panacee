from generate_pdf import *


def main():
    
    pdf_path = generate_all_departments_in_one_pdf()
    print(f"PDF généré : {pdf_path}")


if __name__ == "__main__":
    main()