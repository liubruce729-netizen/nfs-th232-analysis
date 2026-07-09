import re
import csv
import math
import argparse

# —— 支持的元素符号到原子序数映射 ——
ELEMENT_Z = {
    'h': 1,  'he': 2,  'li': 3,  'be': 4,  'b': 5,   'c': 6,   'n': 7,   'o': 8,   'f': 9,   'ne': 10,
    'na': 11,'mg': 12, 'al': 13,'si': 14, 'p': 15,  's': 16,  'cl':17,  'ar':18,
    'k':19,  'ca':20,  'sc':21, 'ti':22, 'v':23,  'cr':24, 'mn':25, 'fe':26, 'co':27, 'ni':28, 'cu':29, 'zn':30,
    'ga':31, 'ge':32, 'as':33, 'se':34, 'br':35, 'kr':36,
    'rb':37, 'sr':38, 'y':39,  'zr':40, 'nb':41, 'mo':42, 'tc':43, 'ru':44, 'rh':45, 'pd':46, 'ag':47, 'cd':48,
    'in':49, 'sn':50, 'sb':51, 'te':52, 'i':53,   'xe':54,
    'cs':55, 'ba':56, 'la':57, 'ce':58, 'pr':59, 'nd':60, 'pm':61, 'sm':62, 'eu':63, 'gd':64, 'tb':65, 'dy':66, 'ho':67, 'er':68, 'tm':69, 'yb':70, 'lu':71,
    'hf':72, 'ta':73, 'w':74,  're':75, 'os':76, 'ir':77, 'pt':78, 'au':79, 'hg':80,
    'tl':81, 'pb':82, 'bi':83, 'po':84, 'at':85, 'rn':86,
    'fr':87, 'ra':88, 'ac':89, 'th':90, 'pa':91, 'u':92,  'np':93, 'pu':94, 'am':95, 'cm':96, 'bk':97, 'cf':98, 'es':99, 'fm':100,
    'md':101,'no':102,'lr':103,'rf':104,'db':105,'sg':106,'bh':107,'hs':108,'mt':109,'ds':110,'rg':111,'cn':112,
    'nh':113,'fl':114,'mc':115,'lv':116,'ts':117,'og':118,'un':150
}

def parse_nuclide(label):
    m = re.fullmatch(r'([A-Za-z]+)(\d+)([A-Za-z]?)', label)
    if not m:
        raise ValueError(f"无法解析带标签：{label}")
    elem, A_str, var = m.groups()
    elem_l = elem.lower()
    if elem_l not in ELEMENT_Z:
        raise KeyError(f"请在 ELEMENT_Z 中添加元素 '{elem_l}' → Z")
    Z = ELEMENT_Z[elem_l]
    A = int(A_str)
    return elem_l, A, var, Z, label

def parse_ags(filename):
    levels = []
    gammas = []
    band_labels = {}

    with open(filename, 'r') as f:
        lines = [ln.rstrip('\n') for ln in f]
    it = iter(lines)

    # 跳到 GLS 版本行
    for ln in it:
        if 'ASCII GLS file format version' in ln:
            break

    # 读取全局参数
    for ln in it:
        txt = ln.strip()
        if not txt or txt.startswith('*'):
            continue
        parts = txt.split()
        if len(parts) >= 4:
            try:
                _, Nlevels, Ngammas, Nbands = map(int, parts[:4])
                break
            except ValueError:
                continue
    else:
        raise RuntimeError("未能在文件中找到全局参数行")

    # 跳到 LevelBreak (Level 段开始)
    for ln in it:
        if 'levelbreak' in ln.lower():
            break

    # 读取 Levels
    cnt = 0
    while cnt < Nlevels:
        ln1 = next(it, None)
        if ln1 is None:
            raise RuntimeError("无法读取 Level 段")
        s = ln1.strip('& ').strip()
        if not s or s.startswith('*') or s.startswith('+'):
            continue
        parts = re.split(r'\s+', s)
        if len(parts) < 6:
            continue
        try:
            idx = int(parts[0])
            spin = parts[3]
            bandid = int(parts[5])
        except ValueError:
            continue
        levels.append({'idx': idx, 'spin': spin, 'band': bandid})
        cnt += 1
        next(it, None)

    # 跳到 Bands 段
    for ln in it:
        if 'band' in ln.lower() and 'name' in ln.lower():
            break

    # 读取 Bands
    cnt = 0
    while cnt < Nbands:
        ln = next(it, None)
        if ln is None:
            raise RuntimeError("无法读取 Band 段")
        text = ln.split('&')[0].strip()
        if not text or text.startswith('*') or text.startswith('+'):
            continue
        parts = re.split(r'\s+', text)
        try:
            bid = int(parts[0])
            label = parts[1]
        except (ValueError, IndexError):
            continue
        band_labels[bid] = label
        cnt += 1

    # 跳到 Gammas 段
    for ln in it:
        if 'gamma' in ln.lower():
            break

    # 读取 Gammas
    cnt = 0
    while cnt < Ngammas:
        ln1 = next(it, None)
        if ln1 is None:
            raise RuntimeError("无法读取 Gamma 段")
        s = ln1.strip('& ').strip()
        if not s or s.startswith('*') or s.startswith('+'):
            continue
        parts = re.split(r'\s+', s)
        if len(parts) < 9:
            next(it, None); next(it, None)
            continue
        try:
            _, energy, denergy, _, _, lvl_i, lvl_f, I_rel, dI_rel = parts[:9]
            lvl_i = int(lvl_i); lvl_f = int(lvl_f)
            energy = float(energy); denergy = float(denergy)
            I_rel = float(I_rel); dI_rel = float(dI_rel)
        except ValueError:
            next(it, None); next(it, None)
            continue
        gammas.append({
            'lvl_i': lvl_i, 'lvl_f': lvl_f,
            'energy': energy, 'denergy': denergy,
            'I_rel': I_rel, 'dI_rel': dI_rel
        })
        cnt += 1
        next(it, None); next(it, None)

    return levels, gammas, band_labels

def apply_long_lived_correction(levels, gammas, band_labels):
    """
    对如下长寿命激发态添加修正：
      Te-134 6+ (τ=164ns), Te-132 6+ (τ=145ns)
    使其所有直接 γ 强度乘以 k=1/(1−e^(−15ns/τ))，
    并将额外强度沿后续所有跃迁传递（每条后续 γ 都加上 a*(k−1)）。
    """
    tau_dict = {
        ('te', 134, '6+'): 164e-9,
        ('te', 132, '6+'): 145e-9,
        ('nd', 152, '2+'): 4.18e-9,
        ('nd', 154, '2+'): 7.7e-9,
        ('i', 136, '4+'): 4e-9,
        ('sn', 134, '6+'): 80e-9,
        ('sn', 132, '4+'): 3.95e-9,
        ('sr', 95, '7/2+'): 21.9e-9
    }
    t0 = 80e-9
    # 对每个长寿命电平
    for lvl in levels:
        lbl = band_labels.get(lvl['band'], '')
        elem, A, var, Z, _ = parse_nuclide(lbl)
        key = (elem, A, lvl['spin'])
        if key not in tau_dict:
            continue
        tau = tau_dict[key]
        k = 1.0 / (1.0 - math.exp(-t0 / tau))
        # 扫描所有指向该态的 γ 强度
        for g in gammas:
            if g['lvl_i'] != lvl['idx']:
                continue
            a = g['I_rel']
            if a <= 0.0:
                continue
            extra = a * (k - 1.0)
            g['I_rel'] = a * k
            # 递归将 extra 传递到后续所有 γ
            propagate_extra(g['lvl_f'], extra, gammas)

def propagate_extra(start_lvl, extra, gammas):
    """
    从 start_lvl 出发，递归地对所有出射 γ 强度加上 extra。
    """
    visited = set()
    stack = [start_lvl]
    while stack:
        lvl_i = stack.pop()
        if lvl_i in visited:
            continue
        visited.add(lvl_i)
        for g in gammas:
            if g['lvl_i'] == lvl_i:
                g['I_rel'] += extra
                stack.append(g['lvl_f'])

def compute_intensities(levels, gammas):
    inc = {lvl['idx']: 0.0 for lvl in levels}
    inc_err2 = {lvl['idx']: 0.0 for lvl in levels}
    out = {lvl['idx']: 0.0 for lvl in levels}
    out_err2 = {lvl['idx']: 0.0 for lvl in levels}

    for g in gammas:
        out[g['lvl_i']] += g['I_rel']
        out_err2[g['lvl_i']] += g['dI_rel'] ** 2
        inc[g['lvl_f']] += g['I_rel']
        inc_err2[g['lvl_f']] += g['dI_rel'] ** 2

    inc_err = {idx: math.sqrt(err2) for idx, err2 in inc_err2.items()}
    out_err = {idx: math.sqrt(err2) for idx, err2 in out_err2.items()}
    return inc, inc_err, out, out_err

def write_levels_csv(levels, inc, inc_err, out, out_err, band_labels, csvfile):
    with open(csvfile, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow([
            'level_index','level_name','spin','Z','A','short_name',
            'incoming_Irel','incoming_Irel_unc','outgoing_Irel','outgoing_Irel_unc'
        ])
        for lvl in sorted(levels, key=lambda x: x['idx']):
            idx = lvl['idx']
            spin = lvl['spin']
            label = band_labels.get(lvl['band'], '')
            short, A, var, Z, name = parse_nuclide(label)
            w.writerow([
                idx, name, spin, Z, A, short,
                f"{inc[idx]:.6f}", f"{inc_err[idx]:.6f}",
                f"{out[idx]:.6f}", f"{out_err[idx]:.6f}"
            ])

def write_transitions_csv(levels, gammas, band_labels, csvfile):
    level_map = {lvl['idx']: lvl for lvl in levels}
    with open(csvfile, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow([
            'init_nuclide','final_nuclide','transition_energy',
            'transition_energy_unc','I_rel','I_rel_unc','init_spin','final_spin'
        ])
        for g in gammas:
            li = g['lvl_i']; lf = g['lvl_f']
            lvl_i = level_map[li]; lvl_f = level_map[lf]
            label_i = band_labels.get(lvl_i['band'], '')
            label_f = band_labels.get(lvl_f['band'], '')
            _, Ai, _, Zi, name_i = parse_nuclide(label_i)
            _, Af, _, Zf, name_f = parse_nuclide(label_f)
            spin_i = lvl_i['spin']; spin_f = lvl_f['spin']
            w.writerow([
                name_i, name_f,
                f"{g['energy']:.3f}", f"{g['denergy']:.3f}",
                f"{g['I_rel']:.4f}", f"{g['dI_rel']:.4f}",
                spin_i, spin_f
            ])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="从 AGS 文件提取 level 和 transition 信息，并导出 CSV"
    )
    parser.add_argument("ags_file", help="输入的 AGS 文件路径")
    parser.add_argument("csv_levels", help="输出的 levels CSV 文件路径")
    parser.add_argument("csv_trans", help="输出的 transitions CSV 文件路径")
    args = parser.parse_args()

    # 读取和处理
    levels, gammas, band_labels = parse_ags(args.ags_file)

    # 在计算强度前，先做长寿命修正
    apply_long_lived_correction(levels, gammas, band_labels)

    inc, inc_err, out, out_err = compute_intensities(levels, gammas)
    write_levels_csv(levels, inc, inc_err, out, out_err, band_labels, args.csv_levels)
    write_transitions_csv(levels, gammas, band_labels, args.csv_trans)

    print(f"已生成 → {args.csv_levels} 和 {args.csv_trans}")