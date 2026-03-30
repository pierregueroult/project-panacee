from pathlib import Path
import pandas as pd


DATA_DIR = Path(__file__).resolve().parent.parent / "data"


def load_towns() -> pd.DataFrame:
    towns_path = DATA_DIR / "communes-france-metrople-2025.csv"

    towns = pd.read_csv(towns_path, header=None)

    towns.columns = [
        "insee",
        "city_name",
        "region_code",
        "region_name",
        "department_code",
        "department_name",
        "postal_code",
        "inhabitants_count",
        "latitude",
        "longitude",
    ]

    towns["insee"] = towns["insee"].astype(str).str.zfill(5)
    towns["department_code"] = towns["department_code"].astype(str)
    towns["postal_code"] = towns["postal_code"].astype(str)

    towns["inhabitants_count"] = pd.to_numeric(
        towns["inhabitants_count"], errors="coerce"
    ).fillna(0).astype(int)
    towns["latitude"] = pd.to_numeric(towns["latitude"], errors="coerce")
    towns["longitude"] = pd.to_numeric(towns["longitude"], errors="coerce")

    return towns


def load_hospitals() -> pd.DataFrame:
    hospitals_path = DATA_DIR / "hospitals.csv"
    hospitals = pd.read_csv(hospitals_path)

    hospitals["insee"] = hospitals["insee"].astype(str).str.zfill(5)
    hospitals["beds_count"] = pd.to_numeric(
        hospitals["beds_count"], errors="coerce"
    ).fillna(0).astype(int)

    return hospitals


def load_fitness() -> pd.DataFrame:
    fitness_path = DATA_DIR / "fitness.csv"
    fitness = pd.read_csv(fitness_path)
    return fitness


def load_all_data() -> tuple[pd.DataFrame, pd.DataFrame, pd.DataFrame]:
    towns = load_towns()
    hospitals = load_hospitals()
    fitness = load_fitness()
    return towns, hospitals, fitness


def preview_data() -> None:
    towns, hospitals, fitness = load_all_data()

    print("\n=== TOWNS ===")
    print(towns.head())
    print("\nColonnes :", list(towns.columns))
    print("Shape :", towns.shape)

    print("\n=== HOSPITALS ===")
    print(hospitals.head())
    print("\nColonnes :", list(hospitals.columns))
    print("Shape :", hospitals.shape)

    print("\n=== FITNESS ===")
    print(fitness.head())
    print("\nColonnes :", list(fitness.columns))
    print("Shape :", fitness.shape)