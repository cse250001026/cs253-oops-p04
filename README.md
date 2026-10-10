# README
# Project 04: Critical Nodes and Links in a Network
    Team <team> — Oops ,
    CS 253 / MA 253, 
    semester 3

## Team and roles
| Name               | Roll no.   | GitHub                   | Role              | Module folder | Derivation |
|--------------------|------------|--------------------------|-------------------|---------------|------------|
| Chetna Mundra      | 250001024  | @cse250001024            | Structure A owner | src/structA/  | D1         |
| Mayank Gupta       | 250001047  | @cse250001047-MG         | Structure B owner | src/structB/  | D2         |
| Sufiyan Ahmad      | 250001068  | @Sufiyan-cse-250001068   | Architect         | app/          | D3         |
| Shubham Sharma     | 250051017  | @ep250051017-shubham     | Oracle owner      | oracle/       | D4         |
| Raunak Kumar Singh | 250001060  | @cse250001060-collab     | Adversary         | adversary/    | D5         |
| Deepak Chahar      | 250001026  | @cse250001026            | Analyst           | bench/        | D6         |

## Summary
<!-- Fill after results are in — 3 to 5 sentences: problem, Structure A vs B, main finding -->

## Requirements
    # OS: &lt;e.g. Ubuntu 22.04&gt; - Compiler: &lt;e.g. g++ 11.4, -O2&gt;

## Build
    make
    # or: cmake -B build && cmake --build build

## Quick start
    ./bin/app --input examples/small.txt
    # expected output: ...

## Tests
    make test
    # differential test, 10^6 operations, seed 20260928
    # a passing run ends with: ALL TESTS PASSED

## Evaluation interface (hidden tests)
    ./bin/run_ops --structure A < ops.txt > out.txt
    ./bin/run_ops --structure B < ops.txt > out.txt
    # input and output follow the format published for Project 04

## Benchmarks and figures
    ./scripts/run_all.sh
    # ~<N> min; writes results/raw, results/figures

## Repository map
```
cs253-Oops-p04/
├── README.md               project overview, build and test instructions
├── REPORT.md               full report (REPORT.pdf at final tag)
├── INTERFACE.md            common interface and structural invariants
├── TOOLS.md                AI and other tools declared
├── Makefile                build system
├── include/                shared interface header
├── src/
│   ├── structA/            iterative Tarjan low-link DFS
│   └── structB/            union-find bridge detection
├── app/                    robustness report and block-cut tree app
├── oracle/                 BFS oracle and differential test harness
├── adversary/              four adversarial graph generators
├── bench/                  benchmark harness and figures
├── tests/                  unit tests and invariant checks
├── scripts/
│   └── run_all.sh          one command: build, test, benchmarks, figures
├── results/
│   ├── raw/                raw CSV logs, one file per experiment
│   └── figures/            figures generated from results/raw/
└── docs/
    ├── derivations/        D1_rollno.pdf … D6_rollno.pdf
    └── slides/             slides.pdf
```


## Results at a glance
<!-- Fill after measuring — short table of key numbers -->
See REPORT.md for the full analysis.

## Known limitations
<!-- State honestly before final tag — anything incomplete or not working -->

## Tools and references
See TOOLS.md for all tools declared.
Main papers:
