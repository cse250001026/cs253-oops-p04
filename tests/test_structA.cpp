#include "../src/structA/tarjan_lowlink.hpp"
#include <random>
#include <stdexcept>

using namespace std;

void check(bool condition, const string& message) {
    if (!condition) throw runtime_error(message);
}

Graph makeGraph(int n, initializer_list<pair<int, int>> edges) {
    Graph g(n);
    for (auto [u, v] : edges) g.add_edge(u, v);
    return g;
}

// Independent connectivity checker. -1 means no vertex/edge is removed.
int countComponents(const Graph& g, int removedVertex = -1, int removedEdge = -1) {
    vector<char> seen(g.V, false);
    queue<int> q;
    int components = 0;
    for (int start = 0; start < g.V; ++start) {
        if (start == removedVertex || seen[start]) continue;
        ++components;
        seen[start] = true;
        q.push(start);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (const AdjEdge& e : g.adj[u]) {
                if (e.id == removedEdge || e.to == removedVertex || seen[e.to]) continue;
                seen[e.to] = true;
                q.push(e.to);
            }
        }
    }
    return components;
}

void verifyOracle(const Graph& g, const StructAResult& r) {
    int original = countComponents(g);
    check((int)r.components_after_removal.size() == g.V, "Removal count size mismatch");

    vector<int> expectedArticulation, expectedBridges;
    for (int v = 0; v < g.V; ++v) {
        int c = countComponents(g, v);
        check(r.components_after_removal[v] == c,
              "Incorrect components_after_removal for vertex " + to_string(v));
        if (c > original) expectedArticulation.push_back(v);
    }
    for (int id = 0; id < g.E; ++id) {
        if (countComponents(g, -1, id) > original) expectedBridges.push_back(id);
    }
    check(r.articulation_points == expectedArticulation, "Articulation points disagree with BFS oracle");
    check(r.bridge_ids == expectedBridges, "Bridge IDs disagree with BFS oracle");
    check(r.bridges.size() == r.bridge_ids.size(), "Bridge object count incorrect");
    for (size_t i = 0; i < r.bridges.size(); ++i) {
        check(r.bridges[i] == g.edges[r.bridge_ids[i]], "Bridge object differs from graph edge");
    }
}

//Checks partition and consistency of BCC outputs (not a complete BCC oracle).
void verifyBccInvariants(const Graph& g, const StructAResult& r) {
    check(r.bcc_edge_ids.size() == r.bcc_vertices.size(), "BCC edge/vertex list count differs");
    vector<int> edgeUse(g.E, 0);
    int previousMin = -1;

    for (size_t b = 0; b < r.bcc_edge_ids.size(); ++b) {
        const auto& ids = r.bcc_edge_ids[b];
        const auto& vertices = r.bcc_vertices[b];
        check(!ids.empty(), "Empty BCC edge list");
        check(is_sorted(ids.begin(), ids.end()), "BCC edges not sorted");
        check(adjacent_find(ids.begin(), ids.end()) == ids.end(), "Repeated edge in BCC");
        check(is_sorted(vertices.begin(), vertices.end()), "BCC vertices not sorted");
        check(adjacent_find(vertices.begin(), vertices.end()) == vertices.end(), "Repeated vertex in BCC");
        check(ids.front() > previousMin, "BCCs not ordered by smallest edge ID");
        previousMin = ids.front();

        vector<int> expectedVertices;
        for (int id : ids) {
            check(id >= 0 && id < g.E, "Invalid edge ID in BCC");
            check(g.edges[id].u != g.edges[id].v, "Self-loop should be omitted under graph.hpp convention");
            ++edgeUse[id];
            expectedVertices.push_back(g.edges[id].u);
            expectedVertices.push_back(g.edges[id].v);
        }
        sort(expectedVertices.begin(), expectedVertices.end());
        expectedVertices.erase(unique(expectedVertices.begin(), expectedVertices.end()), expectedVertices.end());
        check(vertices == expectedVertices, "BCC vertex list doesn't match edge endpoints");
    }
    for (const Edge& e : g.edges) {
        check(edgeUse[e.id] == (e.u == e.v ? 0 : 1), "Edge missing from BCC or belongs to multiple BCCs");
    }
}

void verifyCase(const string& title, const Graph& g,
                const vector<int>& articulation, const vector<int>& bridges,
                const vector<vector<int>>& bccs,
                const vector<int>& removalCounts) {
    auto r = runStructA(g);
    check(r.articulation_points == articulation, title + ": wrong articulation points");
    check(r.bridge_ids == bridges, title + ": wrong bridges");
    check(r.bcc_edge_ids == bccs, title + ": wrong BCCs");
    check(r.components_after_removal == removalCounts, title + ": wrong deletion counts");
    verifyOracle(g, r);
    verifyBccInvariants(g, r);
    cout << "[PASS] " << title << '\n';
}

void fixedTests() {
    verifyCase("4-vertex path", makeGraph(4, {{0,1},{1,2},{2,3}}),
               {1,2}, {0,1,2}, {{0},{1},{2}}, {1,2,2,1});
    verifyCase("triangle", makeGraph(3, {{0,1},{1,2},{2,0}}),
               {}, {}, {{0,1,2}}, {1,1,1});
    verifyCase("triangle plus tail", makeGraph(4, {{0,1},{1,2},{2,0},{2,3}}),
               {2}, {3}, {{0,1,2},{3}}, {1,1,2,1});
    verifyCase("two disconnected triangles", makeGraph(6, {{0,1},{1,2},{2,0},{3,4},{4,5},{5,3}}),
               {}, {}, {{0,1,2},{3,4,5}}, {2,2,2,2,2,2});
    verifyCase("two parallel edges", makeGraph(2, {{0,1},{0,1}}),
               {}, {}, {{0,1}}, {1,1});
    verifyCase("three isolated vertices", makeGraph(3, {}),
               {}, {}, {}, {2,2,2});
    verifyCase("empty graph", makeGraph(0, {}),
               {}, {}, {}, {});
    verifyCase("single vertex", makeGraph(1, {}),
               {}, {}, {}, {0});
    verifyCase("star", makeGraph(5, {{0,1},{0,2},{0,3},{0,4}}),
               {0}, {0,1,2,3}, {{0},{1},{2},{3}}, {4,1,1,1,1});
    verifyCase("triangle and parallel tail", makeGraph(4, {{0,1},{1,2},{2,0},{2,3},{2,3}}),
               {2}, {}, {{0,1,2},{3,4}}, {1,1,2,1});
    verifyCase("self-loop ignored for connectivity", makeGraph(2, {{0,0},{0,1}}),
               {}, {1}, {{1}}, {1,1});
}

void randomTests() {
    mt19937 rng(20261018); // Fixed seed
    constexpr int TRIALS = 3000;
    for (int t = 0; t < TRIALS; ++t) {
        int n = uniform_int_distribution<int>(0, 9)(rng);
        Graph g(n);
        if (n > 0) {
            int m = uniform_int_distribution<int>(0, 22)(rng);
            for (int i = 0; i < m; ++i) {
                int u = uniform_int_distribution<int>(0, n - 1)(rng);
                int v = uniform_int_distribution<int>(0, n - 1)(rng);
                g.add_edge(u, v); // Includes loops, duplicates and disconnected cases.
            }
        }
        try {
            StructAResult r = runStructA(g);
            verifyOracle(g, r);
            verifyBccInvariants(g, r);
        } catch (const exception& e) {
            cerr << "Random test " << t << " FAILED: " << e.what() << '\n';
            cerr << "V=" << g.V << ", edges (ID:u-v): ";
            for (const Edge& e : g.edges)
                cerr << e.id << ':' << e.u << '-' << e.v << ' ';
            cerr << '\n';
            throw;
        }
    }
    cout << "[PASS] " << TRIALS << " randomized brute-force tests\n";
}

void stressTests() {
    constexpr int N = 100000;
    {
        Graph g(N);
        for (int i = 1; i < N; ++i) g.add_edge(i - 1, i);
        auto r = runStructA(g);
        check((int)r.bridge_ids.size() == N - 1, "Stress path: bridge count");
        check((int)r.articulation_points.size() == N - 2, "Stress path: articulation count");
        check((int)r.bcc_edge_ids.size() == N - 1, "Stress path: BCC count");
        check(r.components_after_removal[N / 2] == 2, "Stress path: removal count");
        cout << "[PASS] 100,000-vertex path\n";
    }
    {
        Graph g(N);
        for (int i = 0; i < N; ++i) g.add_edge(i, (i+1)%N);
        auto r = runStructA(g);
        check(r.bridge_ids.empty() && r.articulation_points.empty(), "Stress cycle: unexpected cut");
        check(r.bcc_edge_ids.size() == 1 && r.bcc_edge_ids[0].size() == N,
              "Stress cycle: expected one BCC");
        cout << "[PASS] 100,000-vertex cycle\n";
    }
    {
        Graph g(N);
        for (int i = 1; i < N; ++i) g.add_edge(0, i);
        auto r = runStructA(g);
        check(r.articulation_points == vector<int>{0}, "Stress star: articulation points");
        check((int)r.bridge_ids.size() == N-1, "Stress star: bridge count");
        check(r.components_after_removal[0] == N-1, "Stress star: removal count");
        cout << "[PASS] 100,000-vertex star\n";
    }
    {
        Graph g(N);
        //Stress test with 100,000 vertices and 1,000,000 edges in a single biconnected component.
        for (int i = 0; i < N; ++i) g.add_edge(i, (i+1)%N);
        for (int i = 0; i < 900000; ++i) g.add_edge(i%N, (i%N+17)%N);
        auto r = runStructA(g);
        check(r.bridge_ids.empty() && r.articulation_points.empty(), "Million-edge stress: unexpected cut");
        check(r.bcc_edge_ids.size() == 1 && r.bcc_edge_ids[0].size() == (size_t)g.E,
              "Million-edge stress: expected one BCC");
        cout << "[PASS] " << g.V << " vertices, " << g.E << " edges\n";
    }
}

int main(int argc, char* argv[]) {
    try {
        fixedTests();
        randomTests();
        if (argc > 1 && string(argv[1]) == "--stress") stressTests();
        cout << "\nALL STRUCTURE A TESTS PASSED\n";
        return 0;
    } catch (const exception& e) {
        cerr << "\nTEST SUITE FAILED: " << e.what() << '\n';
        return 1;
    }
}
