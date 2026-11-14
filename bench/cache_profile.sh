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

# Each size is profiled three times. The "none" run builds the graph and makes
# the one Structure A solve that bin/bench always runs as its reference. The A
# and B runs do the same and add their own solves, so (A - none) and (B - none)
# count only the added solves. The "solves" column says how many there are:
#   none  0
#   A     REPS       the timed solves
#   B     REPS + 1   the timed solves and one untimed warm-up solve
#
# taskset starts perf, not the other way round, so perf and bin/bench run only
# on CPU $CPU. On a hybrid CPU the E-core counters (cpu_atom/...) then read
# <not counted> instead of picking up the start of the run on another core.
out="$RAW/cache_profile.csv"
echo "V,E,seed,structure,solves,event,count" > "$out"
for pair in "${SIZES[@]}"; do
    read -r V E <<< "$pair"
    seed=$((SEED_BASE + V))
    for s in none A B; do
        case $s in
            none) n=0 ;;
            A)    n=$REPS ;;
            B)    n=$((REPS + 1)) ;;
        esac
        echo "perf V=$V E=$E structure=$s" >&2
        taskset -c "$CPU" perf stat -x, -e "$EVENTS" -- bin/bench --random "$V" "$E" "$seed" \
            --structures "$s" --reps "$REPS" 2>&1 >/dev/null \
            | awk -F, -v V="$V" -v E="$E" -v seed="$seed" -v s="$s" -v n="$n" \
                'NF >= 3 && $3 != "" { print V "," E "," seed "," s "," n "," $3 "," $1 }' >> "$out"
    done
done
echo "done: $out" >&2
