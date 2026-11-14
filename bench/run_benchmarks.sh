#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

RAW=${RAW_DIR:-results/raw}
CPU=${BENCH_CPU:-2}
REPS=${BENCH_REPS:-5}
CXX=${CXX:-g++}
SEED_BASE=20260928
mkdir -p bin "$RAW"

B_SRC=(src/structB/DSU.cpp src/structB/DFSForest.cpp src/structB/makeCyclePath.cpp src/structB/union_find_bridges.cpp)
echo "building bin/bench and bin/bench_count" >&2
"$CXX" -O2 -std=c++17 bench/bench.cpp "${B_SRC[@]}" -o bin/bench
"$CXX" -O2 -std=c++17 -DBENCH_COUNT bench/bench.cpp "${B_SRC[@]}" -o bin/bench_count

PIN=()
if command -v taskset >/dev/null; then PIN=(taskset -c "$CPU"); else echo "taskset missing: runs are not pinned" >&2; fi

{
    echo "date: $(date -Iseconds)"
    echo "pinned_cpu: ${CPU}"
    echo "governor: $(cat /sys/devices/system/cpu/cpu"${CPU}"/cpufreq/scaling_governor 2>/dev/null || echo unknown)"
    echo "kernel: $(uname -r)"
    echo "compiler: $("$CXX" --version | head -1)"
    echo "flags: -O2 -std=c++17 (timing), plus -DBENCH_COUNT (counting)"
    lscpu
} > "$RAW/machine.txt"

if [[ "${QUICK:-0}" == 1 ]]; then
    SIZES=(1000 2000 5000)
else
    SIZES=(1000 2000 3000 4000 5000 10000 20000 50000 100000)
fi

out="$RAW/scaling.csv"; : > "$out"; first=1
for V in "${SIZES[@]}"; do
    echo "scaling V=$V" >&2
    hdr=(); [[ $first == 1 ]] && hdr=(--header); first=0
    "${PIN[@]}" bin/bench --random "$V" $((5 * V)) $((SEED_BASE + V)) --structures A,B,oracle \
        --reps "$REPS" --experiment scaling "${hdr[@]}" >> "$out"
done

if [[ "${QUICK:-0}" != 1 ]]; then
    echo "fullscale V=100000 E=1000000" >&2
    "${PIN[@]}" bin/bench --random 100000 1000000 $((SEED_BASE + 100000)) --structures A,B \
        --reps "$REPS" --experiment fullscale --header > "$RAW/fullscale.csv"
fi

out="$RAW/counters.csv"; : > "$out"; first=1
for V in "${SIZES[@]}"; do
    hdr=(); [[ $first == 1 ]] && hdr=(--header); first=0
    bin/bench_count --random "$V" $((5 * V)) $((SEED_BASE + V)) --structures A,B,oracle \
        --reps 1 --experiment counters "${hdr[@]}" >> "$out"
done

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
declare -a ADV=(
    "gen_path|gen_path_n100000|100000 $SEED_BASE"
    "gen_cycle|gen_cycle_n100000|100000 $SEED_BASE"
    "gen_cliques|gen_cliques_k4_s700|4 700 $SEED_BASE"
    "gen_root|gen_root_star_n100000|star 100000 $SEED_BASE"
    "gen_root|gen_root_twocycles_n100000|twocycles 100000 $SEED_BASE"
    "gen_root|gen_root_path_n100000|path 100000 $SEED_BASE"
    "gen_root|gen_root_cycle_n100000|cycle 100000 $SEED_BASE"
    "gen_root|gen_root_comps_n100000|comps 100000 $SEED_BASE"
    "gen_root|gen_root_clique_n1000|clique 1000 $SEED_BASE"
    "gen_multi|gen_multi_n100000|100000 $SEED_BASE"
)
out="$RAW/adversarial.csv"; : > "$out"; first=1
for entry in "${ADV[@]}"; do
    IFS='|' read -r gen label args <<< "$entry"
    if [[ ! -f "adversary/$gen.cpp" ]]; then
        echo "adversarial: adversary/$gen.cpp not on this branch, skipping $label" >&2
        continue
    fi
    "$CXX" -O2 -std=c++17 "adversary/$gen.cpp" -o "bin/$gen"
    "bin/$gen" $args > "$TMP/$label.txt" 2>/dev/null
    echo "adversarial $label" >&2
    hdr=(); [[ $first == 1 ]] && hdr=(--header); first=0
    "${PIN[@]}" bin/bench --input "$TMP/$label.txt" --name "$label" --structures A,B,oracle \
        --reps "$REPS" --experiment adversarial "${hdr[@]}" >> "$out"
done
[[ -s "$out" ]] || rm -f "$out"

echo "done: CSVs in $RAW" >&2
