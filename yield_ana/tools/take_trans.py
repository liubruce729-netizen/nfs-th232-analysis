import re
import argparse
from pathlib import Path
import warnings
import numpy as np
import pandas as pd


INPUT_CSV = "th232_sy_260326_fix_v1_for_tradition_trans.csv"
OUTPUT_CSV = "second_level_transitions.csv"
LOG_CSV = "long_lived_correction_log.csv"

TIME_WINDOW_NS = 80.0


# =========================
# 特殊接口 1：手动指定唯一跃迁
# =========================
SPECIAL_SELECT = {
    # "kr90": [
    #     ("kr90b", "2+", "kr90", "2+"),
    # ],
}


# =========================
# 特殊接口 2：手动指定多个唯一跃迁，并将强度相加
# =========================
SPECIAL_SUM = {
    "kr90": [
        ("kr90b", "2+", "kr90", "2+"),
        ("kr90",  "4+", "kr90", "2+"),
        ("kr90b",  "4+", "kr90", "2+"),
        ("kr90b",  "2+", "kr90b", "0+"),
    ],
    "sr96":[
        ("sr96b", "2+", "sr96", "0+"),
        ("sr96b", "2+", "sr96", "2+"),
        ("sr96b", "4+", "sr96", "2+"),
        ("sr96", "4+", "sr96", "2+"),
        ("sr96c", "2+", "sr96", "2+"),
    ],
    "kr92": [
        ("kr92a",  "4+", "kr92a", "2+"),
        ("kr92b",  "4+", "kr92a", "2+"),
    ],
    "sr94":[
        ("sr94",  "4+", "sr94", "2+"),
        ("sr94b",  "3-", "sr94", "2+"),
    ],
    "xe136":[
        ("xe136",  "4+", "xe136", "2+"),
        ("xe136",  "2+", "xe136", "2+"),
    ],
    "ge82":[
        ("ge82",  "4+", "ge82", "2+"),
        ("ge82b",  "4+", "ge82", "2+"),
    ],
}


# =========================
# 特殊接口 3：长寿命态修正
# =========================
# base_nuclide: [
#     {
#         "state_nuclide": "sr95a",
#         "state_spin": "7/2+",
#         "half_life_ns": 21.95,
#     },
# ]
LONG_LIVED_STATES = {
    "sr95": [
        {
            "state_nuclide": "sr95a",
            "state_spin": "7/2+",
            "half_life_ns": 21.9,
        },
    ],
    "sn130": [
        {
            "state_nuclide": "sn130",
            "state_spin": "4+",
            "half_life_ns": 52,
        },
    ],
    "te134": [
        {
            "state_nuclide": "te134",
            "state_spin": "6+",
            "half_life_ns": 164.1,
        },
    ],
    "te135": [
        {
            "state_nuclide": "te135",
            "state_spin": "4+",
            "half_life_ns": 511,
        },
    ],
    "xe136": [
        {
            "state_nuclide": "xe136",
            "state_spin": "6+",
            "half_life_ns": 2950,
        },
    ],
}


def normalize_nuclide(name):
    name = str(name).strip().lower()
    m = re.match(r"^([a-z]+)(\d+)", name)
    if not m:
        return name
    return m.group(1) + m.group(2)


def clean_spin(spin):
    return str(spin).strip()


def clean_nuclide(name):
    return str(name).strip().lower()


def spin_to_value(spin):
    s = clean_spin(spin)
    s = s.replace("+", "").replace("-", "")

    if "/" in s:
        a, b = s.split("/")
        return float(a) / float(b)

    return float(s)


def match_unique_transition(df, init_nuclide, init_spin, final_nuclide, final_spin):
    return df[
        (df["init_nuclide"].apply(clean_nuclide) == clean_nuclide(init_nuclide)) &
        (df["init_spin"].apply(clean_spin) == clean_spin(init_spin)) &
        (df["final_nuclide"].apply(clean_nuclide) == clean_nuclide(final_nuclide)) &
        (df["final_spin"].apply(clean_spin) == clean_spin(final_spin))
    ].copy()


def make_output_row(r, nuclide, mode, first=None):
    return {
        "nuclide": nuclide,
        "source_init_nuclide": r["init_nuclide"],
        "source_final_nuclide": r["final_nuclide"],
        "transition_energy": r["transition_energy"],
        "transition_energy_unc": r["transition_energy_unc"],
        "I_rel": r["I_rel"],
        "I_rel_unc": r["I_rel_unc"],
        "init_spin": r["init_spin"],
        "final_spin": r["final_spin"],
        "first_level_init_spin": "" if first is None else first["init_spin"],
        "first_level_final_spin": "" if first is None else first["final_spin"],
        "mode": mode,
    }


def apply_long_lived_correction(df, log_csv):
    """
    对长寿命态进行有限时间窗修正。

    对指定态 J：
    1. 找到该态向下的跃迁：state_nuclide state_spin -> ...
    2. 取该跃迁强度 I_observed
    3. k = exp(-ln2 * window / T_half)
    4. delta_I = I_observed * k / (1-k)
    5. 将 delta_I 加到该态以下 cascade 的所有跃迁中
    """
    df = df.copy()

    df["I_rel"] = df["I_rel"].astype(float)
    df["I_rel_unc"] = df["I_rel_unc"].astype(float)

    correction_records = []

    for base_nuclide, states in LONG_LIVED_STATES.items():
        for state in states:
            state_nuclide = clean_nuclide(state["state_nuclide"])
            state_spin = clean_spin(state["state_spin"])
            half_life_ns = float(state["half_life_ns"])

            print(
                f"INFO: applying long-lived correction for "
                f"{state_nuclide} {state_spin}, T1/2 = {half_life_ns} ns."
            )

            state_decay = df[
                (df["init_nuclide"].apply(clean_nuclide) == state_nuclide) &
                (df["init_spin"].apply(clean_spin) == state_spin)
            ].copy()

            if state_decay.empty:
                warnings.warn(
                    f"{base_nuclide}: long-lived state transition not found: "
                    f"{state_nuclide} {state_spin} -> ..."
                )
                continue

            if len(state_decay) > 1:
                warnings.warn(
                    f"{base_nuclide}: multiple outgoing transitions found for long-lived state "
                    f"{state_nuclide} {state_spin}. "
                    f"All outgoing intensities will be summed."
                )

            I_observed = state_decay["I_rel"].astype(float).sum()
            I_observed_unc = np.sqrt(np.sum(state_decay["I_rel_unc"].astype(float) ** 2))

            k = np.exp(-np.log(2.0) * TIME_WINDOW_NS / half_life_ns)
            factor = k / (1.0 - k)

            delta_I = I_observed * factor
            delta_I_unc = I_observed_unc * factor

            # 找该态以下的 cascade：
            # 从 state_spin 的末态开始一路向下追溯。
            spins_to_correct = set()
            next_spins = set(state_decay["final_spin"].apply(clean_spin))

            while next_spins:
                current_spin = next_spins.pop()
                if current_spin in spins_to_correct:
                    continue

                spins_to_correct.add(current_spin)

                lower_rows = df[
                    (df["base_nuclide"] == base_nuclide) &
                    (df["init_spin"].apply(clean_spin) == current_spin)
                ]

                for fs in lower_rows["final_spin"].apply(clean_spin):
                    if fs not in spins_to_correct:
                        next_spins.add(fs)

            # 被修正的跃迁包括：
            # 1. 长寿命态自身向下的跃迁
            # 2. 其下方所有 init_spin 属于 spins_to_correct 的跃迁
            mask_state_decay = (
                (df["init_nuclide"].apply(clean_nuclide) == state_nuclide) &
                (df["init_spin"].apply(clean_spin) == state_spin)
            )

            mask_lower_cascade = (
                (df["base_nuclide"] == base_nuclide) &
                (df["init_spin"].apply(clean_spin).isin(spins_to_correct))
            )

            mask_correct = mask_state_decay | mask_lower_cascade

            n_corr = int(mask_correct.sum())

            if n_corr == 0:
                warnings.warn(
                    f"{base_nuclide}: no transitions corrected for long-lived state "
                    f"{state_nuclide} {state_spin}."
                )
                continue

            df.loc[mask_correct, "I_rel"] = df.loc[mask_correct, "I_rel"] + delta_I
            df.loc[mask_correct, "I_rel_unc"] = np.sqrt(
                df.loc[mask_correct, "I_rel_unc"] ** 2 + delta_I_unc ** 2
            )

            correction_records.append({
                "base_nuclide": base_nuclide,
                "state_nuclide": state_nuclide,
                "state_spin": state_spin,
                "half_life_ns": half_life_ns,
                "k_un-decayed": k,
                "factor_k_over_1_minus_k": factor,
                "I_observed": I_observed,
                "I_observed_unc": I_observed_unc,
                "delta_I": delta_I,
                "delta_I_unc": delta_I_unc,
                "n_corrected_transitions": n_corr,
            })

            print(
                f"INFO: {base_nuclide}: delta_I = {delta_I:.6g}, "
                f"delta_I_unc = {delta_I_unc:.6g}, "
                f"corrected transitions = {n_corr}."
            )

    if correction_records:
        pd.DataFrame(correction_records).to_csv(
            log_csv,
            index=False
        )
        print(f"INFO: long-lived correction log written to {log_csv}")

    return df


def find_first_level_transition(df_nuclide, nuclide):
    df = df_nuclide.copy()
    df["final_spin_value"] = df["final_spin"].apply(spin_to_value)

    min_final_spin = df["final_spin_value"].min()
    first_candidates = df[df["final_spin_value"] == min_final_spin]

    if len(first_candidates) > 1:
        warnings.warn(
            f"{nuclide}: multiple first-level candidates found. "
            f"Use the first one to trace second-level transitions."
        )

    return first_candidates.iloc[0]


def extract_default_second_level(df_nuclide, nuclide):
    first = find_first_level_transition(df_nuclide, nuclide)
    target_final_spin = clean_spin(first["init_spin"])

    second = df_nuclide[
        df_nuclide["final_spin"].apply(clean_spin) == target_final_spin
    ].copy()

    if second.empty:
        warnings.warn(
            f"{nuclide}: no second-level transition found. "
            f"First-level transition is {first['init_spin']} -> {first['final_spin']}."
        )
        return []

    if len(second) > 1:
        warnings.warn(
            f"{nuclide}: multiple second-level transitions found for "
            f"final_spin = {target_final_spin}. All will be extracted."
        )

    return [
        make_output_row(r, nuclide, "auto", first=first)
        for _, r in second.iterrows()
    ]


def extract_special_select(df_nuclide, nuclide, rules):
    print(f"INFO: {nuclide} is specified by SPECIAL_SELECT.")

    rows = []

    for init_nuclide, init_spin, final_nuclide, final_spin in rules:
        matched = match_unique_transition(
            df_nuclide,
            init_nuclide,
            init_spin,
            final_nuclide,
            final_spin,
        )

        if matched.empty:
            warnings.warn(
                f"{nuclide}: specified transition not found: "
                f"{init_nuclide} {init_spin} -> {final_nuclide} {final_spin}"
            )
            continue

        if len(matched) > 1:
            warnings.warn(
                f"{nuclide}: specified transition is still not unique: "
                f"{init_nuclide} {init_spin} -> {final_nuclide} {final_spin}. "
                f"All matched rows will be extracted."
            )

        for _, r in matched.iterrows():
            rows.append(make_output_row(r, nuclide, "special_select"))

    return rows


def extract_special_sum(df_nuclide, nuclide, rules):
    print(f"INFO: {nuclide} is specified by SPECIAL_SUM.")

    selected_rows = []

    for init_nuclide, init_spin, final_nuclide, final_spin in rules:
        matched = match_unique_transition(
            df_nuclide,
            init_nuclide,
            init_spin,
            final_nuclide,
            final_spin,
        )

        if matched.empty:
            warnings.warn(
                f"{nuclide}: specified sum transition not found: "
                f"{init_nuclide} {init_spin} -> {final_nuclide} {final_spin}"
            )
            continue

        if len(matched) > 1:
            warnings.warn(
                f"{nuclide}: specified sum transition is still not unique: "
                f"{init_nuclide} {init_spin} -> {final_nuclide} {final_spin}. "
                f"All matched rows will be included."
            )

        selected_rows.append(matched)

    if not selected_rows:
        return []

    selected = pd.concat(selected_rows, ignore_index=True)

    I_sum = selected["I_rel"].astype(float).sum()
    I_unc_sum = np.sqrt(np.sum(selected["I_rel_unc"].astype(float) ** 2))

    return [{
        "nuclide": nuclide,
        "source_init_nuclide": "",
        "source_final_nuclide": "",
        "transition_energy": "",
        "transition_energy_unc": "",
        "I_rel": I_sum,
        "I_rel_unc": I_unc_sum,
        "init_spin": "",
        "final_spin": "",
        "first_level_init_spin": "",
        "first_level_final_spin": "",
        "mode": "special_sum",
    }]


def parse_args():
    parser = argparse.ArgumentParser(
        description="从 AGS 解析出的 transition CSV 提取 traditional 方法使用的 second-level transitions。"
    )
    parser.add_argument(
        "--input",
        default=INPUT_CSV,
        help=f"输入 transition CSV，默认 {INPUT_CSV}",
    )
    parser.add_argument(
        "--output",
        default=OUTPUT_CSV,
        help=f"输出 second-level transition CSV，默认 {OUTPUT_CSV}",
    )
    parser.add_argument(
        "--log",
        default=None,
        help=f"长寿命态修正日志；默认与输出文件同目录的 {LOG_CSV}",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    input_csv = Path(args.input)
    output_csv = Path(args.output)
    log_csv = Path(args.log) if args.log else output_csv.with_name(LOG_CSV)

    df = pd.read_csv(input_csv)

    required_cols = [
        "init_nuclide",
        "final_nuclide",
        "transition_energy",
        "transition_energy_unc",
        "I_rel",
        "I_rel_unc",
        "init_spin",
        "final_spin",
    ]

    for col in required_cols:
        if col not in df.columns:
            raise ValueError(f"Missing required column: {col}")

    df["base_nuclide"] = df["final_nuclide"].apply(normalize_nuclide)

    # 先做长寿命态修正
    df = apply_long_lived_correction(df, log_csv)

    results = []

    for nuclide, g in df.groupby("base_nuclide"):
        if nuclide in SPECIAL_SUM:
            rows = extract_special_sum(g, nuclide, SPECIAL_SUM[nuclide])
        elif nuclide in SPECIAL_SELECT:
            rows = extract_special_select(g, nuclide, SPECIAL_SELECT[nuclide])
        else:
            rows = extract_default_second_level(g, nuclide)

        results.extend(rows)

    out = pd.DataFrame(results)
    out.to_csv(output_csv, index=False)

    print(f"Done. Output written to: {output_csv}")
    print(f"Number of extracted rows: {len(out)}")


if __name__ == "__main__":
    main()