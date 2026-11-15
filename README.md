# Project 04: Critical Nodes and Links in a Network

Team Oops — CS 253 / MA 253, semester 3 (2026)

## Team and roles
| Name               | Roll no.   | GitHub                   | Role              | Module folder | Derivation |
|--------------------|------------|--------------------------|-------------------|---------------|------------|
| Chetna Mundra      | 250001024  | @cse250001024            | Structure A owner | src/structA/  | D1         |
| Mayank Gupta       | 250001047  | @cse250001047-MG         | Structure B owner | src/structB/  | D2         |
| Sufiyan Ahmad      | 250001068  | @Sufiyan-cse-250001068   | Architect         | app/          | D3         |
| Shubham Sharma     | 250051017  | @ep250051017-shubham     | Oracle owner      | oracle/       | D4         |
| Raunak Kumar Singh | 250001060  | @cse250001060-collab     | Adversary         | adversary/    | D5         |
| Deepak Chahar      | 250001026  | @cse250001026            | Analyst           | bench/        | D6         |

## Input and output assumptions
The course documents do not give an input and output format for the Project 04 hidden tests, so the
evaluation program `bin/run_ops` and the application `bin/app` work on the assumptions below. If the
instructor publishes a format, `run_ops` will be changed to follow it.

**Input**
- The first line is `n m`, the number of vertices and of edges, followed by the m edges as `u v`, one per
  line (any whitespace separates the numbers).
- The graph is undirected. Vertices are numbered 0 … n − 1, and an edge's ID is its position in the
  input, counting from 0.
- Parallel edges, self-loops, isolated vertices and several connected components are all allowed:
  - a self-loop keeps its edge ID but never changes an answer: it is never a bridge and belongs to no
    biconnected component;
  - an edge that appears twice is never a bridge;
  - an isolated vertex is never an articulation point and forms a 2-edge-connected component of its own;
  - in a disconnected graph, a vertex is an articulation point when deleting it increases the number of
    connected components.
- After the edges, `run_ops` reads optional commands (below) and `app` reads optional queries
  `separates v x y`.
- Full-scale graphs (10^5 vertices, 10^6 edges) take about 0.2 s with either program.

**Output of `run_ops`**

Every list is printed on one line in ascending order, with the numbers separated by single spaces; an
empty list prints an empty line. Without commands, `run_ops` prints every result of the chosen structure.
Each command after the edges prints one result, without a header line.

| Command | `--structure` | Prints |
|---|---|---|
| none | A | `ARTICULATION_POINTS k`, then the k vertices; `BRIDGES k`, then the k bridge edge IDs; `BCCS k`, then one line per biconnected component with its edge IDs, components ordered by their smallest edge ID |
| none | B | `BRIDGES k`, then the k bridge edge IDs; `TWO_ECCS k`, then one line per 2-edge-connected component with its vertices, components ordered by their smallest vertex |
| `AP` | A | the articulation points |
| `BRIDGES` | A or B | the bridge edge IDs |
| `BCCS` | A | the number of biconnected components, then one line each (edge IDs) |
| `TWO_ECCS` | B | the number of 2-edge-connected components, then one line each (vertices) |
| `ROBUSTNESS` | A | n numbers: how many connected components are left after deleting vertex 0, 1, …, n − 1 |
| `SEPARATES v x y` | A | `1` if x and y are connected and every path between them passes through v, otherwise `0` (also when v is x or y, or x = y) |

`--structure` defaults to A. A structure ignores commands it does not support. Command names are upper
case as shown; `articulation_points`, `bridges`, `bccs`, `two_eccs`, `robustness` and `separates` are also
accepted. The input is not validated (see Known limitations).

## Summary
A network is fragile at its articulation points and bridges, the nodes and links whose failure splits it;
we find both, with the biconnected components, on graphs of up to 10^5 vertices and 10^6 edges, and rank
the nodes by how many pieces their failure leaves. Structure A is Tarjan's low-link DFS, written
iteratively; Structure B finds the bridges a second way, by marking cycle edges on a DFS tree with
union-find; and a block-cut tree built from A's components answers "does the failure of v separate x from
y". A and B agree on every input and match a brute-force BFS oracle wherever it can run (V ≤ 5,000).
Both scale linearly, taking 54 ms (A) and 43 ms (B) at full scale, while the oracle grows faster than
quadratically and already needs 3.2 s at V = 5,000. B executes twice A's instructions and is 5–27 %
slower while the graph fits in cache, but about 20 % faster at full scale, where A's extra memory traffic
dominates.

## Requirements
- OS: Arch Linux, kernel 7.2.14 (x86-64). Any Linux with the tools below should work.
- Compiler: g++ (GCC) 16.2.1 with `-std=c++17 -O2 -Wall -Wextra`; GNU Make 4.4.1.
- Libraries: none. The structures, the oracle and the tests use only the C++ standard library.
- For the experiments only:
  - Bash 5.3 and `taskset` (util-linux 2.42), to pin timed runs to one core;
  - Linux `perf` 7.3, for the cache profile (optional: `run_all.sh` skips it if `perf` is missing);
  - Python 3.14 with matplotlib 3.11, for the figures.

## Build
    make            # builds every program into bin/

This builds the tests (`test_structA`, `test_structB`, `oracle_tests`, `differential_test`), the
benchmark driver (`bench`, plus `bench_count` with operation counters), the five adversarial
generators (`gen_*`), and the application `app` and evaluation program `run_ops`.

## Quick start
    make
    ./bin/app --input examples/small.txt

`examples/small.txt` holds a 9-vertex network: the triangle 0–1–2, joined by the bridge 2–3 to the
triangle 3–4–5, then the bridges 3–6 and 6–7, and the isolated vertex 8. The file uses the input format
in [Input and output assumptions](#input-and-output-assumptions), followed by five queries
`separates v x y`. The application runs Structures A and B,
checks that their bridges agree, ranks the vertices by the number of components left when each one fails
(`--top K` shows the first K, default 10), and answers each query with the block-cut tree: YES when x and
y are connected and every path between them passes through v. Expected output:

    Network Summary
      Vertices:              9
      Edges:                 9
      Connected components:  2
      Articulation points:   3
      Bridges (A / B):       3 / 3 [OK]
      Biconnected blocks:    5
      Block-cut tree nodes:  8 (3 cut nodes, 5 block nodes)

    Top 9 Critical Nodes (by components after removal):
      Rank  Vertex  Component_Pieces  Total_Components
      1     3       3                 4
      2     2       2                 3
      3     6       2                 3
      4     0       1                 2
      5     1       1                 2
      6     4       1                 2
      7     5       1                 2
      8     7       1                 2
      9     8       0                 1

    Separation Queries:
      separates(3, 0, 4) -> YES
      separates(3, 4, 5) -> NO
      separates(2, 0, 6) -> YES
      separates(6, 5, 7) -> YES
      separates(1, 0, 2) -> NO

## Tests
    make test

`make test` runs the Structure A and Structure B tests (fixed cases, invariant checks and stress tests)
and the oracle tests, then the differential test, which compares A, B and the oracle on 10^6 operations
with seed 20260928. On every graph it also checks the graph invariants and the block-cut tree's (it
alternates cut and block nodes and is a forest), and on graphs with at most 15 vertices it checks every
separation answer against a BFS. A passing run ends with:

    DIFFERENTIAL TEST PASSED: 1000000 operations, seed 20260928

    ALL TESTS PASSED

The adversarial checks run separately (about 5 s):

    bash scripts/adversary.sh   # small version of every adversarial graph against the oracle,
                                # full-size versions A against B; ends with ADVERSARY CHECKS PASSED

## Evaluation interface (hidden tests)
    ./bin/run_ops --structure A < ops.txt > out.txt
    ./bin/run_ops --structure B < ops.txt > out.txt

`ops.txt` and `out.txt` follow the format in [Input and output assumptions](#input-and-output-assumptions).
On the Quick start graph without its queries (`examples/small_graph.txt`):

    $ ./bin/run_ops --structure A < examples/small_graph.txt
    ARTICULATION_POINTS 3
    2 3 6
    BRIDGES 3
    3 7 8
    BCCS 5
    0 1 2
    3
    4 5 6
    7
    8

    $ ./bin/run_ops --structure B < examples/small_graph.txt
    BRIDGES 3
    3 7 8
    TWO_ECCS 5
    0 1 2
    3 4 5
    6
    7
    8

With commands after the edges:

    $ { cat examples/small_graph.txt; printf 'ROBUSTNESS\nSEPARATES 3 0 4\nSEPARATES 1 0 2\n'; } | ./bin/run_ops --structure A
    2 2 3 4 2 2 3 2 1
    1
    0

## Benchmarks and figures
    ./scripts/run_all.sh        # about 1.5 minutes: build, test, run the app, experiments, figures

`run_all.sh` builds everything, runs `make test`, runs the application on `examples/small.txt`, writes
the raw data to `results/raw/` and redraws `results/figures/` from it. The steps can also be run one at a
time:

    make bench      # timings and operation counts: scaling, full scale, counters, adversarial (about 1 min)
    make cache      # perf cache profile, results/raw/cache_profile.csv (a few seconds)
    make figures    # redraw results/figures/ from results/raw/

Timed runs are pinned to CPU 2. On another machine set `BENCH_CPU` to a performance core, for example
`BENCH_CPU=0 make bench`. `BENCH_REPS` sets the timed runs per point (default 5), and `QUICK=1` runs a
short version. The benchmark driver also works on its own:

    ./bin/bench --random 1000 5000 20261928 --structures A,B,oracle --reps 5 --header
    ./bin/bench --input graph.txt --structures A,B     # graph.txt: "n m", then m lines "u v"

## Repository map
```
cs253-oops-p04/
├── README.md         build, run and test instructions (this file)
├── REPORT.md         the report (REPORT.pdf at the final tag)
├── INTERFACE.md      common interface and structural invariants
├── TOOLS.md          AI and other tools, with representative prompts
├── Makefile          make, make test, make bench, make cache, make figures
├── include/          graph.hpp: the shared Graph and result classes
├── src/structA/      Structure A: iterative Tarjan low-link DFS
├── src/structB/      Structure B: DFS forest, union-find path marking, 2-edge-connected components
├── app/              robustness report, block-cut tree and run_ops (evaluation interface)
├── oracle/           BFS oracle, differential test, failure shrinker, graph invariant checks
├── adversary/        five adversarial graph generators and their checker (check_adversary.cpp)
├── bench/            benchmark driver, operation counters, experiment and plotting scripts
├── examples/         small inputs used in this README (small.txt, small_graph.txt)
├── tests/            tests for Structure A, Structure B and the oracle
├── scripts/          run_all.sh (everything) and adversary.sh (adversarial checks)
├── results/raw/      raw data, one CSV per experiment, and machine.txt
├── results/figures/  figures and the counter table, drawn from results/raw/
└── docs/             derivations/ (D1–D6) and slides/slides.pdf
```

## Results at a glance
One Intel Core i7-13650HX laptop, one P-core, g++ `-O2`; mean of 5 runs. Full analysis in REPORT.md §11.

| Input | Structure A | Structure B | Oracle |
|---|---:|---:|---:|
| Random graph, V = 10^3, E = 5 × 10^3 | 0.11 ms | 0.13 ms | 63 ms |
| Random graph, V = 5 × 10^3, E = 2.5 × 10^4 | 0.64 ms | 0.67 ms | 3.19 s |
| Random graph, V = 10^5, E = 5 × 10^5 | 28.1 ms | 25.3 ms | not run (over an hour) |
| Full scale, V = 10^5, E = 10^6 | 53.8 ms | 42.8 ms | not run |
| Slowest adversarial input (`gen_multi`, V = 10^5) | 26.2 ms | 15.3 ms | not run |
| Growth with V (log–log slope, V ≤ 5 × 10^3) | 1.09 | 1.01 | 2.43 |

- A and B agree on every input, and both match the oracle on every graph it can run (V ≤ 5,000).
- All 96 checks of the operation counts predicted before measuring (REPORT.md §9.2) pass.
- B is 5–27 % slower than A while the graph fits in cache and about 20 % faster at full scale.

## Known limitations
- The oracle runs only up to V = 5,000, because its cost grows as (V + E)^2. On larger graphs A and B are
  checked only against each other; `scripts/adversary.sh` checks a small version of every adversarial
  shape against the oracle.
- The differential test compares articulation points, bridge IDs and the components left after each
  removal. The other outputs (`bridges`, biconnected components, 2-edge-connected components) are checked
  by the invariant tests on their own random graphs (REPORT.md §7.4, bug 10).
- Timings come from one laptop and vary by up to 15 % between runs. A and B are timed alternately in one
  process, which shifts their ratio by about 10 % at V = 10^5 (REPORT.md §11.3 and §12).
- `app` and `run_ops` do not check their input: a vertex outside 0 … n − 1 crashes them, and an edge
  list shorter than its header is not reported. (`bench/bench.cpp` checks both.)
- The cache profile needs Linux `perf` with access to the hardware counters; without it, `run_all.sh`
  skips that step.

## Tools and references
AI assistants and other tools are declared in [TOOLS.md](TOOLS.md); the full reference list is in
REPORT.md §13. The main papers:
- R. E. Tarjan, "Depth-first search and linear graph algorithms," *SIAM Journal on Computing* 1(2):146–160, 1972 (Structure A).
- J. Hopcroft and R. E. Tarjan, "Algorithm 447: Efficient algorithms for graph manipulation," *Communications of the ACM* 16(6):372–378, 1973.
- R. E. Tarjan, "A note on finding the bridges of a graph," *Information Processing Letters* 2(6):160–161, 1974 (Structure B).
- R. E. Tarjan, "Efficiency of a good but not linear set union algorithm," *Journal of the ACM* 22(2):215–225, 1975 (the union-find bound).
- F. Harary, *Graph Theory*, Addison-Wesley, 1969, Chapter 3 (the block-cut tree).
- J. M. Schmidt, "A simple test on 2-vertex- and 2-edge-connectivity," *Information Processing Letters* 113(7):241–244, 2013.
