#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
cal_yield_v3.py

与 v2 保持调用方式一致：
    python3 cal_yield_v3.py levels.csv transitions.csv correction.csv output.csv

功能变更（按用户“v3”规范）：
- 一般规则：
  1) 对每个核素（按 base 归并，去掉 a/b 等后缀），先在 levels 里找到 |J| 最小的自旋 Jg；
  2) 在该核素的所有跃迁（按 final_nuclide 归并且做 base 归并）中，找 "次低能态自旋" J2 = min{ final_J | final_J > Jg }；
  3) 产额 = Σ_{final_J=J2 且 init_J>J2} I_rel，误差按二范数合成。
- 方法1（有自旋修正，读取 correction.csv 的 R_analytic 列；C=1/(1-R)）：
  特殊核素覆盖：
    * sr96：末态为 2+ 且 跃迁能量(transition_energy) ∈ {691,1161,978,1305} ±2
    * kr90：末态为 2+ 且 跃迁能量 ∈ {1123,1056,665} ±2
    * kr92、ge82、sr94：所有末态为 2+ 的跃迁求和
- 方法2（无修正）：自动输出一个 _raw 文件（不改变原始脚本调用方式）。
  特殊核素覆盖（4+→2+ @ 指定 跃迁能量 ±2）：
    sr96@978, sr94@1306, kr92a@1035 (归并为 kr92), kr90@1123, ge82@939
- 特别说明：带后缀的核素（如 kr90a/kr90b）均归并为同一 base（kr90），
  能量判断均基于 “跃迁能量 transition_energy”（不是末态能级能量）。

输入列名（与提供文件一致，不做猜测）：
levels.csv: level_index, level_name, spin, Z, A, short_name, incoming_Irel, incoming_Irel_unc, outgoing_Irel, outgoing_Irel_unc
transitions.csv: init_nuclide, final_nuclide, transition_energy, transition_energy_unc, I_rel, I_rel_unc, init_spin, final_spin
correction.csv: 需要包含 Z, A, R_analytic（与 v2 一致）；如果未命中则视为 R=0。
"""

import csv
import math
import re
import argparse
from collections import defaultdict

TOL_E = 2.0  # keV
_SPIN_RE = re.compile(r"""
    ^\s*
    (?P<num>(\d+(/\d+)?)|(\d*\.\d+))   # 0, 2, 4, 5/2, 3.5 等
    \s*
    (?P<parity>[\+\-])?                # + 或 -
    \s*$
""", re.X)

# --------- 特殊核素（方法1：带修正）---------
M1_FINAL2P_ENERGIES = {
    "sr96": [691.0, 1161.0, 978.0, 1305.0],
    "kr90": [1123.0, 1056.0, 665.0],
}
M1_FINAL2P_ALL = {"kr92", "ge82", "sr94"}  # 所有末态为 2+ 的跃迁和

# --------- 特殊核素（方法2：不带修正，固定 4+→2+@E）---------
M2_4TO2_TARGETS = {
    "sr96": [978.0],
    "sr94": [1306.0],
    "kr92": [1035.0],  # kr92a → 归并 kr92
    "kr90": [1123.0],
    "ge82": [939.0],
}

def base_key(name: str) -> str:
    """去掉 a/b 等末尾的单字母后缀，并去除连字符，转小写。例如 'kr-90a' → 'kr90'。"""
    if not name:
        return name
    s = name.strip().lower().replace('-', '')
    m = re.fullmatch(r'([a-z]+)(\d+)([a-z]?)', s)
    if not m:
        return s
    return f"{m.group(1)}{m.group(2)}"

def parse_spin(spin: str):
    """把 '5/2+' → (2.5,'+'); '2+' → (2.0,'+'); '0+' → (0.0,'+').
       不能解析则返回 (None,None)。"""
    if spin is None:
        return None, None
    s = str(spin).strip().lower()
    s = s.replace('(', '').replace(')', '').replace('?', '').replace('~', '').strip()
    m = _SPIN_RE.match(s)
    if not m:
        return None, None
    num = m.group('num')
    if '/' in num:
        n, d = num.split('/')
        j = float(n)/float(d)
    else:
        j = float(num)
    parity = m.group('parity') or ''
    return j, parity

def energy_close(x, target, tol=TOL_E):
    return (x is not None) and (abs(x - target) <= tol)

def load_levels(path):
    levels = []
    with open(path, newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            levels.append({
                'level_name': row['level_name'].strip().lower(),
                'base': base_key(row['level_name']),
                'spin_raw': row['spin'].strip(),
                'Z': int(row['Z']), 'A': int(row['A']),
            })
    return levels

def load_transitions(path):
    trs = []
    with open(path, newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            init = row['init_nuclide'].strip().lower()
            final = row['final_nuclide'].strip().lower()
            init_j, init_p = parse_spin(row['init_spin'])
            final_j, final_p = parse_spin(row['final_spin'])
            I_rel  = float(row['I_rel'] or 0.0)
            dI_rel = float(row['I_rel_unc'] or 0.0)
            E_tr   = float(row['transition_energy'] or 0.0)
            # energy_unc 可以保留但不用于判窗（用户仅要求 ±2 窗基于跃迁能量本身）
            trs.append({
                'init': init, 'final': final,
                'base_init': base_key(init), 'base_final': base_key(final),
                'init_spin_raw': row['init_spin'].strip(),
                'final_spin_raw': row['final_spin'].strip(),
                'init_J': init_j, 'init_parity': init_p,
                'final_J': final_j, 'final_parity': final_p,
                'I_rel': I_rel, 'dI_rel': dI_rel,
                'E_trans': E_tr,
            })
    return trs

def load_rcorr(path):
    r = {}
    with open(path, newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            Z = int(row['Z']); A = int(row['A'])
            raw = (row.get('R_analytic','') or '').strip()
            try:
                R = float(raw) if raw else 0.0
            except ValueError:
                R = 0.0
            r[(Z,A)] = R
    return r

def ground_spin_of_base(levels_by_base):
    """返回每个 base 核素的“最低能态自旋 Jg”（按 |J| 最小）。"""
    g = {}
    for base, arr in levels_by_base.items():
        best = None
        for lv in arr:
            J, P = parse_spin(lv['spin_raw'])
            if J is None:
                continue
            if (best is None) or (abs(J) < abs(best)):
                best = J
        g[base] = best
    return g

def sum_with_err(pairs):
    s = 0.0; e2 = 0.0
    for I, d in pairs:
        s += I
        e2 += (d or 0.0)**2
    return s, math.sqrt(e2)

def method1_general(base, gJ, trs_of_base):
    """一般规则（方法1/2共同）：取“次低能态 J2”为 final_J 中最小的 > gJ；
       求所有 final_J=J2 且 init_J>J2 的跃迁合计。"""
    if gJ is None:
        return 0.0, 0.0
    cand = sorted({t['final_J'] for t in trs_of_base if (t['final_J'] is not None and t['final_J'] > gJ)})
    if not cand:
        return 0.0, 0.0
    J2 = cand[0]
    pairs = [(t['I_rel'], t['dI_rel']) for t in trs_of_base
             if (t['final_J'] == J2) and (t['init_J'] is not None) and (t['init_J'] > J2)]
    return sum_with_err(pairs)

def method1_special(base, trs_by_base):
    """方法1特殊核素覆盖：
       - 指定列表内：末态 2+ 且 跃迁能量命中 {…}±2；
       - kr92/ge82/sr94：所有末态为 2+ 的跃迁求和。"""
    trs = trs_by_base.get(base, [])
    if base in M1_FINAL2P_ENERGIES:
        targets = M1_FINAL2P_ENERGIES[base]
        pairs = []
        for t in trs:
            if t['final_spin_raw'].replace(' ', '') != '2+':
                continue
            if any(energy_close(t['E_trans'], v) for v in targets):
                pairs.append((t['I_rel'], t['dI_rel']))
        return sum_with_err(pairs)
    if base in M1_FINAL2P_ALL:
        pairs = [(t['I_rel'], t['dI_rel']) for t in trs if t['final_spin_raw'].replace(' ','') == '2+']
        return sum_with_err(pairs)
    return None

def method2_special_raw(base, trs_by_base):
    """方法2（_raw，无修正）特殊核素：固定 4+→2+ @ 指定 跃迁能量 ±2。"""
    if base not in M2_4TO2_TARGETS:
        return None
    targets = M2_4TO2_TARGETS[base]
    trs = trs_by_base.get(base, [])
    pairs = []
    for t in trs:
        if t['init_spin_raw'].replace(' ','') != '4+' or t['final_spin_raw'].replace(' ','') != '2+':
            continue
        if any(energy_close(t['E_trans'], v) for v in targets):
            pairs.append((t['I_rel'], t['dI_rel']))
    return sum_with_err(pairs)

def main():
    ap = argparse.ArgumentParser(description="cal_yield v3 (exact headers)")
    ap.add_argument("levels_csv")
    ap.add_argument("transitions_csv")
    ap.add_argument("correction_csv")
    ap.add_argument("output_csv")
    args = ap.parse_args()

    levels = load_levels(args.levels_csv)
    trans  = load_transitions(args.transitions_csv)
    rcorr  = load_rcorr(args.correction_csv)

    # 按 base 归并
    levels_by_base = defaultdict(list)
    meta_by_base   = {}
    for lv in levels:
        levels_by_base[lv['base']].append(lv)
        meta_by_base[lv['base']] = (lv['Z'], lv['A'])

    trs_by_base = defaultdict(list)
    for t in trans:
        # 以 'final' 的 base 作为该跃迁计入的核素（同时跨分支会自动归并到相同 base）
        b = t['base_final']
        trs_by_base[b].append(t)

    gspin = ground_spin_of_base(levels_by_base)

    # 方法1（有修正）
    m1_results = {}
    all_bases = sorted(set(list(levels_by_base.keys()) + list(trs_by_base.keys())))
    for base in all_bases:
        if base not in meta_by_base:
            # 没有 level 元数据（Z,A），无法出结果
            continue
        Z, A = meta_by_base[base]

        sp = method1_special(base, trs_by_base)
        if sp is not None:
            total, unc = sp
        else:
            total, unc = method1_general(base, gspin.get(base), trs_by_base.get(base, []))

        R = rcorr.get((Z, A), 0.0)
        C = (float('inf') if R >= 1.0 else (1.0 / (1.0 - R)))
        corr = total * C
        corr_unc = unc * C
        m1_results[base] = (Z, A, total, unc, corr, corr_unc)

    # 写方法1输出
    out1 = args.output_csv
    with open(out1, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['AtomicNumber','MassNumber','TotalIntensity','TotalUncertainty',
                    'CorrectedIntensity','CorrectedUncertainty'])
        for base in sorted(m1_results.keys(), key=lambda b:(m1_results[b][0], m1_results[b][1])):
            Z, A, total, unc, corr, corr_unc = m1_results[base]
            w.writerow([Z, A,
                        f"{total:.6f}", f"{unc:.6f}",
                        f"{corr:.6f}", f"{corr_unc:.6f}"])
    print(f"[方法1] 已生成 → {out1}")

    # 方法2（_raw，无修正）
    m2_results = {}
    for base in all_bases:
        if base not in meta_by_base:
            continue
        Z, A = meta_by_base[base]

        sp = method2_special_raw(base, trs_by_base)
        if sp is not None:
            total, unc = sp
        else:
            total, unc = method1_general(base, gspin.get(base), trs_by_base.get(base, []))

        corr = total
        corr_unc = unc
        m2_results[base] = (Z, A, total, unc, corr, corr_unc)

    # 写方法2输出（_raw）
    if '.' in args.output_csv:
        stem, ext = args.output_csv.rsplit('.', 1)
        out2 = f"{stem}_raw.{ext}"
    else:
        out2 = f"{args.output_csv}_raw"
    with open(out2, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['AtomicNumber','MassNumber','TotalIntensity','TotalUncertainty',
                    'CorrectedIntensity','CorrectedUncertainty'])
        for base in sorted(m2_results.keys(), key=lambda b:(m2_results[b][0], m2_results[b][1])):
            Z, A, total, unc, corr, corr_unc = m2_results[base]
            w.writerow([Z, A,
                        f"{total:.6f}", f"{unc:.6f}",
                        f"{corr:.6f}", f"{corr_unc:.6f}"])
    print(f"[方法2] 已生成 → {out2}")

if __name__ == "__main__":
    main()
