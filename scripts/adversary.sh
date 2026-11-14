#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p bin adversary/out

for g in path cycle cliques root multi; do
    g++ -O2 -std=c++17 adversary/gen_$g.cpp -o bin/gen_$g
done
g++ -O2 -std=c++17 adversary/check_adversary.cpp \
    src/structB/union_find_bridges.cpp src/structB/DFSForest.cpp \
    src/structB/DSU.cpp src/structB/makeCyclePath.cpp \
    -o bin/check_adversary

SEED=${SEED:-20260928}
failed=0

run_case() {
    local mode=$1
    shift
    local label="$*"
    local file=adversary/out/case.txt
    ./bin/$label > "$file" 2> adversary/out/case.err
    printf '%-34s ' "$label"
    if ./bin/check_adversary $mode < "$file"; then
        :
    else
        failed=1
    fi
}

echo "== small graphs, checked against the oracle (seed $SEED) =="
for mode in cycle path clique star twocycles comps; do
    run_case oracle gen_root $mode 11 $SEED
done
run_case oracle gen_path 300 $SEED
run_case oracle gen_cycle 300 $SEED
run_case oracle gen_cliques 4 12 $SEED
run_case oracle gen_cliques 5 2 $SEED
run_case oracle gen_multi 300 $SEED

echo
echo "== full scale, Structure A against Structure B =="
run_case plain gen_path 100000 $SEED
run_case plain gen_cycle 100000 $SEED
run_case plain gen_cliques 25000 5 $SEED
run_case plain gen_cliques 4 700 $SEED
run_case plain gen_multi 100000 $SEED

rm -f adversary/out/case.txt adversary/out/case.err
echo
if [ "$failed" -eq 0 ]; then
    echo "ADVERSARY CHECKS PASSED"
else
    echo "ADVERSARY CHECKS FAILED"
    exit 1
fi
