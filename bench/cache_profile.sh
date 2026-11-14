#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

command -v perf >/dev/null || { echo "perf not installed (Arch: sudo pacman -S perf)" >&2; exit 1; }
[[ -x bin/bench ]] || { echo "run bench/run_benchmarks.sh first (it builds bin/bench)" >&2; exit 1; }

RAW=${RAW_DIR:-results/raw}
CPU=${BENCH_CPU:-2}
REPS=${BENCH_REPS:-5}
SEED_BASE=20260928
EVENTS=cycles:u,instructions:u,L1-dcache-loads:u,L1-dcache-load-misses:u,LLC-loads:u,LLC-load-misses:u

SIZES=("2000 10000" "20000 100000" "100000 500000" "100000 1000000")

out="$RAW/cache_profile.csv"
echo "V,E,seed,structure,solves,event,count" > "$out"
for pair in "${SIZES[@]}"; do
    read -r V E <<< "$pair"
    seed=$((SEED_BASE + V))
    for s in none A B; do
        echo "perf V=$V E=$E structure=$s" >&2
        perf stat -x, -e "$EVENTS" -- taskset -c "$CPU" bin/bench --random "$V" "$E" "$seed" \
            --structures "$s" --reps "$REPS" 2>&1 >/dev/null \
            | awk -F, -v V="$V" -v E="$E" -v seed="$seed" -v s="$s" -v n=$((REPS + 1)) \
                'NF >= 3 && $3 != "" { print V "," E "," seed "," s "," n "," $3 "," $1 }' >> "$out"
    done
done
echo "done: $out" >&2
