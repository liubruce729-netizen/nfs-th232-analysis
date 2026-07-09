import re
import argparse
from pathlib import Path
import pandas as pd


TRANSITION_CSV = "second_level_transitions.csv"
CORRECTION_CSV = "spin_correction_factors_260501.csv"

OUTPUT_CSV = "th232_sy_260326_fix_v1_for_tradition_level_corrected_yields.csv"


SYMBOL_TO_Z = {
    "h": 1, "he": 2, "li": 3, "be": 4, "b": 5,
    "c": 6, "n": 7, "o": 8, "f": 9, "ne": 10,
    "na": 11, "mg": 12, "al": 13, "si": 14,
    "p": 15, "s": 16, "cl": 17, "ar": 18,
    "k": 19, "ca": 20, "sc": 21, "ti": 22,
    "v": 23, "cr": 24, "mn": 25, "fe": 26,
    "co": 27, "ni": 28, "cu": 29, "zn": 30,
    "ga": 31, "ge": 32, "as": 33, "se": 34,
    "br": 35, "kr": 36, "rb": 37, "sr": 38,
    "y": 39, "zr": 40, "nb": 41, "mo": 42,
    "tc": 43, "ru": 44, "rh": 45, "pd": 46,
    "ag": 47, "cd": 48, "in": 49, "sn": 50,
    "sb": 51, "te": 52, "i": 53, "xe": 54,
    "cs": 55, "ba": 56, "la": 57, "ce": 58,
    "pr": 59, "nd": 60, "pm": 61, "sm": 62,
    "eu": 63, "gd": 64,
}


def parse_nuclide(nuclide):
    """
    sr95 -> (38,95)
    xe144 -> (54,144)
    """
    nuclide = str(nuclide).strip().lower()

    m = re.match(r"^([a-z]+)(\d+)", nuclide)

    if not m:
        raise ValueError(f"Cannot parse nuclide: {nuclide}")

    symbol = m.group(1)
    A = int(m.group(2))

    if symbol not in SYMBOL_TO_Z:
        raise ValueError(f"Unknown element symbol: {symbol}")

    Z = SYMBOL_TO_Z[symbol]

    return Z, A


def parse_args():
    parser = argparse.ArgumentParser(
        description="根据 second-level transitions 和 spin 修正因子计算 traditional 产额。"
    )
    parser.add_argument(
        "--transitions",
        default=TRANSITION_CSV,
        help=f"take_trans.py 生成的 second-level transition CSV，默认 {TRANSITION_CSV}",
    )
    parser.add_argument(
        "--correction",
        default=CORRECTION_CSV,
        help=f"spin 修正因子 CSV，默认 {CORRECTION_CSV}",
    )
    parser.add_argument(
        "--output",
        default=OUTPUT_CSV,
        help=f"输出 traditional corrected yields CSV，默认 {OUTPUT_CSV}",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    transition_csv = Path(args.transitions)
    correction_csv = Path(args.correction)
    output_csv = Path(args.output)

    df_trans = pd.read_csv(transition_csv)
    df_corr = pd.read_csv(correction_csv)

    correction_map = {
        str(row["nuclide"]).strip().lower(): float(row["spin_correction_factor"])
        for _, row in df_corr.iterrows()
    }

    results = []

    for _, row in df_trans.iterrows():
        nuclide = str(row["nuclide"]).strip().lower()

        if nuclide not in correction_map:
            continue

        factor = correction_map[nuclide]

        total_intensity = float(row["I_rel"])
        total_uncertainty = float(row["I_rel_unc"])

        corrected_intensity = total_intensity * factor
        corrected_uncertainty = total_uncertainty * factor

        Z, A = parse_nuclide(nuclide)

        results.append({
            "AtomicNumber": Z,
            "MassNumber": A,
            "TotalIntensity": total_intensity,
            "TotalUncertainty": total_uncertainty,
            "CorrectedIntensity": corrected_intensity,
            "CorrectedUncertainty": corrected_uncertainty,
        })

    out = pd.DataFrame(results)

    out = out.sort_values(
        by=["AtomicNumber", "MassNumber"]
    ).reset_index(drop=True)

    out.to_csv(output_csv, index=False)

    print(f"Done. Output written to: {output_csv}")
    print(f"Number of corrected nuclides: {len(out)}")


if __name__ == "__main__":
    main()