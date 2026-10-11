# INTERFACE

# INTERFACE.md: Common Interface & Structural Invariants

**Project 4: Critical Nodes and Links in a Network**  
**Role:** Architect (D3) — Sufiyan Ahmad (Roll No: 250001068)  
**Course Lab:** CS 253 / MA 253 Data Structures and Algorithms Laboratory (Autumn 2026)

---

```
Here is how we represent an edge, graph, and the exact format we all agree to use. Please respond if something needs refinement.
```

## 1. Architectural Overview & Shared Abstraction

To ensure modularity and enforce that `Struc A, D1, Chetna` (Iterative Tarjan Low-Link), `Struc B, D2, Mayank` (Union-Find Path Marking), `Oracle, D4, Shubham` (Brute-force verification), and `App, D3, Sufiyan` (Block-Cut Tree Query Engine) interact seamlessly, all modules communicate via the standardized interface in `include/graph.hpp`.

### 1.1 Core Definitions

- **`Adjacency array`**: An Adjacency array is just an array of arrays or vector<int>.
- **`Graph`**: An **undirected graph** represented as an adjacency array.
  - Scale: $V \le 100,000$, $E \le 1,000,000$.
  - Vertex labels: $0, 1, \dots, V-1$.
  - Edge IDs: $0, 1, \dots, E-1$.
- **`BlockCutTree` (Owned by D3 - Architect(Me))**:
  - Bipartite tree structure decomposing the graph into **Cut Vertices (C-nodes)** and **Blocks/BCCs (B-nodes)**.
  - Supports queries: `separates(v, x, y)` $\to$ returns `true` if node $v$ cuts $x$ off from $y$.
  - Query time complexity: $O(1)$ after $O(V)$ preprocessing using Tree Entry/Exit timestamps (Euler tour traversal) and Lowest Common Ancestor (LCA) binary lifting.
  - Robustness ranking: Identifies critical nodes sorted by the number of components created upon their deletion.

---

## 2. API Specifications

### 2.1 Structure A Interface (`src/structA/tarjan_lowlink.hpp`)

```cpp
//code here
```

### 2.2 Structure B Interface (`src/structB/union_find_bridges.hpp`)

```cpp
//code here

```

### 2.3 Application Interface (`app/block_cut_tree.hpp`)

```cpp
//code here
```

## 3. Structural Invariants

Will write subsequently.

## 4. Contract for Testing and Hidden Evaluation
