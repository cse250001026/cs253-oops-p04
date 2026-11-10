This folder contains the information about brute force BFS oracle and differencial test used to check the optimized graph algorithms.

1. BFS oracle (bfs_oracle.hpp)

BfsOracle::solve(const Graph& g) returns the Oracle Result;

count_comp(g, delNode, vis, que), counts connected componenets using BFS. Setting the deletion argument to -1 means nothing is delted. A deleted vertex is never visited and a deleted edge is ignored by its edge ID;

The solver does three things:
1. Counts the connected components of the original graph, C0.
2. Removes every vertex v in turn and stores the resulting count in components_after_removal[v]. If this count is greater than C0, then v is an articulation point.
3. Removes every non-self-loop edge in turn. If the component count increases, the edge is a bridge.

- Using edge IDs matters for parallel edges: deleting one copy must not delete the other.
- Self-loops cannot be bridges. In our Graph interface they are stored in edges but not added to adj.
- The graph may already be disconnected, so the condition is greater than C0, not greater than 1.
- Removing an isolated vertex decreases the component count by one; it is not an articulation point.
- vis and the array-backed BFS queue are reused to avoid allocating them for each deletion.

Complexity: One BFS-based component count is O(V + E). Testing all vertices and all edges takes O(V(V + E) + E(V + E)) = O((V + E)^2) time in this implementation, with O(V) auxiliary space. 

2. Differential test (differential_test.cpp)

This runs the three implementations on the same graph : 
- Structure A: runStructA(g)
- Structure B: UnionFindBridgeSolver::solve(g)
- Oracle: BfsOracle::solve(g)

The compare(g) function checks the agreement rules from INTERFACE.md:
Result	Comparison
Articulation points	A == Oracle
Bridge IDs	A == B == Oracle
Components after removing each vertex	A == Oracle


We compare bridge IDs rather than lists of Edge objects because the solvers may store the edge objects in different orders.

The test first checks an empty graph. Then it repeatedly generates graphs with 1–30 vertices, adds up to 60 random edges per graph, and calls compare(g) before the first addition and after each addition. One operation currently means one add_edge(u, v) followed by a comparison; this definition is proposed in the c2 task sheet and should be confirmed with the team/TA if needed.

The random generator is mt19937, with default seed 20260928, so a run can be reproduced. The program accepts the operation count and seed as command-line arguments. It saves the edge additions in ops and, on a mismatch, prints the seed, graph, operation number, mismatching result and solver outputs before exiting with an error.
Build and run
From the repository root:
mkdir -p bin

g++ -std=c++17 -O2 -Wall -Wextra \
    oracle/differential_test.cpp \
    src/structB/DFSForest.cpp \
    src/structB/DSU.cpp \
    src/structB/makeCyclePath.cpp \
    src/structB/union_find_bridges.cpp \
    -o bin/differential_test

./bin/differential_test 100000 20260928
./bin/differential_test 1000000 20260928

A successful 100,000-operation run should print:

DIFFERENTIAL TEST PASSED: 100000 operations, seed 20260928

The existing oracle-specific tests are in tests/oracle_tests.cpp. They include hand-written graphs and 2,000 random multigraphs compared with an independent recursive Tarjan reference.