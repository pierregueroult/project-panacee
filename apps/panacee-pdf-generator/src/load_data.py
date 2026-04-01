from pathlib import Path
import pandas as pd


DATA_DIR = Path(__file__).resolve().parent.parent / "data"

def merge_towns_hospitals() -> pd.DataFrame:
    towns, hospitals, _ = load_all_data()

    merged = hospitals.merge(
        towns,
        on="insee",
        how="left"
    )

    return merged

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

def department_summary() -> pd.DataFrame:
    merged = merge_towns_hospitals()
    towns, _, _ = load_all_data()

    hospitals_by_department = (
        merged.groupby(["department_code", "department_name"], dropna=False)
        .agg(
            hospital_count=("insee", "count"),
            total_beds=("beds_count", "sum"),
        )
        .reset_index()
    )

    population_by_department = (
        towns.groupby(["department_code", "department_name"], dropna=False)
        .agg(
            total_population=("inhabitants_count", "sum"),
        )
        .reset_index()
    )

    summary = hospitals_by_department.merge(
        population_by_department,
        on=["department_code", "department_name"],
        how="outer"
    )

    summary["hospital_count"] = summary["hospital_count"].fillna(0).astype(int)
    summary["total_beds"] = summary["total_beds"].fillna(0).astype(int)
    summary["total_population"] = summary["total_population"].fillna(0).astype(int)

    summary["beds_per_1000"] = (
        summary["total_beds"] / summary["total_population"].replace(0, pd.NA)
    ) * 1000

    summary["beds_per_1000"] = summary["beds_per_1000"].fillna(0).round(2)

    return summary.sort_values("department_code")


def preview_department_summary() -> None:
    summary = department_summary()

    print("\n=== DEPARTMENT SUMMARY ===")
    print(summary.head(20))
    print("\nColonnes :", list(summary.columns))
    print("Shape :", summary.shape)

def get_department_data(department_code: str) -> tuple[pd.DataFrame, pd.DataFrame, pd.DataFrame]:
    towns, hospitals, _ = load_all_data()
    merged = merge_towns_hospitals()

    department_towns = towns[towns["department_code"] == department_code].copy()

    department_hospitals = merged[
        merged["department_code"] == department_code
    ].copy()

    department_summary_df = department_summary()
    department_stats = department_summary_df[
        department_summary_df["department_code"] == department_code
    ].copy()

    return department_towns, department_hospitals, department_stats


def preview_one_department(department_code: str) -> None:
    department_towns, department_hospitals, department_stats = get_department_data(department_code)

    print(f"\n=== DEPARTMENT {department_code} ===")

    print("\n--- STATS ---")
    print(department_stats)

    print("\n--- TOWNS ---")
    print(department_towns.head())
    print("Nombre de communes :", len(department_towns))

    print("\n--- HOSPITALS ---")
    print(
        department_hospitals[
            ["insee", "city_name", "department_name", "beds_count", "latitude", "longitude"]
        ].head(20)
    )
    print("Nombre d'hôpitaux :", len(department_hospitals))

