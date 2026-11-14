#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

if [[ -s Makefile ]]; then
    make
    make test
else
    echo "run_all: Makefile is empty, skipping build and tests" >&2
fi

bash bench/run_benchmarks.sh

if command -v perf >/dev/null; then
    bash bench/cache_profile.sh
else
    echo "run_all: perf not installed, skipping the cache profile (Arch: sudo pacman -S perf)" >&2
fi

python3 bench/plot_results.py
