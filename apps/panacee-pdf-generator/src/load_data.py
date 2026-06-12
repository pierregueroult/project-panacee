import json
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import pandas as pd


REPO_ROOT = Path(__file__).resolve().parents[3]
OUTPUT_DIR = REPO_ROOT / "data" / "output"
INPUT_DIR = REPO_ROOT / "data" / "input"


def load_communes_coords() -> pd.DataFrame:
    df = pd.read_csv(
        INPUT_DIR / "communes-france-metrople-2025.csv",
        header=None,
        names=[
            "insee", "name", "region_code", "region_name",
            "dept_code", "dept_name", "postal_code", "inhabitants",
            "lat", "lon",
        ],
        usecols=["insee", "lat", "lon"],
    )
    df["insee"] = df["insee"].astype(str).str.zfill(5)
    df["lat"] = pd.to_numeric(df["lat"], errors="coerce")
    df["lon"] = pd.to_numeric(df["lon"], errors="coerce")
    return df.dropna(subset=["lat", "lon"])


Ring = List[Tuple[float, float]]  # list of (lat, lon)


def load_department_borders() -> Dict[str, List[Ring]]:
    """Returns {padded_dept_code: [ring, ring, ...]} where each ring is
    a list of (lat, lon). Polygon -> 1+ rings; MultiPolygon -> all rings
    from all polygons concatenated."""
    with open(INPUT_DIR / "departements.geojson", encoding="utf-8") as f:
        gj = json.load(f)

    out: Dict[str, List[Ring]] = {}
    for feature in gj.get("features", []):
        props = feature.get("properties", {})
        code = str(props.get("code") or "").strip()
        if not code:
            continue
        geom = feature.get("geometry") or {}
        gtype = geom.get("type")
        coords = geom.get("coordinates") or []

        rings: List[Ring] = []
        if gtype == "Polygon":
            polygons = [coords]
        elif gtype == "MultiPolygon":
            polygons = coords
        else:
            continue

        for poly in polygons:
            for ring in poly:
                rings.append([(lat, lon) for lon, lat in ring])

        out[code] = rings
    return out


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
    assigned = pd.to_numeric(df["assigned_hospital_insee"], errors="coerce")
    df["assigned_hospital_insee"] = assigned.where(assigned >= 0).map(
        lambda v: f"{int(v):05d}", na_action="ignore"
    )

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

    return summary.sort_values("department_code", key=lambda s: s.str.zfill(3))


def get_department_data(
    department_code: str,
    hospitals: pd.DataFrame,
    towns_status: pd.DataFrame,
    summary: pd.DataFrame,
    coords: Optional[pd.DataFrame] = None,
):
    dept_hospitals = hospitals[hospitals["department_code"] == department_code].copy()
    dept_towns = towns_status[towns_status["department_code"] == department_code].copy()
    dept_stats = summary[summary["department_code"] == department_code].copy()

    if coords is not None:
        dept_hospitals = dept_hospitals.merge(coords, on="insee", how="left")
        dept_towns = dept_towns.merge(coords, on="insee", how="left")

    return dept_towns, dept_hospitals, dept_stats
