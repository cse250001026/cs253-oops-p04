#!/usr/bin/env python3
import argparse
import csv
import math
import os
import statistics
import sys
from collections import defaultdict

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FixedLocator, FuncFormatter, NullLocator

COLOR = {"A": "#2a78d6", "B": "#eb6834", "oracle": "#1baf7a"}
LABEL = {"A": "A (Tarjan low-link)", "B": "B (union-find marking)", "oracle": "Oracle (BFS)"}
INK, INK2, MUTED, GRID = "#0b0b0b", "#52514e", "#898781", "#e1e0d9"

BYTES_PER_VERTEX = 240
L2_BYTES = 1.25 * 2**20
L3_BYTES = 24 * 2**20

plt.rcParams.update({
    "font.size": 9, "axes.titlesize": 10, "axes.labelsize": 9,
    "axes.edgecolor": MUTED, "axes.labelcolor": INK2, "axes.titlecolor": INK,
    "xtick.color": MUTED, "ytick.color": MUTED, "xtick.labelcolor": INK2, "ytick.labelcolor": INK2,
    "axes.spines.top": False, "axes.spines.right": False,
    "axes.grid": True, "grid.color": GRID, "grid.linewidth": 0.6,
    "legend.frameon": False, "legend.fontsize": 8,
    "figure.facecolor": "white", "axes.facecolor": "white",
    "lines.linewidth": 1.5, "lines.markersize": 5.5,
})

def read_csv(path):
    if not os.path.exists(path):
        return None
    with open(path, newline="") as f:
        rows = list(csv.DictReader(f))
    for r in rows:
        for k in ("V", "E", "self_loops", "components", "bridges", "rep"):
            if k in r and r[k] not in ("", "NA"):
                r[k] = int(r[k])
        if "time_ms" in r:
            r["time_ms"] = float(r["time_ms"])
    return rows

def summarize(rows):
    groups = defaultdict(list)
    for r in rows:
        groups[(r["structure"], r["V"], r["E"], r["input"])].append(r)
    out = {}
    for key, rs in groups.items():
        t = [r["time_ms"] for r in rs]
        out[key] = {"mean": statistics.fmean(t), "sd": statistics.stdev(t) if len(t) > 1 else 0.0,
                    "n": len(t), "row": rs[0]}
    return out

def footnote(fig, text):
    fig.text(0.01, -0.02, text, fontsize=7, color=MUTED, ha="left", va="top")

def save(fig, out_dir, name):
    path = os.path.join(out_dir, name)
    fig.savefig(path, dpi=200, bbox_inches="tight")
    plt.close(fig)
    print("wrote", path)

def series(summary, structure):
    pts = sorted(((k[1], k[2], v) for k, v in summary.items() if k[0] == structure), key=lambda p: (p[0], p[1]))
    return [p[0] for p in pts], [p[1] for p in pts], [p[2] for p in pts]

def check_reps(summary, name):
    low = sorted({v["n"] for v in summary.values() if v["n"] < 5})
    if low:
        print(f"warning: {name} has points with fewer than 5 timed runs {low}; "
              "figures need at least 5 (project §6)", file=sys.stderr)

def fig_time_vs_v(summary, out_dir):
    fig, ax = plt.subplots(figsize=(7, 4.2))
    for s in ("A", "B", "oracle"):
        Vs, Es, st = series(summary, s)
        if not Vs:
            continue
        means = [x["mean"] for x in st]
        sds = [x["sd"] for x in st]
        ax.errorbar(Vs, means, yerr=sds, color=COLOR[s], marker="o", capsize=2, label=f"{LABEL[s]}: measured")
        if s == "oracle":
            work = [2 * (E - x["row"]["self_loops"]) * (V + E - x["row"]["self_loops"]) for V, E, x in zip(Vs, Es, st)]
            pred_label = "Oracle: predicted ∝ 2E′(V + E′)"
        else:
            work = [V + E for V, E in zip(Vs, Es)]
            pred_label = f"{s}: predicted ∝ V + E"
        pred = [means[0] * w / work[0] for w in work]
        ax.plot(Vs, pred, color=COLOR[s], linestyle="--", linewidth=1.2, label=pred_label)
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("vertices V (log scale; E = 5V)")
    ax.set_ylabel("time per solve (ms, log scale)")
    ax.set_title("Running time against V: measured and predicted")
    ax.legend(loc="upper left", bbox_to_anchor=(1.01, 1.0), fontsize=7.5)
    footnote(fig, "Data: results/raw/scaling.csv · bench/plot_results.py · mean ± 1 sd of 5 runs · "
                  "seed 20260928 + V · predictions anchored at the smallest V (REPORT.md §9.1)")
    save(fig, out_dir, "fig1_time_vs_V.png")

def fig_ns_per_edge(summary, full, out_dir):
    fig, ax = plt.subplots(figsize=(7, 4.2))
    top = 0
    for s in ("A", "B"):
        Vs, Es, st = series(summary, s)
        if not Vs:
            continue
        per = [x["mean"] * 1e6 / (V + E) for V, E, x in zip(Vs, Es, st)]
        sd = [x["sd"] * 1e6 / (V + E) for V, E, x in zip(Vs, Es, st)]
        top = max(top, max(p + d for p, d in zip(per, sd)))
        ax.errorbar(Vs, per, yerr=sd, color=COLOR[s], marker="o", capsize=2, label=f"{LABEL[s]}: measured")
        ax.plot([Vs[0], Vs[-1]], [per[0], per[0]], color=COLOR[s], linestyle="--", linewidth=1.2,
                label=f"{s}: predicted flat (cache-resident)")
        if full:
            for (fs, fV, fE, _), x in full.items():
                if fs == s:
                    v = x["mean"] * 1e6 / (fV + fE)
                    top = max(top, v + x["sd"] * 1e6 / (fV + fE))
                    ax.errorbar([fV], [v], yerr=[x["sd"] * 1e6 / (fV + fE)], color=COLOR[s], marker="s",
                                markerfacecolor="white", capsize=2, linestyle="none",
                                label=f"{s}: full scale (V = 10⁵, E = 10⁶)")
    for limit, text, ha in ((L2_BYTES, "working set > L2\n(1.25 MiB)", "left"),
                            (L3_BYTES, "working set > L3\n(24 MiB)", "right")):
        v = limit / BYTES_PER_VERTEX
        ax.axvline(v, color=MUTED, linestyle=":", linewidth=1)
        ax.text(v * 1.05 if ha == "left" else v / 1.05, top * 1.33, text, color=INK2, fontsize=7.5,
                va="top", ha=ha)
    ax.set_xscale("log")
    ax.set_xlim(left=700, right=max(130000, ax.get_xlim()[1]))
    ax.set_ylim(0, top * 1.4 if top else 1)
    ax.set_xlabel("vertices V (log scale; E = 5V)")
    ax.set_ylabel("time per (V + E) (ns)")
    ax.set_title("Time per vertex-plus-edge: predicted flat while cache-resident")
    ax.legend(loc="upper left", bbox_to_anchor=(1.01, 1.0), fontsize=7.5)
    footnote(fig, "Data: results/raw/scaling.csv, results/raw/fullscale.csv · bench/plot_results.py · "
                  "mean ± 1 sd of 5 runs · cache limits from REPORT.md §9.3 (≈240 B per vertex)")
    save(fig, out_dir, "fig2_ns_per_edge.png")

def fig_oracle(summary, out_dir):
    Vs, Es, st = series(summary, "oracle")
    if not Vs:
        print("no oracle rows: skipping fig3", file=sys.stderr)
        return
    fig, ax = plt.subplots(figsize=(7, 4.2))
    reads = [2 * (E - x["row"]["self_loops"]) * (V + E - x["row"]["self_loops"]) for V, E, x in zip(Vs, Es, st)]
    lo = [r * 1e-6 for r in reads]
    hi = [r * 3e-6 for r in reads]
    ax.fill_between(Vs, lo, hi, color=COLOR["oracle"], alpha=0.18, linewidth=0,
                    label="predicted: 2E′(V + E′) half-edge reads × 1–3 ns")
    ax.errorbar(Vs, [x["mean"] for x in st], yerr=[x["sd"] for x in st], color=COLOR["oracle"],
                marker="o", capsize=2, label="Oracle (BFS): measured")
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.xaxis.set_major_locator(FixedLocator(Vs))
    ax.xaxis.set_major_formatter(FuncFormatter(lambda v, _: f"{v:,.0f}"))
    ax.xaxis.set_minor_locator(NullLocator())
    ax.set_xlabel("vertices V (log scale; E = 5V)")
    ax.set_ylabel("time per solve (ms, log scale)")
    ax.set_title("Oracle running time against its absolute prediction")
    ax.legend(loc="upper left")
    footnote(fig, "Data: results/raw/scaling.csv · bench/plot_results.py · mean ± 1 sd of 5 runs · "
                  "prediction from REPORT.md §9.3, written before measuring")
    save(fig, out_dir, "fig3_oracle.png")

def fig_ratio(summary, out_dir):
    VA, _, sA = series(summary, "A")
    VB, _, sB = series(summary, "B")
    a = dict(zip(VA, sA))
    b = dict(zip(VB, sB))
    Vs = sorted(set(a) & set(b))
    if not Vs:
        return
    ratio = [b[v]["mean"] / a[v]["mean"] for v in Vs]
    sd = [r * math.sqrt((b[v]["sd"] / b[v]["mean"]) ** 2 + (a[v]["sd"] / a[v]["mean"]) ** 2) for r, v in zip(ratio, Vs)]
    fig, ax = plt.subplots(figsize=(7, 3.6))
    ax.axhspan(1.10, 1.20, color=COLOR["B"], alpha=0.15, linewidth=0, label="predicted: B 10–20% slower (§9.1)")
    ax.errorbar(Vs, ratio, yerr=sd, color=COLOR["B"], marker="o", capsize=2, label="measured t_B / t_A")
    ax.axhline(1.0, color=MUTED, linewidth=0.8)
    ax.set_xscale("log")
    ax.set_ylim(0, max(max(r + d for r, d in zip(ratio, sd)), 1.3) * 1.1)
    ax.set_xlabel("vertices V (log scale; E = 5V)")
    ax.set_ylabel("t_B / t_A")
    ax.set_title("Structure B relative to Structure A")
    ax.legend(loc="lower right")
    footnote(fig, "Data: results/raw/scaling.csv · bench/plot_results.py · ratio of means of 5 runs, "
                  "± propagated 1 sd")
    save(fig, out_dir, "fig4_B_over_A.png")

def fig_adversarial(rows, out_dir):
    summary = defaultdict(dict)
    for (s, _, _, name), x in summarize(rows).items():
        summary[name][s] = x
    names = sorted(summary, key=lambda n: n)
    if not names:
        return
    fig, ax = plt.subplots(figsize=(7, 0.42 * len(names) + 1.3))
    h = 0.38
    for i, s in enumerate(("A", "B")):
        ys = [k + (i - 0.5) * (h + 0.04) for k in range(len(names))]
        means = [summary[n][s]["mean"] if s in summary[n] else 0 for n in names]
        sds = [summary[n][s]["sd"] if s in summary[n] else 0 for n in names]
        ax.barh(ys, means, height=h, xerr=sds, color=COLOR[s], label=LABEL[s],
                error_kw={"ecolor": INK2, "elinewidth": 0.8, "capsize": 2})
    ax.set_yticks(range(len(names)))
    ax.set_yticklabels([f"{n}  (V={summary[n][next(iter(summary[n]))]['row']['V']:,}, "
                        f"E={summary[n][next(iter(summary[n]))]['row']['E']:,})" for n in names], fontsize=7.5)
    ax.invert_yaxis()
    ax.grid(axis="y", visible=False)
    ax.set_xlabel("time per solve (ms)")
    ax.set_title("Adversarial inputs")
    ax.legend(loc="lower right")
    footnote(fig, "Data: results/raw/adversarial.csv · bench/plot_results.py · mean ± 1 sd of 5 runs · "
                  "inputs regenerated from adversary/ with seed 20260928")
    save(fig, out_dir, "fig5_adversarial.png")

def fig_cache(rows, out_dir):
    totals = defaultdict(float)
    solves = {}
    pmus = defaultdict(set)
    for r in rows:
        try:
            count = float(r["count"])
        except ValueError:
            continue  # <not counted> or <not supported>, e.g. the E-core counters of a pinned run
        V, E, s = int(r["V"]), int(r["E"]), r["structure"]
        base = r["event"].split("/")[-2] if r["event"].count("/") >= 2 else r["event"]
        base = base.split(":")[0]
        totals[(V, E, s, base)] += count
        solves[(V, E, s)] = int(r["solves"])  # solves added on top of the "none" run
        if count:
            pmus[(V, E, s)].add(r["event"].split("/")[0] if r["event"].count("/") >= 2 else "")
    if any(len(p) > 1 for p in pmus.values()):
        print("warning: cache_profile.csv has counts from both core types (cpu_core and cpu_atom); "
              "rerun bench/cache_profile.sh so that perf runs pinned to one core", file=sys.stderr)
    sizes = sorted({(V, E) for (V, E, s), n in solves.items() if s in ("A", "B") and n > 0})
    if not sizes:
        return
    fig, axes = plt.subplots(1, 2, figsize=(7.6, 3.6))
    for ax, event, label in ((axes[0], "L1-dcache-load-misses", "L1 data cache"),
                             (axes[1], "LLC-load-misses", "Last-level cache (L3)")):
        w = 0.38
        for i, s in enumerate(("A", "B")):
            vals = []
            for (V, E) in sizes:
                n = solves.get((V, E, s), 0)
                net = totals.get((V, E, s, event), 0) - totals.get((V, E, "none", event), 0)
                vals.append(max(net, 0) / n / E if n else 0)
            ax.bar([k + (i - 0.5) * (w + 0.03) for k in range(len(sizes))], vals, width=w, color=COLOR[s],
                   label=LABEL[s])
        ax.set_xticks(range(len(sizes)))
        ax.set_xticklabels([f"V={V:,}\nE={E:,}" for V, E in sizes], fontsize=7)
        ax.grid(axis="x", visible=False)
        ax.set_title(label)
        ax.set_xlabel("random graph size")
        ax.set_ylabel("load misses per edge, per solve")
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="upper center", ncol=2, fontsize=8, bbox_to_anchor=(0.5, 1.06))
    footnote(fig, "Data: results/raw/cache_profile.csv (perf stat, user space, one P-core) · bench/plot_results.py · "
                  "(run − 'none' run) ÷ solves column ÷ E")
    fig.tight_layout()
    save(fig, out_dir, "fig6_cache_misses.png")

def table_counters(rows, out_dir):
    if not any(int(r[c]) for r in rows for c in ("half_edges_scanned", "uf_finds", "bfs_runs")):
        print("counters.csv is all zero: the BENCH_* calls are not in A, B and the oracle yet; "
              "skipping table_counters.md", file=sys.stderr)
        return
    preds = {
        "A": {"vertices_visited": lambda V, Ep, C, b: V, "half_edges_scanned": lambda V, Ep, C, b: 2 * Ep,
              "edge_stack_pushes": lambda V, Ep, C, b: Ep},
        "B": {"vertices_visited": lambda V, Ep, C, b: V, "half_edges_scanned": lambda V, Ep, C, b: 2 * Ep,
              "uf_unions": lambda V, Ep, C, b: V - C - b,
              "uf_finds": lambda V, Ep, C, b: 3 * V + 3 * Ep - 2 * C - 5 * b},
        "oracle": {"bfs_runs": lambda V, Ep, C, b: 1 + V + Ep,
                   "half_edges_scanned": lambda V, Ep, C, b: 2 * Ep * (V + Ep),
                   "vertices_visited": lambda V, Ep, C, b: V * (V + Ep)},
    }
    lines = ["| V | Structure | Counter | Predicted | Measured | Match |", "|---|---|---|---|---|---|"]
    mismatches = 0
    depth = {}
    for r in sorted(rows, key=lambda r: (r["V"], r["structure"])):
        s, V = r["structure"], r["V"]
        Ep = r["E"] - r["self_loops"]
        C, b = r["components"], r["bridges"]
        for counter, f in preds.get(s, {}).items():
            p, m = f(V, Ep, C, b), int(r[counter])
            ok = p == m
            mismatches += not ok
            lines.append(f"| {V:,} | {s} | `{counter}` | {p:,} | {m:,} | {'yes' if ok else '**NO**'} |")
        if s in ("A", "B"):
            depth[(V, s)] = int(r["max_stack_depth"])
        if s == "B" and int(r["uf_finds"]):
            steps = int(r["uf_find_steps"]) / int(r["uf_finds"])
            lines.append(f"| {V:,} | B | `uf_find_steps / uf_finds` | below 3, flat in V | {steps:.3f} | "
                         f"{'yes' if steps < 3 else '**NO**'} |")
    for V in sorted({v for v, _ in depth}):
        a, b2 = depth.get((V, "A")), depth.get((V, "B"))
        if a is not None and b2 is not None:
            lines.append(f"| {V:,} | A, B | `max_stack_depth` | A = B, > V/2 | A = {a:,}, B = {b2:,} | "
                         f"{'yes' if a == b2 and a > V / 2 else '**NO**'} |")
    text = ("# Counter predictions (REPORT.md §9.2) against the counting build\n\n"
            "Generated by bench/plot_results.py from results/raw/counters.csv. "
            f"{mismatches} exact prediction(s) do not match.\n\n" + "\n".join(lines) + "\n")
    path = os.path.join(out_dir, "table_counters.md")
    with open(path, "w") as f:
        f.write(text)
    print("wrote", path)

def main():
    ap = argparse.ArgumentParser(description="Draw results/figures from results/raw CSV files.")
    ap.add_argument("--raw", default="results/raw")
    ap.add_argument("--out", default="results/figures")
    args = ap.parse_args()
    os.makedirs(args.out, exist_ok=True)

    scaling = read_csv(os.path.join(args.raw, "scaling.csv"))
    if not scaling:
        sys.exit(f"no {args.raw}/scaling.csv: run bench/run_benchmarks.sh first")
    s_sum = summarize(scaling)
    check_reps(s_sum, "scaling.csv")
    full_rows = read_csv(os.path.join(args.raw, "fullscale.csv"))
    full = summarize(full_rows) if full_rows else None

    fig_time_vs_v(s_sum, args.out)
    fig_ns_per_edge(s_sum, full, args.out)
    fig_oracle(s_sum, args.out)
    fig_ratio(s_sum, args.out)
    adv = read_csv(os.path.join(args.raw, "adversarial.csv"))
    if adv:
        fig_adversarial(adv, args.out)
    cache = read_csv(os.path.join(args.raw, "cache_profile.csv"))
    if cache:
        fig_cache(cache, args.out)
    counters = read_csv(os.path.join(args.raw, "counters.csv"))
    if counters:
        table_counters(counters, args.out)

if __name__ == "__main__":
    main()
