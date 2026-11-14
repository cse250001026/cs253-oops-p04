# Project 04: Critical Nodes and Links in a Network: Report

Team Oops · CS 253 / MA 253 · cs253-oops-p04

<!--
HOW TO FILL THIS REPORT (delete this comment before the final tag)
- Each section starts with a "TO FILL" box naming its owner. Write only in your own sections.
- Replace the box with your text when you are done. Keep the section and sub-section headings.
- Write in your own words. Text from a paper, website or AI tool must be quoted and cited (guidelines 4.4),
  and AI use must also be declared in TOOLS.md.
- Give numbers with units, and sizes as exact values or powers of ten.
- Work on a branch, commit with the prefix "report:", and open a PR. Deadline: Sunday 15 Nov, 20:00.
- Before the final tag, `grep -n "TO FILL" REPORT.md` must print nothing.
Target length: 12-20 pages, excluding appendices. Derivations are separate PDFs and do not count.
-->

## 1. Team and contributions

| Name | Roll no. | GitHub | Role | Module folder | Derivation |
|------|----------|--------|------|---------------|------------|
| Chetna Mundra | 250001024 | @cse250001024 | Structure A owner | `src/structA/` | D1 |
| Mayank Gupta | 250001047 | @cse250001047-MG | Structure B owner | `src/structB/` | D2 |
| Sufiyan Ahmad | 250001068 | @Sufiyan-cse-250001068 | Architect | `app/` | D3 |
| Shubham Sharma | 250051017 | @ep250051017-shubham | Oracle owner | `oracle/` | D4 |
| Raunak Kumar Singh | 250001060 | @cse250001060-collab | Adversary | `adversary/` | D5 |
| Deepak Chahar | 250001026 | @cse250001026 | Analyst | `bench/` | D6 |

> **TO FILL: every member, in your own row below** · ½ page in total
> Three or four bullets on what you built, each linked to the commit or PR that added it. The PRs already merged are listed for you; add any that follow.

**Chetna Mundra (Structure A):** PRs [#9](https://github.com/cse250001026/cs253-oops-p04/pull/9), [#14](https://github.com/cse250001026/cs253-oops-p04/pull/14), [#17](https://github.com/cse250001026/cs253-oops-p04/pull/17).
- …

**Mayank Gupta (Structure B):** PRs [#8](https://github.com/cse250001026/cs253-oops-p04/pull/8), [#10](https://github.com/cse250001026/cs253-oops-p04/pull/10), [#12](https://github.com/cse250001026/cs253-oops-p04/pull/12).
- Iterative DFS forest construction (`DFSForest.cpp`) with explicit heap stack and edge index pointers, preventing call stack overflow on deep paths up to $V = 100,000$ ([#8](https://github.com/cse250001026/cs253-oops-p04/pull/8)).
- Custom Disjoint Set Union (`DSU.cpp`) with two-pass path compression, union-by-rank, and `highest[]` depth tracking to preserve $O(\alpha(V))$ set operations while accessing shallowest path vertices ([#10](https://github.com/cse250001026/cs253-oops-p04/pull/10)).
- Fundamental cycle path marking (`makeCyclePath.cpp`), ID-ordered bridge extraction, and 2-ECC component grouping (`union_find_bridges.cpp`) ([#8](https://github.com/cse250001026/cs253-oops-p04/pull/8), [#12](https://github.com/cse250001026/cs253-oops-p04/pull/12)).
- Operation counter instrumentation matching analytical cost models (`BENCH_*`), invariant test harness (`tests/test_structB.cpp`), and D2 derivation document.

**Sufiyan Ahmad (Architect):** PRs [#2](https://github.com/cse250001026/cs253-oops-p04/pull/2), [#5](https://github.com/cse250001026/cs253-oops-p04/pull/5).
- Designed the shared graph representation (`include/graph.hpp`) and common API specifications (`INTERFACE.md`) for `Graph`, `AdjEdge`, `Edge`, `StructAResult`, `StructBResult`, `OracleResult`, and `NodeRobustness` ([#26](https://github.com/cse250001026/cs253-oops-p04/pull/26), [#27](https://github.com/cse250001026/cs253-oops-p04/pull/27)).
- Implemented the Block-Cut Forest construction, Euler-tour timestamping (`tin`/`tout`), separation query engine (`separates(v, x, y)`), and node robustness ranking in `app/block_cut_tree.hpp`.
- Built the application driver (`app/app.cpp`), hidden-test evaluation driver (`app/run_ops.cpp`), Block-Cut Tree invariant verifier (`check_bct_invariants` in `oracle/invarient_checks.hpp`), and D3 derivation (`docs/derivations/D3_250001068.pdf`).
- …

**Shubham Sharma (Oracle):** PRs [#4](https://github.com/cse250001026/cs253-oops-p04/pull/4), [#7](https://github.com/cse250001026/cs253-oops-p04/pull/7), [#13](https://github.com/cse250001026/cs253-oops-p04/pull/13).
- …

**Raunak Kumar Singh (Adversary):** PR [#16](https://github.com/cse250001026/cs253-oops-p04/pull/16), and the generators PR (merge [`7e9c107`](https://github.com/cse250001026/cs253-oops-p04/commit/7e9c107)).
- …

**Deepak Chahar (Analyst):** PRs [#1](https://github.com/cse250001026/cs253-oops-p04/pull/1), [#6](https://github.com/cse250001026/cs253-oops-p04/pull/6), and the cost-model and harness PR.
- …

## 2. Problem

A network is an undirected graph of nodes and links, and we want to know where it is fragile. A node is
**critical** (an *articulation point*) if removing it splits the network into more connected pieces than
before. A link is critical (a *bridge*) if removing it does the same. The graph also splits into
*biconnected components*: the largest groups of links in which any two links lie on a common cycle, so no single node failure
separates them. A bridge forms a component of its own.

The project must compute all three for a network of 100,000 nodes, and support two uses on top of them:

- **A robustness report** that ranks the critical nodes by how many connected pieces the network falls
  into when each one fails.
- **A query service** that answers "if node v fails, is x cut off from y?". It uses a *block-cut tree*
  built from the biconnected components, and answers each query from DFS entry and exit times on that tree.

Two independent methods must find the same bridges. Structure A uses Tarjan's low-link DFS and also produces
the articulation points and biconnected components. Structure B marks cycle edges on a DFS tree, using
union-find to skip edges already marked. A brute-force oracle removes each node and each link in turn and counts
the pieces by BFS. Both structures must agree with it exactly on every graph small enough for it
(up to about 5,000 nodes).

**Scale.** We worked at the course's full scale for graph projects, 10^5 vertices and 10^6 edges, and
measured A, B and the oracle for V from 10^3 to 10^5. At this size, three things make the problem harder
than the textbook version:

- **Recursion depth.** A recursive DFS overflows the call stack on a path of 10^5 vertices, so both
  structures must be iterative.
- **The DFS root.** The root needs its own rule, and leaving it out gives wrong answers only on particular
  graphs.
- **Messy inputs.** The graphs may contain parallel links, self-loops, isolated nodes and several
  disconnected parts, and every module must handle each case in the same way.

## 3. Interface and invariants

### 3.1 Common interface

All modules communicate through the header `include/graph.hpp` and adhere to the contract established in `INTERFACE.md` before implementation began.

- **Graph representation (`class Graph`):** An undirected multigraph with $V$ vertices labelled $0, 1, \dots, V - 1$ and $E$ edges labelled $0, 1, \dots, E - 1$ in insertion order. Each call to `g.add_edge(u, v)` appends a canonical `Edge(min(u, v), max(u, v), id)` to `g.edges`. If $u \neq v$, it appends `AdjEdge(v, id)` to `g.adj[u]` and `AdjEdge(u, id)` to `g.adj[v]`.
- **Edge-case conventions:**
  - *Parallel edges:* Stored as distinct entries in `edges` and `adj` with unique `id` values. Traversals skip the tree-parent edge by comparing `e.id == parentEdge[u]` rather than `e.to == parent[u]`, so parallel edges form two-edge cycles and are never misclassified as bridges.
  - *Self-loops ($u = v$):* Stored in `g.edges` to keep edge IDs contiguous in $[0, E - 1]$, but omitted from `g.adj` because a loop from a vertex to itself cannot belong to a simple cycle or alter connectivity.
  - *Disconnected graphs and isolated vertices:* Let $C_0$ be the initial number of connected components. Removing vertex $v$ leaves `components_after_removal[v]` components among the remaining $V - 1$ vertices. A vertex $v$ is an articulation point iff `components_after_removal[v] > C_0`. Removing an isolated vertex decreases the component count to $C_0 - 1$, so isolated vertices are never articulation points.
- **Module APIs:**
  - `TarjanSolver::solve(const Graph& g) -> StructAResult` (`src/structA/tarjan_lowlink.hpp`): Returns `articulation_points` (sorted ascending vertex IDs), `bridge_ids` (sorted ascending edge IDs), `bridges` (`Edge` objects in `bridge_ids` order), `bcc_edge_ids` (non-loop edges partitioned into biconnected components, sorted internally and ordered by smallest edge ID), `bcc_vertices` (sorted unique vertices per BCC), and `components_after_removal` (size $V$).
  - `UnionFindBridgeSolver::solve(const Graph& g) -> StructBResult` (`src/structB/union_find_bridges.hpp`): Returns `bridge_ids` (sorted ascending), `bridges` (in `bridge_ids` order), and `two_eccs` (disjoint partition of $\{0, \dots, V - 1\}$ into 2-edge-connected components, each sorted ascending and ordered by smallest vertex ID).
  - `BfsOracle::solve(const Graph& g) -> OracleResult` (`oracle/bfs_oracle.hpp`): Returns `initial_components` ($C_0$), `articulation_points`, `bridge_ids`, `bridges`, and `components_after_removal` by brute-force BFS deletion.
  - `BlockCutTree` (`app/block_cut_tree.hpp`): `build(g, a_res)` constructs the bipartite Block-Cut Forest in $O(V + E)$ time; `separates(v, x, y)` answers whether removing $v$ disconnects $x$ and $y$ using DFS entry/exit timestamps; `get_robustness_ranking()` returns vertices sorted descending by `total_components_after` (ties broken by smaller vertex ID).
- **Input and evaluation formats:**
  - *Generator / benchmark format:* Header line `V E` followed by `E` lines of `u v` ($0 \le u, v < V$).
  - *Evaluation interface (`./bin/run_ops --structure A|B < ops.txt > out.txt`):* Reads `V E` and `E` edges, followed either by EOF (printing standard summary blocks `ARTICULATION_POINTS`, `BRIDGES`, `BCCS` / `TWO_ECCS`) or by explicit query tokens (`AP`, `BRIDGES`, `BCCS`, `TWO_ECCS`, `ROBUSTNESS`, `SEPARATES v x y`).

### 3.2 Structural invariants

| Invariant | Holds for | Checked by (file, function) |
|-----------|-----------|-----------------------------|
| **Graph symmetry & ID integrity:** `edges.size() == E`, `adj.size() == V`, `0 <= edges[id].u <= edges[id].v < V`, non-loop edge `id` appears once in `adj[u]` and once in `adj[v]`, self-loops excluded from `adj` | `Graph` | `oracle/invarient_checks.hpp` (`check_graph_invariants`), `tests/test_structB.cpp` (`verifyGraphInvariants`) |
| **Root & non-root cut rules:** root $r \in \text{AP} \iff \text{children}(r) \ge 2$; non-root $u \in \text{AP} \iff \exists$ child $v$ with $\text{low}[v] \ge \text{tin}[u]$; removal count matches $C_0 - 1 + \text{pieces}(v)$ | Structure A | `tests/test_structA.cpp` (`verifyOracle`), `oracle/differential_test.cpp` (`compare`) |
| **BCC edge partition:** every non-loop edge belongs to exactly one BCC ($\sum |BCC_i| = E'$), edge IDs and vertex lists sorted with no duplicates, BCCs ordered by minimum edge ID | Structure A | `tests/test_structA.cpp` (`verifyBccInvariants`) |
| **Cross-structure bridge equivalence:** `structA.bridge_ids == structB.bridge_ids` and `bridges[i] == g.edges[bridge_ids[i]]` across all graphs up to $V = 10^5$ | Structure A, Structure B | `tests/test_structB.cpp` (`verifyOracleAndStructA`, `verifyStructBInvariants`), `oracle/differential_test.cpp` (`compare`), `bench/bench.cpp` |
| **2-ECC partition & reachability:** `two_eccs` partitions $\{0, \dots, V - 1\}$ ($\sum |C_i| = V$), every bridge connects distinct 2-ECCs, every non-bridge non-loop edge stays within one 2-ECC, components match $G \setminus \text{Bridges}$ | Structure B | `tests/test_structB.cpp` (`verifyStructBInvariants`) |
| **Oracle agreement ($V \le 5000$):** `articulation_points`, `bridge_ids`, and `components_after_removal` match brute-force BFS vertex/edge deletion | Structure A, Structure B, Oracle | `oracle/differential_test.cpp` (`compare`), `tests/oracle_tests.cpp` (`chk`), `tests/test_structA.cpp` (`verifyOracle`) |
| **Block-cut tree bipartite, acyclic & query equivalence:** every BCT edge joins one C-node and one B-node, each BCT component with $n_C$ C-nodes and $n_B$ B-nodes has $n_C + n_B - 1$ edges, and `separates(v, x, y)` matches BFS reachability in $G \setminus \{v\}$ | Application (`BlockCutTree`) | `oracle/invarient_checks.hpp` (`check_bct_invariants`) |

### 3.1 Common interface

### 3.2 Structural invariants

| Invariant | Holds for | Checked by (file, function) |
|-----------|-----------|-----------------------------|
| | | |

## 4. Structure A: iterative Tarjan low-link DFS

> **TO FILL: Chetna** · 2–3 pages
> Code: `src/structA/tarjan_lowlink.hpp`, `TarjanSolver::solve` (calls `runStructA`).

### 4.1 Representation
> The arrays kept (`tin`, `low`, `parent`, `parentEdge`, `next`, `separated`, the explicit DFS stack, the edge stack), what each stores, and the memory used.

### 4.2 Operations and their complexity
> | Step | What it does | Cost |
> |------|--------------|------|
> | Iterative DFS with `next[u]` | … | O(V + E) |
> | Bridges: `low[u] > tin[p]` | … | |
> | BCCs by popping the edge stack | … | |
> | Articulation points: `pieces = separated[v] + (parent[v] != -1)` | … | |
> | `components_after_removal` | … | |

### 4.3 Invariants maintained
> What holds on the stack and in `low[]` at each step, and why every edge is pushed onto the edge stack exactly once.

### 4.4 Design decisions and rejected alternatives
> | Decision | Alternative rejected | Why |
> |----------|----------------------|-----|
> | Explicit stack | Recursive DFS | Overflows on a 10^5-vertex path |
> | Skip the parent by edge ID | Skip by parent vertex | … (parallel edges) |
> | One formula for root and non-root | Separate root test | … |
> | | | |

## 5. Structure B: bridges by union-find path marking

Structure B computes the bridges and 2-edge-connected components (2-ECCs) of an undirected graph using an iterative Depth-First Search (DFS) spanning forest combined with a custom Disjoint Set Union (DSU) path-marking mechanism. The core theoretical premise is that an edge in an undirected graph is a bridge if and only if it does not lie on any simple cycle. In a DFS spanning forest, non-tree edges are exclusively back-edges between an ancestor and a descendant, each forming a fundamental cycle with the vertical tree path between its endpoints. By marking all tree edges on these fundamental cycles and contracting already-marked paths via union-find, unmarked tree edges are identified as bridges in $O((V + E)\,\alpha(V))$ total time.

The implementation is modularly structured across four translation units in `src/structB/`:
- `DFSForest.hpp` / `DFSForest.cpp`: Iterative DFS spanning forest construction (`buildDFSForest`).
- `DSU.hpp` / `DSU.cpp`: Disjoint Set Union with path compression, union-by-rank, and minimum-depth metadata tracking (`find`, `unite`, `get_highest`).
- `makeCyclePath.hpp` / `makeCyclePath.cpp`: Cycle path marking along vertical tree paths (`markCyclePath`).
- `union_find_bridges.hpp` / `union_find_bridges.cpp`: Bridge detection and 2-ECC grouping coordinator (`UnionFindBridgeSolver::solve`).

### 5.1 Representation

Structure B uses contiguous, cache-friendly flat arrays (`vector<int>` and `vector<bool>`) avoiding any heap reallocations during graph traversal and set operations.

1. **DFS Forest Arrays (`DFSForest` in `src/structB/DFSForest.hpp`)**:
   - `parent` (`vector<int>`, size $V$): Stores the immediate tree parent of each vertex $v$ in the DFS forest. Initialized to $-1$; DFS roots retain $-1$.
   - `depth` (`vector<int>`, size $V$): Stores the unweighted distance (hop count) from the root of $v$'s DFS tree to $v$. Roots have depth $0$. Used to orient non-tree edges vertically and terminate path marking.
   - `parentEdge` (`vector<int>`, size $V$): Stores the unique edge ID $e \in [0, E-1]$ connecting $v$ to $\text{parent}[v]$. Roots store $-1$. This enables $O(1)$ edge identification without scanning adjacency lists.
   - `isTreeEdge` (`vector<bool>`, size $E$): A boolean bitset marking whether edge ID $e$ is a tree edge ($e \in E_T$) or a non-tree edge ($e \in E \setminus E_T$).

2. **Cycle Path Marking Disjoint Set Union (`DSU` in `src/structB/DSU.hpp`)**:
   - `rep` (`vector<int>`, size $V$): The parent pointer for DSU trees, flattened during `find(v)` via two-pass path compression.
   - `rank` (`vector<int>`, size $V$): The rank (upper bound on tree height) of each set representative, incremented only when uniting two sets of equal rank (union-by-rank).
   - `highest` (`vector<int>`, size $V$): Auxiliary metadata array storing the vertex of strictly minimum DFS depth in the equivalence class of root $r$:
     $$\text{highest}[r] = \arg\min_{w \in \text{Set}(r)} \text{depth}[w]$$
     This field allows union-by-rank to choose the root arbitrarily while always preserving immediate access to the shallowest vertex in the contracted component via `get_highest(v) = highest[find(v)]`.

3. **Cycle Edge Marker (`marked` in `src/structB/union_find_bridges.cpp`)**:
   - `marked` (`vector<bool>`, size $E$): Boolean array initialized to `false`. An edge $e$ is marked `true` when it is traversed during cycle path ascent. An edge is a bridge if and only if `isTreeEdge[e] && !marked[e]`.

4. **Second Disjoint Set Union for 2-ECC Grouping (`ecc_dsu` in `src/structB/union_find_bridges.cpp`)**:
   - A distinct instance of `DSU` initialized with size $V$, used after bridge identification to contract all non-bridge, non-self-loop edges and partition $V$ into 2-edge-connected components.

**Memory Footprint.** Across the entire execution on $V = 100,000$ and $E = 1,000,000$:
- DFS forest: $3 \times 4\text{ B} \times V + E / 8\text{ B} \approx 1.2\text{ MB} + 125\text{ KB} \approx 1.33\text{ MB}$.
- Marking DSU: $3 \times 4\text{ B} \times V \approx 1.2\text{ MB}$.
- Markers & stack: $E / 8\text{ B} + 4\text{ B} \times V \approx 125\text{ KB} + 400\text{ KB} \approx 0.53\text{ MB}$.
- 2-ECC DSU: $3 \times 4\text{ B} \times V \approx 1.2\text{ MB}$ (allocated after marking).
Total auxiliary memory is under $4.5\text{ MB}$, well within typical L3 CPU cache capacities.

### 5.2 Operations and their complexity

The solver executes in four distinct sequential steps:

| Step | What it does | Implementation | Cost |
|------|--------------|----------------|------|
| **1. Build DFS Forest** | Explores all components using an explicit heap-allocated vector stack `st` and per-vertex adjacency edge index pointers `edge_idx[v]`. Records `parent`, `depth`, `parentEdge`, and sets `isTreeEdge[id] = true` for tree edges. | `buildDFSForest(g)` in `DFSForest.cpp` | $O(V + E)$ |
| **2. Mark Cycle Paths** | Scans all $E$ edges. For each non-tree edge $e = (u, v)$ ($u \neq v$), calls `markCyclePath`. Ascends the tree path from the deeper endpoint to the shallower endpoint using `dsu.get_highest(v)` to jump over already-contracted cycles, setting `marked[parentEdge[x]] = true` and uniting $x$ with $\text{parent}[x]$. | `markCyclePath(...)` in `makeCyclePath.cpp` | $O((V + E)\,\alpha(V))$ |
| **3. Collect Bridges** | Iterates over all edge IDs $0 \dots E-1$. An edge is collected into `result.bridge_ids` and `result.bridges` iff `isTreeEdge[id] && !marked[id]`. Emitted in increasing ID order. | Loop 4 in `union_find_bridges.cpp` | $O(E)$ |
| **4. Group 2-ECCs** | Initializes `ecc_dsu(V)`. Unites endpoints of all non-bridge, non-loop edges. Groups vertices into `result.two_eccs` in a single pass over $0 \dots V-1$, preserving sorted vertex order and canonical component ordering. | Loop 5 in `union_find_bridges.cpp` | $O((V + E)\,\alpha(V))$ |
| **Total** | Full bridge detection and 2-edge-connected component decomposition. | `UnionFindBridgeSolver::solve(g)` | $\mathbf{O((V + E)\,\alpha(V))}$ |

#### Detailed Complexity Breakdown of Path Marking
1. **At most $V - C - b$ `unite` calls:** Each call to `dsu.unite(x, p, depth)` merges two previously distinct components along an unmarked tree edge. Because the DFS forest contains exactly $V - C$ tree edges (where $C$ is the number of connected components) and each bridge is never united, the inner loop marks and unites an edge at most $V - C - b \le V - 1$ times across the entire algorithm.
2. **At most $2(V + E)$ `find` calls:** In `markCyclePath`, each while-loop step evaluates `dsu.get_highest(v)` (one `find`). If $\text{depth}[x] > \text{depth}[u]$, a tree edge is marked and `dsu.unite(x, p, depth)` is invoked (two `find`s inside `unite`). If $\text{depth}[x] \le \text{depth}[u]$, the loop terminates. The terminating check happens at most once per non-tree edge ($\le E$ times). Thus, the total number of `find` operations across all non-tree edges is bounded by $3(V - C - b) + E = O(V + E)$.
3. **Amortized Bound:** By Tarjan's Theorem (1975), any sequence of $m'$ operations on $n$ elements using union-by-rank and two-pass path compression runs in $O(m' \alpha(n))$ time. Setting $m' = O(V + E)$ and $n = V$ yields $O((V + E)\,\alpha(V))$ total time. Since $\alpha(V) \le 4$ for all practical inputs ($V \le 10^{80}$), this behaves as strictly linear time in practice.

### 5.3 Invariants maintained

The correctness of Structure B rests upon three invariant properties maintained throughout the algorithm:

1. **Cycle Contraction Invariant (2-ECC Subgraphs)**:
   At any point during the execution of `markCyclePath`, every disjoint set in `dsu` corresponds to a connected subgraph of vertices in the DFS forest connected entirely by tree edges that have already been marked on at least one cycle. All vertices within the same DSU set are mutually 2-edge-connected in the subgraph of examined edges.

2. **Minimum Depth Invariant (`highest`)**:
   For every valid root $r = \text{find}(w)$ of a DSU component, $\text{highest}[r]$ strictly points to the vertex with the minimum DFS depth in that component:
   $$\text{depth}[\text{highest}[r]] = \min_{v \in \text{Set}(r)} \text{depth}[v]$$
   - *Base Case:* Initially, each vertex $v$ forms its own singleton set with $\text{highest}[v] = v$, where $\text{depth}[v]$ is trivially minimal.
   - *Inductive Step:* When `unite(u, v, depth)` merges sets with roots $r_u$ and $r_v$, the winner root $r_{\text{new}}$ is chosen by rank. Regardless of which root wins, the algorithm explicitly sets:
     $$\text{highest}[r_{\text{new}}] = \begin{cases} \text{highest}[r_u] & \text{if } \text{depth}[\text{highest}[r_u]] < \text{depth}[\text{highest}[r_v]] \\ \text{highest}[r_v] & \text{otherwise} \end{cases}$$
     Since $\text{parent}[x]$ always has depth $\text{depth}[x] - 1$, depth tracking is strictly monotonically decreasing as the path ascends.

3. **Single Traversal and Jump Invariant**:
   Whenever a tree edge $e = (x, \text{parent}[x])$ is marked, $x$ is immediately united with $\text{parent}[x]$. For any subsequent non-tree edge whose fundamental cycle contains $x$, `dsu.get_highest(x)` jumps directly to $\text{parent}[x]$ or one of its ancestors in $O(\alpha(V))$ time, bypassing $e$. Thus, no tree edge is ever examined, marked, or united more than once.

4. **Bridge and Partition Invariants (INTERFACE.md §3, checked in `tests/test_structB.cpp`)**:
   - *Bridge Equivalence:* $\text{structB.bridge\_ids} == \text{structA.bridge\_ids}$ on all inputs up to $V = 100,000$.
   - *2-ECC Partition:* `two_eccs` forms a complete disjoint partition of $\{0, 1, \dots, V-1\}$ with $\sum |C_i| = V$, each component sorted ascending, ordered by minimum vertex ID.
   - *Bridge Crossing Invariant:* Every bridge connects two distinct 2-ECCs ($comp[u] \neq comp[v]$).
   - *Non-Bridge Invariant:* Every non-bridge, non-self-loop edge connects vertices within the same 2-ECC ($comp[u] == comp[v]$).
   - *Reachability Equivalence:* Connected components in $G \setminus \text{Bridges}$ match `two_eccs` identically.

### 5.4 Design decisions and rejected alternatives

| Decision | Alternative rejected | Why |
|----------|----------------------|-----|
| **`highest[]` metadata with union-by-rank** | Linking directly to the parent's set (`rep[x] = find(parent[x])`) | Directly setting `rep[x] = parent[x]` forces tree edges to point upward, which prevents union-by-rank. Without union-by-rank, path compression alone degrades worst-case find operations to $O(\log V)$ amortized (or $O(V)$ without full compression), failing the project's $O((V+E)\alpha(V))$ complexity requirement (bug log entry 4). Decoupling the DSU root choice (governed by `rank`) from the shallowest vertex (tracked by `highest[]`) achieves true $O(\alpha(V))$ amortized complexity while allowing instantaneous jumps to the path top. |
| **Iterative DFS with explicit stack and adjacency pointers** | Recursive DFS (`void dfs(u)`) | On long path graphs ($V = 100,000$), recursion stack depth reaches $10^5$ frames ($\approx 3.2\text{ MB}$ to $8\text{ MB}$ of stack memory), exceeding typical thread stack limits ($2\text{ MB}$ on Windows/Linux) and triggering `SIGSEGV` segmentation faults. The iterative implementation maintains an explicit heap `vector<int> st` and a `vector<int> edge_idx` cursor array, safely scaling to $V = 100,000$ and $E = 1,000,000$. |
| **Preserving `bridges` in sequential `bridge_ids` order** | Sorting `bridges` by endpoint pairs using `Edge::operator<` | `Edge::operator<` sorts by $(u, v)$ lexicographically. If bridge edge IDs are not monotonic with endpoint order (e.g. edge 0 = (2,3), edge 1 = (0,1)), sorting `bridges` caused `bridges[i] != g.edges[bridge_ids[i]]`, failing invariant checks and disagreeing with Structure A in 1,218 out of 5,000 random graphs (bug log entry 7). Emitting bridges directly in loop order $0 \dots E-1$ guarantees that `bridges` and `bridge_ids` remain 1-to-1 synchronized. |
| **Early self-loop filtering in DFS and path marking** | Allowing self-loops into adjacency lists or path marking | Self-loops ($u = v$) do not affect connectivity and can never be bridges. `Graph::add_edge` excludes them from `adj[][]`, and `markCyclePath` explicitly skips $u = v$, avoiding wasted DSU operations and ensuring $E' = E - L$ bounds are strictly observed. |
| **Independent second DSU pass for 2-ECC grouping** | Contracting bridges on the path-marking DSU | Attempting to reuse the cycle-marking DSU to produce 2-ECCs requires rolling back or re-routing components. A dedicated second pass over non-bridge edges with `ecc_dsu` runs in just $O((V+E)\alpha(V))$ and provides an isolated, deterministic, and canonically ordered partition. |

## 6. The requirement beyond the lecture

### 6.1 Bridges by union-find path marking

#### Theoretical Foundation: Tarjan (1974) and Tarjan (1975)
In his 1974 paper, *"A note on finding the bridges of a graph"* [3], Robert E. Tarjan demonstrated that bridge detection can be formulated as fundamental cycle detection on a spanning tree combined with disjoint-set path compression. When a depth-first search explores an undirected graph $G = (V, E)$, it partitions edges into a spanning forest $F = (V, E_T)$ and a set of back-edges $E \setminus E_T$, each connecting an ancestor $u$ and descendant $v$ in $F$.

Every back-edge $(u, v)$ induces a unique *fundamental cycle* consisting of the back-edge itself and the vertical tree path $P_T(u, v)$ in $F$. A fundamental graph-theoretic property is that an edge $e \in E$ is a bridge if and only if $e$ does not lie on any simple cycle. Because every cycle in $G$ can be expressed as a symmetric difference of fundamental cycles, an edge is a bridge if and only if it is a tree edge that does not belong to any fundamental cycle.

Tarjan observed that rather than computing lowpoint values recursively as in his 1972 linear-time algorithm [1], one can iteratively traverse and mark the tree path $P_T(u, v)$ for each back-edge $(u, v)$, while using a disjoint-set union (union-find) data structure to contract already-traversed paths. Once a tree edge is marked, its endpoints are united into an equivalence class. When subsequent back-edges traverse the tree, the union-find structure allows the algorithm to skip directly over already-contracted segments in amortized near-constant time.

In his companion 1975 paper, *"Efficiency of a good but not linear set union algorithm"* [4], Tarjan established the tight theoretical complexity of the disjoint-set union data structure. He proved that an intermixed sequence of $m'$ `find` and `unite` operations on $n$ elements using both path compression and union-by-rank (or union-by-size) requires at most $O(m' \alpha(m', n))$ time, where $\alpha$ is the functional inverse of Ackermann's function. Since the path-marking algorithm performs at most $V - 1$ unions (each unmarked tree edge is united at most once) and at most $O(V + E)$ finds (each non-tree back-edge incurs at most one non-uniting find before terminating), the total execution time of path marking is strictly bounded by $O((V + E) \alpha(V))$.

#### Implementation: `markCyclePath`
The cycle path marking mechanism is implemented in `markCyclePath` (`src/structB/makeCyclePath.cpp`). Given a non-tree edge $(u, v)$ with $\text{depth}[u] < \text{depth}[v]$:
1. The search begins at the deeper endpoint $v$. The algorithm queries `curr = dsu.get_highest(v)`, which performs `find(v)` with two-pass path compression to retrieve the root representative, followed by indexing `highest[root]` to obtain the vertex of strictly minimum DFS depth in $v$'s equivalence class.
2. While $\text{depth}[curr] > \text{depth}[u]$:
   - The immediate tree parent of $curr$ is retrieved via $p = \text{forest.parent}[curr]$.
   - The tree edge connecting $curr$ and $p$ is marked as belonging to a cycle: `marked[forest.parentEdge[curr]] = true`.
   - The sets containing $curr$ and $p$ are united via `dsu.unite(curr, p, forest.depth)`.
   - The algorithm advances up the tree: `curr = dsu.get_highest(curr)`.
3. As soon as $\text{depth}[curr] \le \text{depth}[u]$, the path has ascended to or above the shallow ancestor $u$. All tree edges on the fundamental cycle are now marked, and the loop terminates.

Because each unmarked tree edge is traversed and contracted at most once, and each edge is examined at most once per non-tree edge, the work amortizes to $O((V + E) \alpha(V))$. Tree edges that remain unmarked after all non-tree edges are processed are collected as bridges in a single $O(E)$ pass.

#### Differences from Tarjan's Original Paper
Structure B departs from Tarjan's original 1974 formulation in three key design aspects:

1. **Decoupling Rank from Depth (`highest[]` Metadata vs. Naive Parent Pointers)**:
   In Tarjan (1974), the representative of a contracted set was proposed to be the root-ward (shallowest) vertex by simply pointing the representative of the descendant set to the ancestor representative (`rep[v] = rep[p]`). While conceptually straightforward, forcing tree edges to always point upward strictly forbids union-by-rank. Without rank balancing, path compression alone yields an amortized complexity of $O(\log V)$ per operation on pathological inputs (or $O(V)$ in non-compressed variants), violating the required $O((V + E)\alpha(V))$ bound (recorded in Bug Log entry 4).
   Structure B resolves this tension by strictly decoupling the DSU tree balance from the DFS tree geometry:
   - Set roots and parent pointers in `rep[]` are merged purely according to `rank[]` (union-by-rank), preserving Tarjan's (1975) inverse Ackermann bound.
   - An independent metadata array `highest[r]` tracks the vertex with minimal `depth` across all members of root $r$'s equivalence class. During `unite(u, v, depth)`, `highest` for the winning root is assigned the endpoint with minimum DFS depth.
   This guarantees both balanced DSU trees ($O(\alpha(V))$ amortized operations) and instantaneous jumping to the highest ancestor in the contracted component via `get_highest(v) = highest[find(v)]`.

2. **Iterative Heap Stack vs. Recursive Traversal**:
   Tarjan (1974) assumed a standard recursive DFS traversal. On deep graphs such as long path graphs ($V = 100,000$), call-frame overhead causes stack memory exhaustion and segmentation faults on systems with standard $2\text{ MB}$ stack limits. Structure B implements an explicit heap-allocated vector stack `vector<int> st` and per-vertex adjacency cursor pointers `edge_idx` (`DFSForest.cpp`), supporting arbitrary graph topologies up to $V = 100,000$ safely.

3. **Multi-Phase Architecture vs. Interleaved Traversal**:
   Tarjan's paper describes path marking online during DFS backtracking. Structure B decouples the computation into distinct sequential phases:
   - *Phase 1:* Spanning forest construction (`buildDFSForest`).
   - *Phase 2:* Batch path marking over non-tree edges (`markCyclePath`).
   - *Phase 3:* Bridge collection preserving original edge ID ordering ($0 \dots E-1$).
   - *Phase 4:* Partitioning into 2-edge-connected components via an independent DSU instance (`ecc_dsu`).
   This separation guarantees that edge ID ordering is strictly preserved (avoiding discrepancy with Structure A's bridge order, bug log entry 7), keeps each algorithmic phase testable in isolation, and allows direct hardware cache profiling without entangled state.

### 6.2 Block-cut tree and the separation query

#### Theoretical Foundation: Harary (1969), Chapter 3

Following Harary (*Graph Theory*, Chapter 3) [6], a *block* of an undirected graph $G$ is a maximal connected subgraph that has no cut-point (articulation point) of its own. Every non-loop edge of $G$ lies in a unique block: two edges belong to the same block if and only if they lie on a common simple cycle, and every bridge forms a two-vertex block of its own. The *block-cutpoint graph* (or *block-cut tree*) of $G$ is the bipartite graph $T = (\mathcal{C} \cup \mathcal{B}, E_T)$ whose vertex set consists of the set of cut-points $\mathcal{C}$ (C-nodes) and the set of blocks $\mathcal{B}$ (B-nodes), where a C-node $c \in \mathcal{C}$ is adjacent to a B-node $B_i \in \mathcal{B}$ if and only if $c \in V(B_i)$ in $G$.

Harary proves two structural properties that form the basis of our query engine (derived formally in D3):
1. **Acyclicity (Theorem 3.4):** Within every connected component of $G$, the block-cut graph is a tree. If a cycle $B_1 - c_1 - B_2 - c_2 - \dots - B_k - c_k - B_1$ existed in $T$, the union $\bigcup_{i=1}^k B_i$ would be 2-vertex-connected (removing any single vertex leaves the remaining blocks connected along the opposite arc of the cycle), contradicting the maximality of the individual blocks $B_i$.
2. **Path Separation Characterisation:** Every vertex $u \in V(G)$ with at least one incident non-loop edge maps to a representative node $\text{rep}(u) \in V(T)$: if $u \in \mathcal{C}$, $\text{rep}(u)$ is its C-node $c_u$; otherwise $u$ belongs to a unique block $B(u)$, so $\text{rep}(u) = B(u)$. Removing vertex $v \notin \{x, y\}$ disconnects $x$ from $y$ in $G$ if and only if $v \in \mathcal{C}$ and its C-node $c_v$ lies strictly on the unique simple path between $\text{rep}(x)$ and $\text{rep}(y)$ in $T$—including the boundary cases where $x$ or $y$ is itself an articulation point (so $\text{rep}(x) = c_x$ or $\text{rep}(y) = c_y$).

#### Construction from Structure A's Biconnected Components

In `BlockCutTree::build` (`app/block_cut_tree.hpp`), the Block-Cut Forest is constructed in $O(V + E)$ time directly from `StructAResult`:
1. The $n_C = |\text{articulation\_points}|$ cut vertices are assigned C-node indices $0, 1, \dots, n_C - 1$, stored in `cut_id[v]` and `rep[v]`.
2. The $n_B = |\text{bcc\_vertices}|$ blocks are assigned B-node indices $n_C, n_C + 1, \dots, n_C + n_B - 1$.
3. Iterating over each block $b \in [0, n_B - 1]$ and each vertex $v \in \text{bcc\_vertices}[b]$:
   - If `cut_id[v] != -1`, an undirected tree edge is added between C-node `cut_id[v]` and B-node $n_C + b$.
   - Otherwise, $v$ is a non-cut vertex belonging solely to block $b$, and we set `rep[v] = n_C + b`.
   Since $\sum_b |\text{bcc\_vertices}[b]| \le 2E$, this construction takes $O(V + E)$ time and space.

#### Answering `separates(v, x, y)` via DFS Entry and Exit Timestamps

To answer separation queries without Lowest Common Ancestor (LCA) sparse tables, we run an iterative DFS over each tree of the Block-Cut Forest, recording each node's component ID `comp[u]`, entry timestamp `tin[u]`, exit timestamp `tout[u]`, and ordered list of DFS children `tree_children[u]`. In any rooted DFS tree, node $u$ is an ancestor of node $w$ (`is_sub(u, w)`) if and only if $\text{tin}[u] \le \text{tin}[w]$ and $\text{tout}[w] \le \text{tout}[u]$.

Given a query `separates(v, x, y)`:
1. **Guard checks:** If $v \in \{x, y\}$, $x = y$, `cut_id[v] == -1`, or $\text{rep}(x)$ and $\text{rep}(y)$ lie in different connected components, return `false` in $O(1)$ time.
2. **Subtree classification:** Let $c = \text{cut\_id}[v]$, $X = \text{rep}[x]$, and $Y = \text{rep}[y]$. Since $v \notin \{x, y\}$ and C-nodes correspond bijectively to distinct original vertices, $X \neq c$ and $Y \neq c$. We evaluate `x_under = is_sub(c, X)` and `y_under = is_sub(c, Y)` in $O(1)$ time:
   - **Case 1 (`x_under != y_under`):** Exactly one of $X, Y$ lies inside the DFS subtree of $c$, while the other lies outside. Every path in the tree from inside $c$'s subtree to outside must pass through $c$, so $c$ separates $X$ and $Y$; return `true` in $O(1)$ time.
   - **Case 2 (`!x_under && !y_under`):** Both $X$ and $Y$ lie outside $c$'s subtree. Because a subtree is convex in a tree, the unique path between $X$ and $Y$ never enters $c$'s subtree; return `false` in $O(1)$ time.
   - **Case 3 (`x_under && y_under`):** Both $X$ and $Y$ are proper descendants of $c$. In a rooted tree, the path between two proper descendants of $c$ passes through $c$ if and only if they reside in the subtrees of two *different* direct children of $c$. Because the DFS visits `tree_children[c]` sequentially, their timestamp intervals $[\text{tin}[w_i], \text{tout}[w_i]]$ are disjoint and sorted in strictly increasing order. The helper `branch_of(c, X)` binary-searches `tree_children[c]` by `tin`/`tout` in $O(\log(\text{deg}_T(c)))$ time to locate the unique child $w_X$ containing $X$ (and $w_Y$ containing $Y$), returning `w_X != w_Y`.

#### Differences from Harary's Formulation

Harary defines blocks to include isolated vertices and single bridges, and assumes a connected graph without self-loops. Our implementation adapts this to the project's multigraph specification in three ways:
1. **Block-Cut Forest for Disconnected Multigraphs:** Rather than assuming $G$ is connected, `BlockCutTree` maintains a forest across all connected components, tracking `comp[u]` so queries on disconnected vertex pairs return `false` in $O(1)$ time.
2. **Isolated Vertices (`rep[v] = -1`):** Structure A defines BCCs strictly as edge-biconnected blocks (`bcc_vertices` is empty for isolated vertices or vertices incident only to self-loops). We assign `rep[v] = -1` to such vertices and recover $C_0$ directly from `components_after_removal[0]` and vertex $0$'s status.
3. **Iterative Euler-Tour Timestamping:** Just as Structure A and Structure B avoid call-stack recursion, the Block-Cut Tree DFS is implemented iteratively with an explicit heap stack `st` and cursor array `cur_edge`, preventing stack overflow when a path graph of $10^5$ vertices produces a Block-Cut Tree of $199,999$ alternating C-nodes and B-nodes (depth $199,998$).

## 7. Correctness

### 7.1 The oracle and why it is correct

> **TO FILL: Shubham** · about ½ page
> `oracle/bfs_oracle.hpp`, `BfsOracle::solve`: remove each vertex and each non-loop edge in turn and count components by BFS. Why this is correct by definition (D4), why edges are removed by ID (parallel edges), why the test is "more than C0" components and not "more than 1", and its cost: O((V + E)^2), since it also removes every edge.

### 7.2 Differential test

> **TO FILL: Shubham** · about ½ page
> `oracle/differential_test.cpp`: what one operation is, the graph sizes, the seed (20260928), the result at 10^5 and 10^6 operations, and how a failure is shrunk to a minimal edge sequence (`oracle/failure_shrinker.cpp`).

### 7.3 Invariant checkers

> **TO FILL: Shubham** · about ½ page
> Each checker and what it tests: `tests/test_structA.cpp` (`verifyBccInvariants`), `tests/test_structB.cpp` (`verifyStructBInvariants`), `tests/oracle_tests.cpp`. Say how they are run (`make test`) and whether the differential test calls them.

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

## 8. Adversarial inputs

> **TO FILL: Raunak (construction and argument), Deepak (measured effect)** · 1–2 pages
> One sub-section per target. Raunak: the command, how the generator builds the graph, and the worst-case argument (D5). Deepak: the measured effect, from `results/raw/adversarial.csv` and `results/figures/fig5_adversarial.png`.

### 8.1 Graphs where only the root rule decides the answer (`gen_root`, six modes)

### 8.2 A long simple cycle (`gen_cycle`)

### 8.3 Cliques joined at one vertex (`gen_cliques`)

### 8.4 Paths deep enough to overflow a recursive DFS (`gen_path`, `gen_root path`)
> Say which input, compiler flags and stack size actually overflow a recursive DFS (bug log, entry 8).

### 8.5 Parallel edges and self-loops (`gen_multi`, beyond the named targets)

## 9. Predicted cost model

The model below was written before any measurement. The first draft was committed at c1 in [`d6d6456`](https://github.com/cse250001026/cs253-oops-p04/commit/d6d6456) (10 Oct 2026); the final model was committed in [`5ff34df`](https://github.com/cse250001026/cs253-oops-p04/commit/5ff34df) (14 Nov 2026), before any file in `results/raw/`.

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

## 11. Results

> **TO FILL: Deepak** · 3–5 pages
> Each figure with a caption saying what to notice, the script and raw file that made it, and the predicted curve on the same axes. Explain every gap between prediction and measurement.

### 11.1 Scaling of A and B (`fig1_time_vs_V.png`, `fig2_ns_per_edge.png`)

### 11.2 The oracle (`fig3_oracle.png`)

### 11.3 B against A (`fig4_B_over_A.png`)

### 11.4 Full scale: V = 10^5, E = 10^6 (`results/raw/fullscale.csv`)

### 11.5 Adversarial inputs (`fig5_adversarial.png`)

### 11.6 Operation counts against prediction (`table_counters.md`)

### 11.7 Cache behaviour (`fig6_cache_misses.png`, perf output in `results/raw/cache_profile.csv`)

## 12. Limitations

> **TO FILL: Deepak, with one line from each member** · ½ page
> What does not work, what you would do differently, and what you would measure next. Each member adds one honest line about their module.
- **Block-Cut Tree (`app/`):** When both query endpoints $x$ and $y$ fall inside the DFS subtree of cut vertex $v$, `separates(v, x, y)` binary-searches $v$'s children in $O(\log(\text{deg}_T(v)))$ time; storing a flat Euler tour with a sparse table would make even this case $O(1)$ at the cost of $O(V \log V)$ extra memory.

## 13. References

> **TO FILL: every member** · as needed
> Add every paper, book, web page or tool you used, in the same style. The project's reading list is already here.

1. R. E. Tarjan, "Depth-first search and linear graph algorithms," *SIAM Journal on Computing* 1(2):146–160, 1972.
2. J. Hopcroft and R. E. Tarjan, "Algorithm 447: Efficient algorithms for graph manipulation," *Communications of the ACM* 16(6):372–378, 1973.
3. R. E. Tarjan, "A note on finding the bridges of a graph," *Information Processing Letters* 2(6):160–161, 1974.
4. R. E. Tarjan, "Efficiency of a good but not linear set union algorithm," *Journal of the ACM* 22(2):215–225, 1975.
5. J. M. Schmidt, "A simple test on 2-vertex- and 2-edge-connectivity," *Information Processing Letters* 113(7):241–244, 2013.
6. F. Harary, *Graph Theory*, Addison-Wesley, 1969, Chapter 3.
7. T. H. Cormen, C. E. Leiserson, R. L. Rivest and C. Stein, *Introduction to Algorithms*, 3rd ed., MIT Press, 2009, Problem 22-2.
8. Tools: g++, Linux `perf`, Python and matplotlib; AI tools as declared in [TOOLS.md](TOOLS.md).
