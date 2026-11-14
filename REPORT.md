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
- …

**Sufiyan Ahmad (Architect):** PRs [#2](https://github.com/cse250001026/cs253-oops-p04/pull/2), [#5](https://github.com/cse250001026/cs253-oops-p04/pull/5).
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

> **TO FILL: Sufiyan** · 1 page
> Taken from INTERFACE.md, not copied wholesale. Summarise:
> - the shared `Graph` (vertices 0…V−1, edges with IDs, adjacency of (vertex, edge ID) pairs, self-loops stored but kept out of the adjacency lists) and the rules for parallel edges, self-loops, isolated vertices and disconnected graphs;
> - the four APIs: `TarjanSolver::solve`, `UnionFindBridgeSolver::solve`, `BfsOracle::solve` and the block-cut tree, with what each result holds and in what order;
> - every structural invariant the checkers test (INTERFACE.md §3), each with the file and function that checks it;
> - the input formats: the generator graph format and the `run_ops` format.

### 3.1 Common interface

### 3.2 Structural invariants

| Invariant | Holds for | Checked by (file, function) |
|-----------|-----------|-----------------------------|
| | | |

## 4. Structure A: iterative Tarjan low-link DFS

Structure A is implemented in `src/structA/tarjan_lowlink.hpp`. The public entry point is `TarjanSolver::solve(const Graph&)`, a thin wrapper over the internal `runStructA`. One iterative depth-first search over the whole graph yields the articulation points, the bridges, the biconnected components (BCCs) and the number of connected components left after deleting each vertex.

The method is Tarjan's low-link scheme. For a vertex `u`, `tin[u]` is its discovery time and `low[u]` is the smallest `tin` reachable from the DFS subtree of `u` by tree edges followed by at most one back edge. Comparing `low[v]` with `tin[u]` when a child `v` finishes tells us whether the subtree of `v` can reach the rest of the graph without passing through `u` (or through the edge `(u,v)`).

### 4.1 Representation

The input is the shared `Graph`: vertices `0…V-1`, edges with unique IDs, and adjacency lists of `(neighbour, edge ID)` pairs. Self-loops are stored in the edge list but kept out of the adjacency lists, so the DFS never sees them. The edge ID is what lets the code tell parallel edges apart.

| Structure | Size | Meaning |
|---|---|---|
| `tin[u]` | V | Discovery time; unvisited until assigned, never changed afterwards. |
| `low[u]` | V | Smallest `tin` reachable from the explored part of `u`'s subtree by tree edges plus at most one back edge. |
| `parent[u]` | V | DFS parent of `u`, or `-1` for a DFS root. |
| `parentEdge[u]` | V | ID of the tree edge that discovered `u`. |
| `next[u]` | V | Index of the next adjacency entry of `u` still to be examined. |
| `separated[u]` | V | Number of DFS children `v` of `u` with `low[v] >= tin[u]`. |
| DFS stack | ≤ V | Discovered, unfinished vertices. Always a root-to-current path of the DFS tree. |
| Edge stack | ≤ E | Edges seen whose BCC is not yet closed. |

`next[u]` acts as the program counter of the simulated recursive call: when a child finishes, `u` resumes at exactly the adjacency entry where it left off. When `next[u]` reaches the end of `u`'s list, `u` is popped and `low[u]` is propagated to its parent.

A tree edge is pushed on the edge stack when its child is discovered. A back edge is pushed only once, from its deeper endpoint to an already discovered ancestor. When a child `v` of `u` finishes with `low[v] >= tin[u]`, the edge stack is popped down to and including `(u,v)`; those edges are one BCC, stored as edge IDs in `bcc_edge_ids`, with `bcc_vertices` holding the distinct endpoints. Bridges are first recorded by edge ID in `bridge_ids`, and the returned `bridges` array follows that order: `bridges[i] == g.edges[bridge_ids[i]]`.

**Memory.** The six per-vertex arrays and the DFS stack take O(V). The edge stack takes O(E) in the worst case, and the returned BCC lists hold O(V+E) entries in total (each edge appears in exactly one BCC, each vertex in at most one BCC per incident component of the block structure). The total auxiliary and output storage is therefore **O(V+E)**.

### 4.2 Operations and their complexity
| Step | What it does | Cost |
|---|---|---|
| Iterative DFS with `next[u]` | Discovers every vertex once and scans every adjacency entry once, with no recursion. Restarted from each undiscovered vertex, so it builds a DFS forest. | O(V+E) |
| Low-link update | `low[u] = min(low[u], tin[w])` for a back edge to `w`; `low[u] = min(low[u], low[v])` when child `v` finishes. | O(1) per edge |
| Bridges: `low[v] > tin[u]` | Tree edge `(u,v)` is a bridge: the subtree of `v` has no route to `u` or above. | O(1) per tree edge |
| BCCs by popping the edge stack | When `low[v] >= tin[u]`, pop through `(u,v)`; the popped edges form one BCC. | O(E) in total |
| Articulation points: `pieces = separated[v] + (parent[v] != -1)` | `separated[u]` is incremented for each child with `low[v] >= tin[u]`. A vertex is an articulation point iff `pieces >= 2`. | O(1) per tree edge, O(V) to read off |
| `components_after_removal` | `(C - 1) + pieces`, where `C` is the number of connected components of the original graph. | O(V) |
| Build the result lists | Articulation points, bridge objects, BCC vertex lists. | O(V+E), equal to the output size |

**Why the popping loop is not quadratic.** A single finished child may pop many edges, but each edge is pushed once and popped once over the whole run, so all pops together cost O(E).

**Why the formula works.** Delete a non-root vertex `v` inside a component. Each child subtree with `low[child] >= tin[v]` becomes its own piece, and everything on the parent side forms one more piece: `separated[v] + 1` pieces. For a DFS root there is no parent side, and each child subtree is a piece: `separated[v]` pieces. The term `(parent[v] != -1)` covers both cases in one expression. The other `C - 1` components are untouched. An isolated vertex has `pieces = 0`, so deleting it leaves `C - 1` components, which is correct. A vertex is an articulation point exactly when its component splits into more than one piece, which gives the usual rules: a root needs at least two DFS children, and a non-root needs at least one child with `low[child] >= tin[v]`.

**Total.** Each vertex is discovered once, each adjacency entry is scanned once, and each edge is pushed and popped once, so Structure A runs in **O(V+E)** time.


### 4.3 Invariants maintained

Each invariant below is stated for a specific moment of the run.

1. **DFS stack is the current tree path.** At every step the DFS stack, read from bottom to top, is the path from a DFS root to the vertex being processed. Every vertex `u` on it has `tin[u]` set and has examined exactly the adjacency entries before `next[u]`. This is what makes resuming after a child equal to recursive behaviour.

2. **Low-link invariant.** While `u` is on the stack, `low[u]` equals the minimum of `tin[u]`, `tin[w]` over back edges `(x,w)` from the already explored part of `u`'s subtree, and `low[v]` over finished children `v`. When `u` is popped, all edges of its subtree have been examined, so `low[u]` is final.

3. **Parent-edge invariant.** While scanning `u`, only the entry whose edge ID equals `parentEdge[u]` is ignored. Any other edge to `parent[u]` is a real back edge and is processed normally.

4. **Edge-stack invariant.** Each edge is pushed exactly once: a tree edge when its child is discovered, a back edge only from the deeper endpoint to an ancestor (so the two adjacency copies of an edge do not cause two pushes). When child `v` of `u` finishes with `low[v] >= tin[u]`, the part of the stack above and including `(u,v)` consists exactly of the edges of the subtree of `v` not already placed in a closed BCC. Popping it therefore removes one complete BCC, and every edge ends in exactly one BCC. Articulation vertices may appear in several BCC vertex lists, since BCCs partition edges, not vertices.

5. **Separation-count invariant.** After `u` finishes, `separated[u]` is exactly the number of its children `v` with `low[v] >= tin[u]`. The same number drives the articulation test and `components_after_removal`.

6. **Forest coverage.** The outer loop starts a new DFS from every undiscovered vertex, so every vertex gets a `tin`, and `C` equals the number of DFS roots.

**Edge cases.** Self-loops never enter the adjacency lists, so they do not change `tin`, `low`, any BCC or any count. An isolated vertex is its own DFS root with `pieces = 0` and belongs to no BCC (it has no edges). A bridge is a BCC consisting of a single edge. Parallel edges are handled by invariant 3: two parallel edges between `u` and its parent give `low[u] <= tin[parent]`, so the connecting edge is correctly not reported as a bridge.

### 4.4 Design decisions and rejected alternatives

| Decision | Alternative rejected | Why |
|---|---|---|
| Explicit DFS stack with `next[u]` | Recursive DFS | A path graph has DFS depth V, and V can be 10^5. Recursion would depend on the call stack. |
| Explicit stack | Recursion with an enlarged stack size (`ulimit -s`, thread stack attribute) | It works only on machines where the limit can be raised, and the safe size depends on the platform and compiler. An explicit stack has no such dependence and its memory use is known (O(V)). |
| Skip the parent by edge ID (`parentEdge[u]`) | Skip every edge whose other endpoint is `parent[u]` | With parallel edges, skipping by vertex ignores the second connection to the parent and reports a false bridge. |
| Edge stack for BCCs | Vertex stack (the common variant that stores vertices) | A vertex stack gives BCC vertex sets directly, but our output is edge-based (`bcc_edge_ids`), and with parallel edges and bridges a vertex-only form loses which edges belong to which BCC. The edge stack gives edge IDs and lets the vertex lists be derived. |
| Edge stack inside the same DFS | Chain decomposition, or a second traversal to rebuild BCCs | Both are also linear, but the low-link test already says when a BCC closes, so popping at that moment avoids a second pass and extra structures. |
| Push back edges only from the descendant | Push every non-tree adjacency entry | Each undirected edge is listed at both endpoints; pushing from both would duplicate edges and break the edge partition. |
| One formula `separated[v] + (parent[v] != -1)` | Separate root and non-root post-processing | The only difference between the cases is the parent-side piece, so one expression is enough and removes a branch where bugs could hide. |
| Restart DFS from every undiscovered vertex | Assume a connected graph | Inputs may be disconnected or contain isolated vertices; correct counts need a DFS forest and the value `C`. |
| Identify bridges by edge ID | Identify bridges by endpoint pair | Parallel edges can have identical endpoints but are distinct edges. |
| One Tarjan pass for everything | Delete each vertex or edge and re-search | Repeated searches require O((V+E)^2) time when checking both vertex and edge removals. This approach is used only by the oracle (`BfsOracle`), not by Structure A. |


## 5. Structure B: bridges by union-find path marking

> **TO FILL: Mayank** · 2–3 pages
> Code: `src/structB/DFSForest.cpp` (`buildDFSForest`), `DSU.cpp` (`find`, `unite`, `get_highest`), `makeCyclePath.cpp` (`markCyclePath`), `union_find_bridges.cpp` (`UnionFindBridgeSolver::solve`).

### 5.1 Representation
> The DFS forest arrays (`parent`, `depth`, `parentEdge`, `isTreeEdge`), the DSU (`rep`, `rank`, `highest`), `marked`, and the second DSU used for 2-edge-connected components.

### 5.2 Operations and their complexity
> | Step | What it does | Cost |
> |------|--------------|------|
> | Build the DFS forest | … | O(V + E) |
> | Mark each non-tree edge's cycle path | … | O((V + E) α(V)) |
> | Collect bridges, group 2-ECCs | … | |

### 5.3 Invariants maintained
> Each DSU set is a connected piece of the DFS tree whose internal edges are all marked; `highest[root]` is its shallowest vertex.

### 5.4 Design decisions and rejected alternatives
> | Decision | Alternative rejected | Why |
> |----------|----------------------|-----|
> | `highest[]` with union by rank | Linking straight to the parent's set | Only O(log V) per find (bug log, entry 4) |
> | `bridges` in `bridge_ids` order | Sorting by endpoints | Broke `bridges[i] == edges[bridge_ids[i]]` (bug log, entry 7) |
> | | | |

## 6. The requirement beyond the lecture

### 6.1 Bridges by union-find path marking

> **TO FILL: Mayank** · about ½–1 page
> What the reading says (Tarjan 1974, "A note on finding the bridges of a graph"; Tarjan 1975 for the union-find bound), how `markCyclePath` implements it, and where your implementation differs from the paper.

### 6.2 Block-cut tree and the separation query

> **TO FILL: Sufiyan** · about ½–1 page
> What the reading says (Harary, Graph Theory, ch. 3), how the block-cut tree is built from Structure A's BCCs, how entry and exit times answer "does v separate x from y" (including when x or y is itself an articulation point), and where your implementation differs.

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
