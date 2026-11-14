/**
 * @file test_structB.cpp
 * @brief Comprehensive test suite for Structure B (Union-Find Path Marking Bridge Solver).
 *
 * Verifies all structural invariants defined in Section 3 of INTERFACE.md:
 *  - Graph Symmetry & ID Invariant:
 *      * Canonical endpoints (edges[id].u < edges[id].v for u != v).
 *      * Contiguous edge IDs [0, E-1].
 *      * Exactly one appearance in adj[u] and adj[v].
 *      * Exclusion of self-loops from adj lists.
 *  - Structure B Invariants:
 *      * Bridge Equivalence: structA.bridge_ids == structB.bridge_ids up to V = 100,000.
 *      * 2-ECC Vertex Partition: two_eccs partitions {0, 1, ..., V-1} into disjoint sets (sum |C_i| = V).
 *      * Bridge Crossing: every bridge connects two distinct 2-ECCs.
 *      * Non-Bridge Invariant: every non-bridge non-self-loop edge connects vertices in the same 2-ECC.
 *      * Reachability Invariant: connected components in G \ Bridges match two_eccs exactly.
 *  - Oracle Agreement Invariant (V <= 5000):
 *      * structA.bridge_ids == structB.bridge_ids == oracle.bridge_ids.
 *
 * Contains:
 *  1. fixedTests(): 25 hand-crafted corner cases (empty, isolated, loops, parallel, trees, cycles, disconnected).
 *  2. randomTests(): Multi-thousand randomized tests comparing against BFS Oracle and StructA.
 *  3. stressTests(): Scaled tests up to V = 100,000 and E = 1,000,000.
 *
 * Compilation:
 *   g++ -O2 -std=c++14 tests/test_structB.cpp src/structB/*.cpp -o bin/test_structB
 */

#include "../src/structB/union_find_bridges.hpp"
#include "../src/structA/tarjan_lowlink.hpp"
#include "../oracle/bfs_oracle.hpp"

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <chrono>

using namespace std;

// -----------------------------------------------------------------------------
// Assertions and Helper Functions
// -----------------------------------------------------------------------------

static void check(bool condition, const string& message) {
    if (!condition) {
        throw runtime_error(message);
    }
}

static Graph makeGraph(int n, initializer_list<pair<int, int>> edges) {
    Graph g(n);
    for (const auto& e : edges) {
        g.add_edge(e.first, e.second);
    }
    return g;
}

// -----------------------------------------------------------------------------
// Invariant Verifiers (INTERFACE.md Section 3)
// -----------------------------------------------------------------------------

/**
 * @brief Verifies Graph Symmetry & ID Invariant (INTERFACE.md Section 3):
 * Every edge id in [0, E-1] with u != v satisfies 0 <= edges[id].u < edges[id].v < V,
 * and appears once in adj[u] and once in adj[v] with matching id.
 */
static void verifyGraphInvariants(const Graph& g) {
    check((int)g.edges.size() == g.E, "Graph edges vector size does not match E");
    for (int id = 0; id < g.E; ++id) {
        const Edge& e = g.edges[id];
        check(e.id == id, "Edge ID mismatch at edges[" + to_string(id) + "]");
        check(e.u <= e.v, "Canonical edge requires u <= v");
        check(e.u >= 0 && e.v < g.V, "Edge endpoints out of bounds [0, V-1]");

        if (e.u != e.v) {
            check(e.u < e.v, "Non-loop edge requires u < v");

            // Must appear once in adj[u] with matching ID and neighbor v
            int count_u = 0;
            for (const AdjEdge& ae : g.adj[e.u]) {
                if (ae.id == id) {
                    check(ae.to == e.v, "adj[u] neighbor does not match edge.v");
                    ++count_u;
                }
            }
            check(count_u == 1, "Edge " + to_string(id) + " must appear exactly once in adj[" + to_string(e.u) + "]");

            // Must appear once in adj[v] with matching ID and neighbor u
            int count_v = 0;
            for (const AdjEdge& ae : g.adj[e.v]) {
                if (ae.id == id) {
                    check(ae.to == e.u, "adj[v] neighbor does not match edge.u");
                    ++count_v;
                }
            }
            check(count_v == 1, "Edge " + to_string(id) + " must appear exactly once in adj[" + to_string(e.v) + "]");
        } else {
            // Self-loop: must not appear in adj[u] per INTERFACE.md and graph.hpp
            for (const AdjEdge& ae : g.adj[e.u]) {
                check(ae.id != id, "Self-loop edge " + to_string(id) + " must not appear in adj list");
            }
        }
    }
}

/**
 * @brief Independent BFS component counter.
 * Supports simulating vertex removal (removedVertex) or edge removal (removedEdge).
 */
static int countComponents(const Graph& g, int removedVertex = -1, int removedEdge = -1) {
    vector<char> seen(g.V, false);
    queue<int> q;
    int components = 0;
    for (int start = 0; start < g.V; ++start) {
        if (start == removedVertex || seen[start]) continue;
        ++components;
        seen[start] = true;
        q.push(start);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (const AdjEdge& e : g.adj[u]) {
                if (e.id == removedEdge || e.to == removedVertex || seen[e.to]) continue;
                seen[e.to] = true;
                q.push(e.to);
            }
        }
    }
    return components;
}

/**
 * @brief Verifies Structure B specific structural invariants (INTERFACE.md Section 3):
 * 1. Bridge Output Invariants:
 *    - bridge_ids sorted in strictly ascending order with valid indices [0, E-1].
 *    - bridges.size() == bridge_ids.size().
 *    - bridges[i] == g.edges[bridge_ids[i]].
 *    - No bridge is a self-loop.
 * 2. 2-ECC Vertex Partition Invariant:
 *    - two_eccs partitions {0, 1, ..., V-1} into disjoint sets (sum |C_i| = V).
 *    - Each component is non-empty (if V > 0) and sorted ascending with no duplicates.
 *    - Components are ordered deterministically by minimum vertex ID.
 * 3. Bridge Crossing Invariant:
 *    - Every bridge connects two distinct 2-ECCs.
 * 4. Non-Bridge Edge Invariant:
 *    - Every non-bridge edge (excluding self-loops) connects vertices within the SAME 2-ECC.
 * 5. Reachability Equivalence Invariant:
 *    - Connected components of G \ Bridges match two_eccs exactly.
 */
static void verifyStructBInvariants(const Graph& g, const StructBResult& r) {
    // 1. Bridge Output Invariants
    check(r.bridges.size() == r.bridge_ids.size(), "Bridges object count does not match bridge_ids count");
    check(is_sorted(r.bridge_ids.begin(), r.bridge_ids.end()), "Bridge IDs are not sorted ascending");
    check(adjacent_find(r.bridge_ids.begin(), r.bridge_ids.end()) == r.bridge_ids.end(), "Duplicate bridge IDs found");

    vector<bool> is_bridge(g.E, false);
    for (size_t i = 0; i < r.bridge_ids.size(); ++i) {
        int id = r.bridge_ids[i];
        check(id >= 0 && id < g.E, "Bridge ID " + to_string(id) + " out of range [0, E-1]");
        check(r.bridges[i] == g.edges[id], "Bridge Edge object differs from graph edge at ID " + to_string(id));
        check(g.edges[id].u != g.edges[id].v, "Self-loop edge cannot be a bridge");
        is_bridge[id] = true;
    }

    // 2. 2-ECC Vertex Partition Invariant
    if (g.V == 0) {
        check(r.two_eccs.empty(), "two_eccs must be empty when V == 0");
    } else {
        check(!r.two_eccs.empty(), "two_eccs cannot be empty when V > 0");
    }

    vector<int> vertex_to_ecc(g.V, -1);
    int total_vertices = 0;
    int prev_comp_min = -1;

    for (size_t c = 0; c < r.two_eccs.size(); ++c) {
        const auto& comp = r.two_eccs[c];
        check(!comp.empty(), "2-ECC component " + to_string(c) + " is empty");
        check(is_sorted(comp.begin(), comp.end()), "2-ECC component " + to_string(c) + " vertices are not sorted");
        check(adjacent_find(comp.begin(), comp.end()) == comp.end(), "Duplicate vertex inside 2-ECC component " + to_string(c));

        // Check canonical ordering of components by minimum vertex ID
        check(comp.front() > prev_comp_min, "2-ECC components are not ordered by minimum vertex ID");
        prev_comp_min = comp.front();

        for (int v : comp) {
            check(v >= 0 && v < g.V, "Vertex ID " + to_string(v) + " in 2-ECC out of range [0, V-1]");
            check(vertex_to_ecc[v] == -1, "Vertex " + to_string(v) + " belongs to multiple 2-ECCs (not disjoint)");
            vertex_to_ecc[v] = (int)c;
            ++total_vertices;
        }
    }
    check(total_vertices == g.V, "Sum of 2-ECC sizes (" + to_string(total_vertices) + ") does not equal V (" + to_string(g.V) + ")");
    for (int v = 0; v < g.V; ++v) {
        check(vertex_to_ecc[v] != -1, "Vertex " + to_string(v) + " does not belong to any 2-ECC");
    }

    // 3. Bridge Crossing Invariant: every bridge connects two distinct 2-ECCs
    for (int id : r.bridge_ids) {
        int u = g.edges[id].u;
        int v = g.edges[id].v;
        check(vertex_to_ecc[u] != vertex_to_ecc[v],
              "Bridge " + to_string(id) + " connects vertices in the same 2-ECC (" + to_string(vertex_to_ecc[u]) + ")");
    }

    // 4. Non-Bridge Edge Invariant: every non-bridge non-loop edge connects vertices in the same 2-ECC
    for (int id = 0; id < g.E; ++id) {
        if (!is_bridge[id] && g.edges[id].u != g.edges[id].v) {
            int u = g.edges[id].u;
            int v = g.edges[id].v;
            check(vertex_to_ecc[u] == vertex_to_ecc[v],
                  "Non-bridge edge " + to_string(id) + " connects vertices in different 2-ECCs");
        }
    }

    // 5. Reachability Equivalence Invariant: connected components in G \ Bridges match two_eccs exactly
    vector<int> bfs_comp(g.V, -1);
    int bfs_comp_count = 0;
    for (int start = 0; start < g.V; ++start) {
        if (bfs_comp[start] != -1) continue;
        int comp_id = bfs_comp_count++;
        queue<int> q;
        bfs_comp[start] = comp_id;
        q.push(start);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (const AdjEdge& ae : g.adj[u]) {
                if (is_bridge[ae.id]) continue;
                if (bfs_comp[ae.to] == -1) {
                    bfs_comp[ae.to] = comp_id;
                    q.push(ae.to);
                }
            }
        }
    }
    check(bfs_comp_count == (int)r.two_eccs.size(),
          "Count of connected components in G \\ Bridges (" + to_string(bfs_comp_count) +
          ") does not match two_eccs count (" + to_string(r.two_eccs.size()) + ")");

    for (int v = 0; v < g.V; ++v) {
        int rep = r.two_eccs[vertex_to_ecc[v]][0];
        check(bfs_comp[v] == bfs_comp[rep],
              "2-ECC connectivity mismatch with G \\ Bridges for vertex " + to_string(v));
    }
}

/**
 * @brief Verifies Oracle Agreement and Structure A Bridge Equivalence (INTERFACE.md Section 3):
 *  - structA.bridge_ids == structB.bridge_ids
 *  - structA.bridge_ids == structB.bridge_ids == oracle.bridge_ids (for V <= 5000)
 */
static void verifyOracleAndStructA(const Graph& g, const StructBResult& r) {
    // 1. Structure A Bridge Equivalence
    StructAResult rA = runStructA(g);
    check(r.bridge_ids == rA.bridge_ids, "Bridge IDs disagree with Structure A (Tarjan Low-Link)");

    // 2. Oracle Agreement (V <= 5000)
    if (g.V <= 5000) {
        OracleResult o = BfsOracle::solve(g);
        check(r.bridge_ids == o.bridge_ids, "Bridge IDs disagree with BFS Oracle");

        // Independent brute-force component check
        int C0 = countComponents(g);
        vector<int> expected_bridges;
        for (int id = 0; id < g.E; ++id) {
            if (g.edges[id].u == g.edges[id].v) continue; // Self-loops cannot be bridges
            if (countComponents(g, -1, id) > C0) {
                expected_bridges.push_back(id);
            }
        }
        check(r.bridge_ids == expected_bridges, "Bridge IDs disagree with independent edge removal counting");
    }
}

/**
 * @brief Helper to test a fixed case with explicit expected bridges and 2-ECCs.
 */
static void verifyCase(const string& title, const Graph& g,
                       const vector<int>& expectedBridges,
                       const vector<vector<int>>& expectedTwoECCs) {
    verifyGraphInvariants(g);
    StructBResult r = UnionFindBridgeSolver::solve(g);
    check(r.bridge_ids == expectedBridges, title + ": bridge IDs mismatch");
    check(r.two_eccs == expectedTwoECCs, title + ": two_eccs mismatch");
    verifyStructBInvariants(g, r);
    verifyOracleAndStructA(g, r);
    cout << "  [PASS] " << title << '\n';
}

// -----------------------------------------------------------------------------
// 1. Fixed Tests (Corner Cases, Hand-checked Topologies)
// -----------------------------------------------------------------------------

void fixedTests() {
    cout << "\n=== RUNNING FIXED TESTS ===" << endl;

    // 1. Empty graph
    verifyCase("empty graph", makeGraph(0, {}),
               {}, {});

    // 2. Single isolated vertex
    verifyCase("single vertex", makeGraph(1, {}),
               {}, {{0}});

    // 3. Multiple isolated vertices
    verifyCase("three isolated vertices", makeGraph(3, {}),
               {}, {{0}, {1}, {2}});

    // 4. Single bridge edge
    verifyCase("single edge (bridge)", makeGraph(2, {{0, 1}}),
               {0}, {{0}, {1}});

    // 5. Two parallel edges (forms a 2-cycle, no bridges)
    verifyCase("two parallel edges", makeGraph(2, {{0, 1}, {0, 1}}),
               {}, {{0, 1}});

    // 6. Three parallel edges (multi-graph, no bridges)
    verifyCase("three parallel edges", makeGraph(2, {{0, 1}, {0, 1}, {0, 1}}),
               {}, {{0, 1}});

    // 7. Self-loop on single vertex
    verifyCase("single vertex self-loop", makeGraph(1, {{0, 0}}),
               {}, {{0}});

    // 8. Self-loop plus isolated vertex
    verifyCase("self-loop plus isolated vertex", makeGraph(2, {{0, 0}}),
               {}, {{0}, {1}});

    // 9. Path graph P4 (all edges are bridges)
    verifyCase("4-vertex path", makeGraph(4, {{0, 1}, {1, 2}, {2, 3}}),
               {0, 1, 2}, {{0}, {1}, {2}, {3}});

    // 10. Triangle (cycle C3, no bridges)
    verifyCase("triangle (C3)", makeGraph(3, {{0, 1}, {1, 2}, {2, 0}}),
               {}, {{0, 1, 2}});

    // 11. Triangle plus pendant tail (tail is bridge)
    verifyCase("triangle plus tail", makeGraph(4, {{0, 1}, {1, 2}, {2, 0}, {2, 3}}),
               {3}, {{0, 1, 2}, {3}});

    // 12. Triangle plus parallel tail (tail is 2 parallel edges, no bridges)
    verifyCase("triangle plus parallel tail", makeGraph(4, {{0, 1}, {1, 2}, {2, 0}, {2, 3}, {2, 3}}),
               {}, {{0, 1, 2, 3}});

    // 13. Two disconnected triangles
    verifyCase("two disconnected triangles", makeGraph(6, {{0, 1}, {1, 2}, {2, 0}, {3, 4}, {4, 5}, {5, 3}}),
               {}, {{0, 1, 2}, {3, 4, 5}});

    // 14. Dumbbell graph (two triangles joined by a single bridge)
    verifyCase("dumbbell graph", makeGraph(6, {{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 5}, {5, 3}}),
               {3}, {{0, 1, 2}, {3, 4, 5}});

    // 15. Bowtie graph (two triangles sharing vertex 2, cut vertex but no bridges)
    verifyCase("bowtie graph", makeGraph(5, {{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 2}}),
               {}, {{0, 1, 2, 3, 4}});

    // 16. Star graph S5 (center 0 connected to 1, 2, 3, 4; all edges are bridges)
    verifyCase("star graph (S5)", makeGraph(5, {{0, 1}, {0, 2}, {0, 3}, {0, 4}}),
               {0, 1, 2, 3}, {{0}, {1}, {2}, {3}, {4}});

    // 17. 5-vertex cycle C5 (no bridges)
    verifyCase("5-vertex cycle (C5)", makeGraph(5, {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}}),
               {}, {{0, 1, 2, 3, 4}});

    // 18. Complete graph K4 (no bridges)
    verifyCase("complete graph (K4)", makeGraph(4, {{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}}),
               {}, {{0, 1, 2, 3}});

    // 19. Complete bipartite graph K2,3 (no bridges)
    verifyCase("complete bipartite graph (K2,3)", makeGraph(5, {{0, 2}, {0, 3}, {0, 4}, {1, 2}, {1, 3}, {1, 4}}),
               {}, {{0, 1, 2, 3, 4}});

    // 20. Theta graph (3 vertex-disjoint paths between 0 and 3, no bridges)
    verifyCase("theta graph", makeGraph(4, {{0, 1}, {1, 3}, {0, 2}, {2, 3}, {0, 3}}),
               {}, {{0, 1, 2, 3}});

    // 21. Path with self-loops on each node
    verifyCase("path with self-loops", makeGraph(3, {{0, 0}, {0, 1}, {1, 1}, {1, 2}, {2, 2}}),
               {1, 3}, {{0}, {1}, {2}});

    // 22. Mixed disconnected graph (triangle, bridge, isolated, parallel, self-loop)
    verifyCase("mixed disconnected graph",
               makeGraph(9, {{0, 1}, {1, 2}, {2, 0}, {3, 4}, {6, 7}, {6, 7}, {8, 8}}),
               {3}, {{0, 1, 2}, {3}, {4}, {5}, {6, 7}, {8}});

    // 23. Cactus graph (three cycles linked by bridge edges)
    verifyCase("cactus graph",
               makeGraph(9, {{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 5}, {5, 3}, {5, 6}, {6, 7}, {7, 8}, {8, 6}}),
               {3, 7}, {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}});

    // 24. Binary tree of 7 vertices (all edges are bridges)
    verifyCase("tree graph (7 vertices)",
               makeGraph(7, {{0, 1}, {0, 2}, {1, 3}, {1, 4}, {2, 5}, {2, 6}}),
               {0, 1, 2, 3, 4, 5}, {{0}, {1}, {2}, {3}, {4}, {5}, {6}});

    // 25. Reverse-ordered edge addition (verifies stability against input order)
    verifyCase("reverse-order edges triangle+tail",
               makeGraph(4, {{2, 3}, {2, 0}, {1, 2}, {0, 1}}),
               {0}, {{0, 1, 2}, {3}});

    cout << "[PASS] All 25 Fixed Tests Passed Successfully!\n";
}

// -----------------------------------------------------------------------------
// 2. Randomized Tests (Small, Medium, Large vs Oracle & StructA)
// -----------------------------------------------------------------------------

void randomTests() {
    cout << "\n=== RUNNING RANDOMIZED TESTS ===" << endl;

    mt19937 rng(20261018); // Fixed deterministic seed per lab specs

    // --- Phase 1: Small Multi-graphs vs BFS Oracle & StructA ---
    constexpr int SMALL_TRIALS = 3000;
    for (int t = 0; t < SMALL_TRIALS; ++t) {
        int n = uniform_int_distribution<int>(0, 12)(rng);
        Graph g(n);
        if (n > 0) {
            int m = uniform_int_distribution<int>(0, 25)(rng);
            for (int i = 0; i < m; ++i) {
                int u = uniform_int_distribution<int>(0, n - 1)(rng);
                int v = uniform_int_distribution<int>(0, n - 1)(rng);
                g.add_edge(u, v); // Generates self-loops, parallel edges, disconnected components
            }
        }
        try {
            verifyGraphInvariants(g);
            StructBResult r = UnionFindBridgeSolver::solve(g);
            verifyStructBInvariants(g, r);
            verifyOracleAndStructA(g, r);
        } catch (const exception& e) {
            cerr << "Random small test " << t << " FAILED: " << e.what() << '\n';
            cerr << "V=" << g.V << ", edges (ID:u-v): ";
            for (const Edge& edge : g.edges) cerr << edge.id << ':' << edge.u << '-' << edge.v << ' ';
            cerr << '\n';
            throw;
        }
    }
    cout << "  [PASS] " << SMALL_TRIALS << " small random multi-graph tests (vs BFS Oracle & StructA)\n";

    // --- Phase 2: Medium Graphs (V up to 150) vs BFS Oracle & StructA ---
    constexpr int MEDIUM_TRIALS = 100;
    for (int t = 0; t < MEDIUM_TRIALS; ++t) {
        int n = uniform_int_distribution<int>(15, 120)(rng);
        Graph g(n);
        // Topology mix: random tree base + random chords + random disconnected clusters
        int comp_count = uniform_int_distribution<int>(1, 4)(rng);
        int v_per_comp = n / comp_count;

        for (int c = 0; c < comp_count; ++c) {
            int start_v = c * v_per_comp;
            int end_v = (c == comp_count - 1) ? n : (c + 1) * v_per_comp;
            int comp_size = end_v - start_v;
            if (comp_size <= 1) continue;

            // Generate spanning tree for component
            for (int i = start_v + 1; i < end_v; ++i) {
                int p = uniform_int_distribution<int>(start_v, i - 1)(rng);
                g.add_edge(p, i);
            }
            // Add chords to create cycles
            int chords = uniform_int_distribution<int>(0, comp_size)(rng);
            for (int i = 0; i < chords; ++i) {
                int u = uniform_int_distribution<int>(start_v, end_v - 1)(rng);
                int v = uniform_int_distribution<int>(start_v, end_v - 1)(rng);
                g.add_edge(u, v);
            }
        }

        try {
            verifyGraphInvariants(g);
            StructBResult r = UnionFindBridgeSolver::solve(g);
            verifyStructBInvariants(g, r);
            verifyOracleAndStructA(g, r);
        } catch (const exception& e) {
            cerr << "Random medium test " << t << " FAILED: " << e.what() << '\n';
            throw;
        }
    }
    cout << "  [PASS] " << MEDIUM_TRIALS << " medium random structured graph tests (vs BFS Oracle & StructA)\n";

    // --- Phase 3: Larger Random Graphs (V up to 1000) vs StructA ---
    constexpr int LARGE_TRIALS = 30;
    for (int t = 0; t < LARGE_TRIALS; ++t) {
        int n = uniform_int_distribution<int>(200, 1000)(rng);
        Graph g(n);
        int m = uniform_int_distribution<int>(n, 3 * n)(rng);
        for (int i = 0; i < m; ++i) {
            int u = uniform_int_distribution<int>(0, n - 1)(rng);
            int v = uniform_int_distribution<int>(0, n - 1)(rng);
            g.add_edge(u, v);
        }
        try {
            verifyGraphInvariants(g);
            StructBResult rB = UnionFindBridgeSolver::solve(g);
            StructAResult rA = runStructA(g);
            check(rB.bridge_ids == rA.bridge_ids, "Bridge equivalence violated on large random graph");
            verifyStructBInvariants(g, rB);
        } catch (const exception& e) {
            cerr << "Random large test " << t << " FAILED: " << e.what() << '\n';
            throw;
        }
    }
    cout << "  [PASS] " << LARGE_TRIALS << " large random graph tests (vs StructA Bridge Equivalence)\n";
    cout << "[PASS] All Randomized Tests Passed Successfully!\n";
}

// -----------------------------------------------------------------------------
// 3. Stress Tests (Scale: V <= 100,000, E <= 1,000,000 per INTERFACE.md)
// -----------------------------------------------------------------------------

void stressTests() {
    cout << "\n=== RUNNING STRESS TESTS (V <= 100,000, E <= 1,000,000) ===" << endl;
    constexpr int N = 100000;

    // Test 1: 100,000-vertex Path Graph (all N-1 edges are bridges)
    {
        auto t0 = chrono::high_resolution_clock::now();
        Graph g(N);
        for (int i = 1; i < N; ++i) {
            g.add_edge(i - 1, i);
        }
        StructBResult rB = UnionFindBridgeSolver::solve(g);
        StructAResult rA = runStructA(g);

        check((int)rB.bridge_ids.size() == N - 1, "Stress path: wrong bridge count");
        check(rB.bridge_ids == rA.bridge_ids, "Stress path: bridge equivalence with StructA violated");
        check((int)rB.two_eccs.size() == N, "Stress path: expected N 2-ECC components of size 1");
        verifyStructBInvariants(g, rB);

        auto t1 = chrono::high_resolution_clock::now();
        double ms = chrono::duration<double, milli>(t1 - t0).count();
        cout << "  [PASS] 100,000-vertex Path (" << N - 1 << " bridges, " << N << " 2-ECCs) in " << ms << " ms\n";
    }

    // Test 2: 100,000-vertex Cycle Graph (0 bridges, 1 2-ECC of size N)
    {
        auto t0 = chrono::high_resolution_clock::now();
        Graph g(N);
        for (int i = 0; i < N; ++i) {
            g.add_edge(i, (i + 1) % N);
        }
        StructBResult rB = UnionFindBridgeSolver::solve(g);
        StructAResult rA = runStructA(g);

        check(rB.bridge_ids.empty(), "Stress cycle: unexpected bridge found");
        check(rB.bridge_ids == rA.bridge_ids, "Stress cycle: bridge equivalence with StructA violated");
        check(rB.two_eccs.size() == 1, "Stress cycle: expected exactly 1 2-ECC");
        check((int)rB.two_eccs[0].size() == N, "Stress cycle: 2-ECC size mismatch");
        verifyStructBInvariants(g, rB);

        auto t1 = chrono::high_resolution_clock::now();
        double ms = chrono::duration<double, milli>(t1 - t0).count();
        cout << "  [PASS] 100,000-vertex Cycle (0 bridges, 1 2-ECC) in " << ms << " ms\n";
    }

    // Test 3: 100,000-vertex Star Graph (center 0 connected to 1..N-1, all N-1 edges bridges)
    {
        auto t0 = chrono::high_resolution_clock::now();
        Graph g(N);
        for (int i = 1; i < N; ++i) {
            g.add_edge(0, i);
        }
        StructBResult rB = UnionFindBridgeSolver::solve(g);
        StructAResult rA = runStructA(g);

        check((int)rB.bridge_ids.size() == N - 1, "Stress star: wrong bridge count");
        check(rB.bridge_ids == rA.bridge_ids, "Stress star: bridge equivalence with StructA violated");
        check((int)rB.two_eccs.size() == N, "Stress star: expected N 2-ECC components");
        verifyStructBInvariants(g, rB);

        auto t1 = chrono::high_resolution_clock::now();
        double ms = chrono::duration<double, milli>(t1 - t0).count();
        cout << "  [PASS] 100,000-vertex Star (" << N - 1 << " bridges, " << N << " 2-ECCs) in " << ms << " ms\n";
    }

    // Test 4: Adversarial Triangle Chain (25,000 triangles linked by 24,999 bridges + 25,000 isolated vertices)
    {
        auto t0 = chrono::high_resolution_clock::now();
        Graph g(N);
        constexpr int TRIANGLES = 25000;
        // Build 25,000 triangles: triangle k has vertices 3k, 3k+1, 3k+2
        for (int k = 0; k < TRIANGLES; ++k) {
            int u = 3 * k;
            g.add_edge(u, u + 1);
            g.add_edge(u + 1, u + 2);
            g.add_edge(u + 2, u);
            // Connect to next triangle via a bridge
            if (k + 1 < TRIANGLES) {
                g.add_edge(u + 2, 3 * (k + 1));
            }
        }
        // Remaining vertices 75,000 to 99,999 are isolated
        StructBResult rB = UnionFindBridgeSolver::solve(g);
        StructAResult rA = runStructA(g);

        check((int)rB.bridge_ids.size() == TRIANGLES - 1, "Stress triangle chain: wrong bridge count");
        check(rB.bridge_ids == rA.bridge_ids, "Stress triangle chain: bridge equivalence violated");
        check((int)rB.two_eccs.size() == TRIANGLES + (N - 3 * TRIANGLES), "Stress triangle chain: wrong 2-ECC count");
        verifyStructBInvariants(g, rB);

        auto t1 = chrono::high_resolution_clock::now();
        double ms = chrono::duration<double, milli>(t1 - t0).count();
        cout << "  [PASS] Triangle Chain (" << TRIANGLES << " triangles, " << TRIANGLES - 1 << " bridges) in " << ms << " ms\n";
    }

    // Test 5: 1,000,000-Edge Dense Cycle-Rich Graph (100,000-vertex cycle + 900,000 chords)
    {
        auto t0 = chrono::high_resolution_clock::now();
        Graph g(N);
        for (int i = 0; i < N; ++i) {
            g.add_edge(i, (i + 1) % N);
        }
        for (int i = 0; i < 900000; ++i) {
            g.add_edge(i % N, (i + 17) % N);
        }
        StructBResult rB = UnionFindBridgeSolver::solve(g);
        StructAResult rA = runStructA(g);

        check(rB.bridge_ids.empty(), "Million-edge stress: unexpected bridge found");
        check(rB.bridge_ids == rA.bridge_ids, "Million-edge stress: bridge equivalence violated");
        check(rB.two_eccs.size() == 1, "Million-edge stress: expected 1 2-ECC");
        check((int)rB.two_eccs[0].size() == N, "Million-edge stress: 2-ECC size mismatch");
        verifyStructBInvariants(g, rB);

        auto t1 = chrono::high_resolution_clock::now();
        double ms = chrono::duration<double, milli>(t1 - t0).count();
        cout << "  [PASS] 1,000,000-Edge Graph (" << g.V << " vertices, " << g.E << " edges) in " << ms << " ms\n";
    }

    // Test 6: 100,000-Vertex Disconnected Forest (20,000 paths of length 5 each)
    {
        auto t0 = chrono::high_resolution_clock::now();
        Graph g(N);
        constexpr int PATHS = 20000;
        for (int p = 0; p < PATHS; ++p) {
            int base = p * 5;
            for (int i = 0; i < 4; ++i) {
                g.add_edge(base + i, base + i + 1);
            }
        }
        StructBResult rB = UnionFindBridgeSolver::solve(g);
        StructAResult rA = runStructA(g);

        check((int)rB.bridge_ids.size() == PATHS * 4, "Disconnected forest: wrong bridge count");
        check(rB.bridge_ids == rA.bridge_ids, "Disconnected forest: bridge equivalence violated");
        check((int)rB.two_eccs.size() == N, "Disconnected forest: expected N 2-ECC components");
        verifyStructBInvariants(g, rB);

        auto t1 = chrono::high_resolution_clock::now();
        double ms = chrono::duration<double, milli>(t1 - t0).count();
        cout << "  [PASS] Disconnected Forest (" << PATHS << " paths, " << PATHS * 4 << " bridges) in " << ms << " ms\n";
    }

    cout << "[PASS] All Stress Tests Passed Successfully!\n";
}

// -----------------------------------------------------------------------------
// Main Test Runner
// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    try {
        bool run_stress = false;
        for (int i = 1; i < argc; ++i) {
            if (string(argv[i]) == "--stress" || string(argv[i]) == "-s" || string(argv[i]) == "--all") {
                run_stress = true;
            }
        }

        fixedTests();
        randomTests();

        if (run_stress) {
            stressTests();
        } else {
            cout << "\n(Note: To run large-scale stress tests up to V=100,000 and E=1,000,000, pass --stress)\n";
        }

        cout << "\n===================================\n";
        cout << " ALL STRUCTURE B TESTS PASSED\n";
        cout << "===================================\n";
        return 0;
    } catch (const exception& e) {
        cerr << "\nTEST SUITE FAILED: " << e.what() << '\n';
        return 1;
    }
}
