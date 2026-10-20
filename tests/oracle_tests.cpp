#include "../oracle/bfs_oracle.hpp"

static int fails = 0;
#define CHECK(c, m) do { if (!(c)) { printf("FAIL: %s\n", m); fails++; } } while (0)

// ---- hand-checked cases ----
static void chk(const char* name, const Graph& g,
                vector<int> ap, vector<int> br, int C0) {
    OracleResult r = BfsOracle::solve(g);
    CHECK(r.articulation_points == ap, name);
    CHECK(r.bridge_ids == br, name);
    CHECK(r.initial_components == C0, name);
    CHECK((int)r.components_after_removal.size() == g.V, name);
    CHECK(r.bridges.size() == r.bridge_ids.size(), name);
}

// ---- independent reference: recursive Tarjan (small V only) ----
struct Ref {
    const Graph& g;
    vector<int> disc, low;
    vector<char> isAP;
    vector<int> bridges;
    int timer = 0;
    Ref(const Graph& gr) : g(gr), disc(gr.V, -1), low(gr.V, 0), isAP(gr.V, 0) {}
    void dfs(int u, int pe) {
        disc[u] = low[u] = timer++;
        int children = 0;
        for (const AdjEdge& e : g.adj[u]) {
            if (e.id == pe) continue;                    // skip parent EDGE (handles parallel edges)
            if (disc[e.to] != -1) { low[u] = min(low[u], disc[e.to]); continue; }
            children++;
            dfs(e.to, e.id);
            low[u] = min(low[u], low[e.to]);
            if (low[e.to] > disc[u]) bridges.push_back(e.id);
            if (pe != -1 && low[e.to] >= disc[u]) isAP[u] = 1;
        }
        if (pe == -1 && children >= 2) isAP[u] = 1;      // root rule
    }
    void run() { for (int i = 0; i < g.V; i++) if (disc[i] == -1) dfs(i, -1); sort(bridges.begin(), bridges.end()); }
};

int main() {
    { Graph g(5); for (int i = 0; i < 4; i++) g.add_edge(i, i + 1);
      chk("path5", g, {1, 2, 3}, {0, 1, 2, 3}, 1); }

    { Graph g(6); for (int i = 0; i < 6; i++) g.add_edge(i, (i + 1) % 6);
      chk("cycle6", g, {}, {}, 1); }

    { Graph g(6); for (int i = 1; i < 6; i++) g.add_edge(0, i);
      chk("star", g, {0}, {0, 1, 2, 3, 4}, 1);
      CHECK(BfsOracle::solve(g).components_after_removal[0] == 5, "star center comps"); }

    { Graph g(5);
      g.add_edge(0, 1); g.add_edge(1, 2); g.add_edge(2, 0);
      g.add_edge(2, 3); g.add_edge(3, 4); g.add_edge(4, 2);
      chk("bowtie", g, {2}, {}, 1); }

    { Graph g(2); g.add_edge(0, 1); g.add_edge(1, 0);
      chk("parallel", g, {}, {}, 1); }

    { Graph g(3); g.add_edge(0, 1); g.add_edge(1, 1);
      chk("selfloop+isolated", g, {}, {0}, 2);
      CHECK(BfsOracle::solve(g).components_after_removal[2] == 1, "isolated removal = C0-1"); }

    { Graph g(0); chk("V0", g, {}, {}, 0); }
    { Graph g(1); chk("V1", g, {}, {}, 1);
      CHECK(BfsOracle::solve(g).components_after_removal[0] == 0, "V1 removal"); }

    // ---- randomized: oracle vs independent Tarjan, incl. self-loops, parallel edges, disconnected ----
    mt19937 rng(20260928);
    for (int t = 0; t < 2000; t++) {
        int V = rng() % 25 + 1, E = rng() % 50;
        Graph g(V);
        for (int i = 0; i < E; i++) g.add_edge(rng() % V, rng() % V);
        OracleResult o = BfsOracle::solve(g);
        Ref r(g); r.run();
        vector<int> refAP;
        for (int v = 0; v < V; v++) if (r.isAP[v]) refAP.push_back(v);
        CHECK(o.articulation_points == refAP, "random: articulation points vs Tarjan");
        CHECK(o.bridge_ids == r.bridges, "random: bridges vs Tarjan");
    }

    printf(fails ? "FAILED (%d)\n" : "ORACLE TESTS PASSED\n", fails);
    return fails != 0;
}