# REPORT
## 7. Correctness

### 7.4 Bug log

Built from every commit in the repository (all branches, 47 commits, of which 31 are non-merge), the pull-request
reviews, and the review notes kept during the project. "Found" is the date the bug was first seen; each fixed bug
links to the commit that fixed it. Where the evidence doesn't show how a bug was found, the entry says so and the
owner should confirm.

#### Fixed

| # | Found | Module | Symptom | How found | Fixed in |
|---|-------|--------|---------|-----------|----------|
| 1 | 2026-10-17 | oracle | Link error: `count_components` was defined in a header without `inline` or an include guard, so any program including `oracle/brute_force.hpp` from two `.cpp` files failed with "multiple definition of count_components". Introduced in [`144a453`](https://github.com/cse250001026/cs253-oops-p04/commit/144a453). | Review; confirmed by compiling two translation units that include the header. | [`429b862`](https://github.com/cse250001026/cs253-oops-p04/commit/429b862) (2026-10-17): moved into `class BfsOracle` with guard `BFS_ORACLE_HPP`; member functions defined in the class are implicitly `inline`. |
| 2 | 2026-10-17 | oracle | Edge deletion did nothing: `count_comp(g, delNode, delEdge, …)` never used `delEdge` (`-Wextra` reported it unused), so a "removed" edge was still traversed, removing an edge could never raise the component count, and no bridge would ever have been reported. The first draft also named the removed edge by its endpoint pair, which cannot tell two parallel edges apart. Introduced in [`144a453`](https://github.com/cse250001026/cs253-oops-p04/commit/144a453), carried into [`429b862`](https://github.com/cse250001026/cs253-oops-p04/commit/429b862). | Review, helped by the `-Wextra` unused-parameter warning. No wrong answer was ever produced, because the fix landed with the first bridge loop. | [`217f6bf`](https://github.com/cse250001026/cs253-oops-p04/commit/217f6bf) (2026-10-18): the BFS skips the edge by ID (`e.id == delEdge`), so deleting one parallel copy leaves the other. |
| 3 | 2026-10-18 | oracle | Performance: `count_comp` took `vis` and `que` **by value**, copying two O(V) arrays on each of its 1 + V + E′ calls and reallocating `vis` inside. Introduced in [`429b862`](https://github.com/cse250001026/cs253-oops-p04/commit/429b862). | Code review *(Shubham to confirm)*. | [`217f6bf`](https://github.com/cse250001026/cs253-oops-p04/commit/217f6bf) (2026-10-18): pass by reference; reset with `fill`. |
| 4 | 2026-10-28 | structB | Complexity: the DSU had path compression only, and path marking linked sets directly (`dsu.rep[x] = dsu.find(parent[x])`). The code therefore did not achieve the O((V + E) α(V)) bound that D2 must prove, only O(log V) amortised per find. Union by rank cannot simply be added, because the marking loop needs each set's root to be its shallowest vertex. | Review of PR `struct2` against the D2 bound. | [`6c7fb89`](https://github.com/cse250001026/cs253-oops-p04/commit/6c7fb89) added union by rank to `DSU::unite`, which path marking still bypassed; [`ebc8b0e`](https://github.com/cse250001026/cs253-oops-p04/commit/ebc8b0e) (2026-10-29) completed the fix: `highest[]` stores each set's shallowest vertex, and marking calls `get_highest()` and `unite(x, parent[x], depth)`. |
| 5 | 2026-10-28 | structB | Test that could never fail: `src/structB/test_structB.cpp` only printed bridges and 2-ECCs, asserted nothing, and compiled only with `-DSTRUCTB_STANDALONE`, so a broken Structure B would still exit 0. | Review of PR `struct2`; re-checked 2026-11-09. | [`014f4f5`](https://github.com/cse250001026/cs253-oops-p04/commit/014f4f5) (2026-11-09): replaced by `tests/test_structB.cpp`, which asserts fixed cases, agreement with A and the oracle, the 2-ECC partition and bridge invariants, and runs 10^5-vertex stress tests. |
| 6 | 2026-11-01 | structA | Interface mismatch: Structure A exposed a free function `runStructA()` with include guard `STRUCT_A_HPP`, but INTERFACE.md §2.1 defines `class TarjanSolver { static StructAResult solve(const Graph&); }` with guard `TARJAN_LOWLINK_HPP`. Any caller written to the spec (application, differential test, benchmark harness) failed to compile with "'TarjanSolver' has not been declared". The code reached main in this state through PR #9 (2026-11-02). The algorithm itself was correct. | Review of `feat/structA` against INTERFACE.md; confirmed 2026-11-06 by compiling a program written only from INTERFACE.md. | [`3c71398`](https://github.com/cse250001026/cs253-oops-p04/commit/3c71398) (2026-11-10, PR #14): added `TarjanSolver::solve` (calling `runStructA`, which the tests keep using) and renamed the guard. |
| 7 | 2026-11-06 | structB | `UnionFindBridgeSolver::solve()` sorted `bridges` by endpoints, while `bridge_ids`, Structure A and the oracle list bridges by edge ID, so `bridges[i] ≠ edges[bridge_ids[i]]`. On 5,000 random graphs, A's and B's `bridges` lists differed in 1,218 cases. Minimal case, reproduced on `a374cae`: with edge 0 = (2,3) and edge 1 = (0,1), both bridges, A returns (2,3), (0,1) and B returned (0,1), (2,3). `bridge_ids` agreed, so every check that compared only IDs passed. The `include/graph.hpp` comment "Sorted canonical Edge objects" invites this sort (see open item 9). | Comparing A's and B's complete outputs on 5,000 random graphs; then guarded by the invariant checker in `tests/test_structB.cpp` (`bridges[i] == g.edges[bridge_ids[i]]`). | [`014f4f5`](https://github.com/cse250001026/cs253-oops-p04/commit/014f4f5) (2026-11-09): sort removed; `bridges` follows `bridge_ids`. |

#### Found, not yet fixed

| # | Found | Module | Symptom | How found | Status |
|---|-------|--------|---------|-----------|--------|
| 8 | 2026-11-11 | adversary | `gen_path` at its defaults does not overflow a recursive DFS, which is the spec's target. Labels are shuffled, so vertex 0 (the DFS root) falls mid-path and the DFS depth is only 60,102. A recursive Tarjan with an 8 MB stack (Linux, g++ 16.2) finishes at both `-O0` and `-O2`. It does overflow at 10^6 vertices, and on `gen_root path 100000` (depth 99,999) at `-O0`. Structures A and B agree on all of these, and match the oracle on the smaller versions. | Adversarial input, run against a recursive reference DFS. Re-checked 2026-11-13. | Open: start the path at vertex 0 and raise the default n, or document which input and stack size overflow. |
| 9 | 2026-11-06 | include | `include/graph.hpp` lines 68, 78 and 88 describe `bridges` as "Sorted canonical Edge objects", without saying sorted by what. All three modules now return them in `bridge_ids` order. This wording is what led to bug 7. | Review. | Open: reword to "in `bridge_ids` order". |
| 10 | 2026-11-11 | oracle | Test gap: the differential test compares only `articulation_points`, `bridge_ids` and `components_after_removal`. It would not catch bug 7, or wrong `bridges`, `two_eccs` or `bcc_*` output. The invariant checkers exist (`verifyBccInvariants` in `tests/test_structA.cpp`, `verifyStructBInvariants` in `tests/test_structB.cpp`) but are not called from it. | Review against INTERFACE.md §3. | Open: call the checkers on every graph in the differential test. |

No wrong-answer bug was found in Structure A's algorithm: from its first commit ([`5361f19`](https://github.com/cse250001026/cs253-oops-p04/commit/5361f19)) it matched the oracle and
Structure B, including on 20,000 random graphs with parallel edges, self-loops and disconnected parts. Owners should
add any bug they found and fixed before committing, with the date, how it was found and the evidence.

## 9. Predicted cost model

### 9.1 Asymptotic model

#### Structure A — O(V + E)
Tarjan's iterative DFS visits every vertex once and every edge once.
Total operations = c1·V + c2·E for small constants c1, c2.
With E = 5V (sparse graph), this becomes c1·V + 5·c2·V, which is still
Θ(V): linear in V (big-O drops the constant factor).
Predicted: time scales linearly as V grows from 10^3 to 10^5.

#### Structure B — O((V + E)·α(V))
Same DFS traversal as A with union-find operations added.
α(V) is the inverse Ackermann function — grows so slowly it is
effectively constant (≤ 5) for all practical V.
Predicted: B runs 10–20% slower than A in practice due to
union-find overhead, but same linear shape.

#### Oracle — O(V · (V + E))
Removes each of V vertices and runs a full BFS costing O(V + E).
Total = O(V^2 + V·E) — quadratic in V.

The bridge check also removes each of the E edges in turn and reruns
BFS (oracle/bfs_oracle.hpp), so the oracle runs 1 + V + E BFS passes in
total: O((V + E)^2) in general. For the sparse graphs benchmarked here
(E = 5V) this is still Θ(V^2), so the O(V(V + E)) growth rate holds, but
the constant is about 6× larger than the vertex-removal count alone.
In terms of the bench/counters.hpp counters, with E = 5V:
- bfs_runs ≈ 1 + V + E ≈ 6V
- half_edges_scanned ≈ 6V · 2E = 60·V^2 (vertex removals alone: 10·V^2)
- at V = 1,000: about 6 × 10^7 half-edge scans; at V = 5,000: about 1.5 × 10^9
Predicted: oracle time grows about 4× each time V doubles.
Predicted: impractical beyond V = 5,000. Curves for A and B
separate visibly from oracle above V = 1,000.

#### Predicted crossover points
- A and B diverge noticeably above V = 10^4
- Oracle becomes impractical above V = 5,000
- Full constants measured and compared in §11 after experiments.

### 9.2 Operation-counter predictions

These are the exact values the counting build (`-DBENCH_COUNT`, see
`bench/counters.hpp`) must report. They are derived from the code, not
from measurements, in derivation D6. A measured count that differs from
its prediction is a bug in the structure or in where the counter is
placed, not noise.

Notation: V vertices, E edges, L self-loops, E' = E − L non-loop edges,
C connected components, T = V − C DFS-forest edges, b bridges.
`graph.hpp` keeps self-loops out of `adj`, so the adjacency lists hold
exactly 2E' half-edges. The harness records V, E, L, C and b in every
CSV row, so each prediction can be evaluated exactly.

| Counter | Structure | Predicted value | Reason |
|---|---|---|---|
| `vertices_visited` | A, B | V | each vertex is discovered once |
| `half_edges_scanned` | A, B | 2E' | the DFS loop reads every adjacency entry once (counted in the DFS loop only; A's later `bcc_vertices` pass reads another 2E') |
| `max_stack_depth` | A, B | A = B exactly; > V/2 on the E = 5V random graphs | both scan `adj` in the same order from roots 0, 1, 2, …, so they build the same DFS forest; a DFS on a random graph of average degree 10 follows one long path before backtracking (Enriquez, Faraud and Ménard, 2020) |
| `edge_stack_pushes` | A | E' | a tree edge is pushed when its child is discovered; a back edge only from its deeper endpoint |
| `uf_unions` | B | V − C − b | one per tree edge that lies on a cycle; each is marked once |
| `uf_finds` | B (both DSUs) | 3V + 3E' − 2C − 5b (≈ 18V at E = 5V) | path marking: 3 finds per marked edge, 1 per non-tree edge; 2-ECC grouping: 2 per non-bridge edge, then 1 per vertex |
| `uf_find_steps` / `uf_finds` | B | below 3, and flat as V grows | union by rank with path compression: amortised O(α(V)), and α(V) ≤ 4 here |
| `bfs_runs` | oracle | 1 + V + E' | one pass with no deletion, one per vertex, one per non-loop edge |
| `half_edges_scanned` | oracle | 2E'(V + E') | 2E' − deg(v) per vertex removal, 2E' per edge removal |
| `vertices_visited` | oracle | V(V + E') | V per pass, V − 1 per vertex removal |

Where they separate, in counter terms: A and B read 2E' half-edges and the
oracle reads 2E'(V + E'), a ratio of V + E' (6V at E = 5V). At V = 10^3 the
oracle does 6 × 10^3 times A's traversal work; at V = 5 × 10^3, 3 × 10^4
times. On log-log axes A and B have slope 1 and the oracle slope 2, so the
gap grows linearly in V and never closes.

### 9.3 Cache prediction

Working set at E = 5V, per vertex:
- the graph itself, about 164 bytes: 10 half-edges × 8 B plus a 24 B vector
  header for `adj`, and 5 edges × 12 B for `edges`;
- A's own arrays (`tin`, `low`, `parent`, `parentEdge`, `next`, `separated`,
  `bccId`, the bridge flags and both stacks), about 75 bytes.

That is about 240 bytes per vertex in total, before vector capacity slack.
On the measurement machine (§10):
- it fits in one P-core's L2 (1.25 MiB) up to V ≈ 5 × 10^3;
- it fits in L3 (24 MiB) up to V ≈ 10^5;
- the full-scale input (V = 10^5, E = 10^6, about 35 MB) does not fit in L3.

Predicted:
- Time per (V + E) for A and B is flat up to V ≈ 5 × 10^3. It rises from
  there to V = 10^5 as the DFS's random accesses miss L2 and hit L3, and
  rises again at the full-scale point, where they go to DRAM.
  `LLC-load-misses` per edge (perf, `bench/cache_profile.sh`) rises at the
  same sizes.
- The oracle's graph fits in L2 for every V it runs on (≈ 0.8 MB at
  V = 5 × 10^3). At 1–3 ns per half-edge read it needs ≈ 0.06–0.18 s per
  run at V = 10^3 and ≈ 1.5–4.5 s at V = 5 × 10^3.
- A and B stay in tens of milliseconds up to V = 10^5.

## 10. Experimental setup

- **Machine:** 13th Gen Intel Core i7-13650HX: 6 P-cores and 8 E-cores,
  20 threads, up to 4.9 GHz. Each P-core has 48 KiB L1d and 1.25 MiB L2;
  the 24 MiB L3 is shared; 15 GiB RAM.
- **Pinning:** every timed run is pinned to one P-core with
  `taskset -c 2` (P-cores are CPUs 0–11). This stops the hybrid scheduler
  moving a run onto an E-core, whose clock and L2 differ. The CPU governor
  (`powersave` by default) is recorded in `results/raw/machine.txt`.
- **Software:** Arch Linux, kernel 7.2.14-arch1-1, g++ (GCC) 16.2.1.
  - Timing build: `-O2 -std=c++17`.
  - Counting build: adds `-DBENCH_COUNT` and is never used for timing.
- **Inputs:**
  - Random multigraphs with E = 5V: endpoints uniform over the V vertices,
    self-loops and parallel edges allowed. V ∈ {10^3, 2·10^3, 3·10^3,
    4·10^3, 5·10^3, 10^4, 2·10^4, 5·10^4, 10^5}.
  - The oracle runs for V ≤ 5 × 10^3.
  - Full-scale point: V = 10^5, E = 10^6.
  - Adversarial inputs come from the `adversary/` generators at their
    default sizes. They are generated by the script on every run and
    never committed.
- **Seeds:** the random graph for size V uses seed 20260928 + V. Every CSV
  row records its seed.
- **Repetitions:** each point gets one untimed warm-up and then 5 timed
  runs. Figures show the mean, with error bars of ± one standard deviation.
- **What is timed:** `std::chrono::steady_clock` around `solve()` only;
  graph construction and the agreement checks are excluded.
- **Agreement:** every run checks B's `bridge_ids` against A. Wherever the
  oracle runs, the harness also checks A's articulation points, bridges and
  `components_after_removal` against the oracle. The result goes in the
  `agree` column.
- **Data:** `bench/run_benchmarks.sh` writes `results/raw/<experiment>.csv`,
  and `bench/plot_results.py` draws `results/figures/`. Both run from
  `scripts/run_all.sh`.
