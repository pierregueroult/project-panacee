from pathlib import Path
import pandas as pd


REPO_ROOT = Path(__file__).resolve().parents[3]
OUTPUT_DIR = REPO_ROOT / "data" / "output"


def load_hospitals() -> pd.DataFrame:
    df = pd.read_csv(OUTPUT_DIR / "hospitals.csv")

    df["insee"] = df["insee"].astype(str).str.zfill(5)
    df["department_code"] = df["department_code"].astype(str)
    df["beds_count"] = pd.to_numeric(df["beds_count"], errors="coerce").fillna(0).astype(int)
    df["inhabitants"] = pd.to_numeric(df["inhabitants"], errors="coerce").fillna(0).astype(int)
    df["is_chru"] = pd.to_numeric(df["is_chru"], errors="coerce").fillna(0).astype(int)

    return df


def load_towns_status() -> pd.DataFrame:
    df = pd.read_csv(OUTPUT_DIR / "towns_status.csv")

    df["insee"] = df["insee"].astype(str).str.zfill(5)
    df["department_code"] = df["department_code"].astype(str)
    df["inhabitants"] = pd.to_numeric(df["inhabitants"], errors="coerce").fillna(0).astype(int)
    df["assigned_hospital_insee"] = df["assigned_hospital_insee"].astype(str).str.zfill(5)

    return df


def load_fitness() -> pd.DataFrame:
    return pd.read_csv(OUTPUT_DIR / "fitness.csv")


def department_summary(hospitals: pd.DataFrame, towns_status: pd.DataFrame) -> pd.DataFrame:
    hosp_agg = (
        hospitals.groupby(["department_code", "department_name"], dropna=False)
        .agg(
            hospital_count=("insee", "count"),
            total_beds=("beds_count", "sum"),
            uhc_count=("is_chru", "sum"),
        )
        .reset_index()
    )
    pop_agg = (
        towns_status.groupby(["department_code", "department_name"], dropna=False)
        .agg(total_population=("inhabitants", "sum"))
        .reset_index()
    )

    summary = hosp_agg.merge(
        pop_agg,
        on=["department_code", "department_name"],
        how="outer",
    )

    for col in ("hospital_count", "total_beds", "uhc_count", "total_population"):
        summary[col] = summary[col].fillna(0).astype(int)

    summary["beds_per_1000"] = (
        summary["total_beds"]
        / summary["total_population"].replace(0, pd.NA)
        * 1000
    ).fillna(0).round(2)

    return summary.sort_values("department_code")


def get_department_data(
    department_code: str,
    hospitals: pd.DataFrame,
    towns_status: pd.DataFrame,
    summary: pd.DataFrame,
):
    dept_hospitals = hospitals[hospitals["department_code"] == department_code].copy()
    dept_towns = towns_status[towns_status["department_code"] == department_code].copy()
    dept_stats = summary[summary["department_code"] == department_code].copy()
    return dept_towns, dept_hospitals, dept_stats
