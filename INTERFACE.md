# INTERFACE.md: Common Interface & Structural Invariants

**Project 4: Critical Nodes and Links in a Network**  
**Team:** Oops (`cs253-oops-p04`)  
**Role:** Architect (D3) — Sufiyan Ahmad (Roll No: 250001068)  
**Course Lab:** CS 253 / MA 253 Data Structures and Algorithms Laboratory (Autumn 2026)

```plaintext
Please respond if anything else needs to be modified.
```

## 1. Architectural Overview & Shared Abstraction

To ensure modularity and enforce that **Structure A** (`src/structA/`, D1 Chetna — Iterative Tarjan Low-Link DFS), **Structure B** (`src/structB/`, D2 Mayank — Union-Find Path Marking), **Oracle** (`oracle/`, D4 Shubham — Brute-Force BFS Verification), and **Application** (`app/`, D3 Sufiyan — Block-Cut Tree & Robustness Engine) interact seamlessly, all modules share the class definitions in `include/graph.hpp`.

### 1.1 Core Definitions & Edge Case Rules

- **Graph Representation (`class Graph`)**: An undirected graph represented via an adjacency array `vector<vector<AdjEdge>> adj` and edge list `vector<Edge> edges`.
  - **Scale**: $V \le 100,000$, $E \le 1,000,000$.
  - **Vertex labels**: $0, 1, \dots, V-1$.
  - **Edge IDs**: $0, 1, \dots, E-1$, assigned sequentially by `Graph::add_edge(u, v)`.
- **Disconnected Graphs & Isolated Vertices**:
  - Supported across all modules. Let $C_0$ be the number of connected components in $G$.
  - Removing vertex $v$ leaves $C(G \setminus \{v\})$ components among the remaining $V-1$ vertices. Vertex $v$ is an articulation point iff $C(G \setminus \{v\}) > C_0$.
  - Removing an isolated vertex reduces total components to $C_0 - 1$, so isolated vertices are never articulation points.
- **Parallel Edges & Self-Loops**:
  - Parallel edges are distinguished via unique `AdjEdge::id`. During DFS, tree-parent edges are skipped by checking `e.id == parent_edge_id` (not `e.to == parent_vertex`).
  - Self-loops ($u = v$) are recorded in `edges[]` to preserve contiguous edge IDs, but excluded from `adj[][]` as they do not affect connectivity, cut vertices, or bridges.
- **No Prohibited STL Containers**:
  - Per course rules, `map`, `set`, `unordered_map`, and `priority_queue` are never used inside Structure A, Structure B, or Application.

---

## 2. API Specifications

### 2.1 Structure A Interface (`src/structA/tarjan_lowlink.hpp`)

```cpp
#ifndef TARJAN_LOWLINK_HPP
#define TARJAN_LOWLINK_HPP

#include "../../include/graph.hpp"

class TarjanSolver {
public:
    // Computes articulation points, bridges, and biconnected components
    // using an explicit heap-allocated stack (iterative DFS) in O(V + E) time.
    static StructAResult solve(const Graph& g);
};

#endif
```

### 2.2 Structure B Interface (`src/structB/union_find_bridges.hpp`)

```cpp
#ifndef UNION_FIND_BRIDGES_HPP
#define UNION_FIND_BRIDGES_HPP

#include "../../include/graph.hpp"

class UnionFindBridgeSolver {
public:
    // Builds an iterative DFS tree and marks tree edges on cycles using
    // custom Union-Find path compression in O((V + E) alpha(V)) time.
    static StructBResult solve(const Graph& g);
};

#endif
```

### 2.3 Application Interface (`app/block_cut_tree.hpp`-my part)

```cpp
#ifndef BLOCK_CUT_TREE_HPP
#define BLOCK_CUT_TREE_HPP

#include "../include/graph.hpp"

class BlockCutTree {
public:
    // Constructs the Block-Cut Forest and computes DFS entry/exit times in O(V + E).
    void build(const Graph& g, const StructAResult& a_res);

    // Returns true iff the failure of vertex v separates x from y,
    // answered using Block-Cut Tree entry/exit timestamps (tin/tout).
    bool separates(int v, int x, int y) const;

    // Returns all vertices ranked by the number of components their removal creates.
    vector<NodeRobustness> get_robustness_ranking() const;
};

#endif
```

### 2.4 Oracle Interface (`oracle/bfs_oracle.hpp`)

```cpp
#ifndef BFS_ORACLE_HPP
#define BFS_ORACLE_HPP

#include "../include/graph.hpp"

class BfsOracle {
public:
    // Brute-force removes each vertex and edge in turn and counts components via BFS
    // in O((V + E)(V + E)) time for graphs up to V <= 5000.
    static OracleResult solve(const Graph& g);
};

#endif
```

# Section 3: Structural Invariants

Every invariant below is checked automatically in `tests/` and during differential testing:

## Graph Symmetry & ID Invariant
* Every edge `id` in $[0, E-1]$ with $u \neq v$ satisfies $0 \le \text{edges}[id].u < \text{edges}[id].v < V$, and appears once in `adj[u]` and once in `adj[v]` with matching `id`.

## Structure A (Tarjan Low-Link) Invariants
* **Root Rule:** A DFS tree root $r$ is in `articulation_points` iff $r$ has $\ge 2$ children in the DFS tree.
* **Non-Root Rule:** A non-root vertex $u$ is in `articulation_points` iff $u$ has at least one DFS child $v$ with $\text{low}[v] \ge \text{disc}[u]$.
* **BCC Edge Partition:** Every non-self-loop edge `id` belongs to exactly one biconnected component in `bcc_edge_ids` ($\sum \vert{}BCC_i\vert{} = E_{\text{non-loop}}$).

## Structure B (Union-Find Path Marking) Invariants
* **Bridge Equivalence:** `structA.bridge_ids == structB.bridge_ids` on all inputs up to $V = 100,000$.
* **2-ECC Vertex Partition:** `two_eccs` partitions $\{0, 1, \dots, V-1\}$ into disjoint sets ($\sum \vert{}C_i\vert{} = V$), and every bridge connects two distinct 2-ECCs.

## Oracle Agreement Invariant ($V \le 5000$)
* `structA.articulation_points == oracle.articulation_points`
* `structA.bridge_ids == structB.bridge_ids == oracle.bridge_ids`
* `structA.components_after_removal == oracle.components_after_removal`

## Block-Cut Tree Invariants (Application - D3)
* **Bipartite Invariant:** Every edge in the Block-Cut Tree connects one Cut-Vertex node (C-node) and one Block node (B-node).
* **Acyclic Forest Invariant:** Each connected component of the Block-Cut Forest with $n_C$ C-nodes and $n_B$ B-nodes has exactly $n_C + n_B - 1$ edges and zero cycles.
* **Separation Equivalence:** On small graphs, `bct.separates(v, x, y)` matches brute-force BFS reachability between $x$ and $y$ in $G \setminus \{v\}$.

---

# Section 4: Contract for Testing and Hidden Evaluation

```bash
./bin/run_ops --structure A < ops.txt > out.txt
./bin/run_ops --structure B < ops.txt > out.txt
```
make test runs the seeded differential test ($10^6$ operations, seed 20260928) and prints ALL TESTS PASSED.