#ifndef TARJAN_LOWLINK_HPP
#define TARJAN_LOWLINK_HPP

#include "../../include/graph.hpp"
#include "../../bench/counters.hpp"

inline StructAResult runStructA(const Graph& g) {
    int n = g.V, m = g.E;
    StructAResult res;

    vector<int> tin(n, -1), low(n), parent(n, -1), parentEdge(n, -1);
    vector<int> next(n, 0), separated(n, 0), bccId(m, -1);
    vector<char> bridge(m, false);
    vector<int> dfs, edgeStack;
    int timer = 0, components = 0, bccCount = 0;

    //Iterative Tarjan DFS.
    for (int root = 0; root < n; ++root) {
        if (tin[root] != -1) continue;
        ++components;
        tin[root] = low[root] = timer++;
        BENCH_INC(vertices_visited);
        dfs.push_back(root);
        BENCH_MAX(max_stack_depth, dfs.size());

        while (!dfs.empty()) {
            int u = dfs.back();

            if (next[u] == (int)g.adj[u].size()) {
                // All neighbors processed: return from u to its parent.
                dfs.pop_back();
                int p = parent[u];
                if (p == -1) continue;

                low[p] = min(low[p], low[u]);
                if (low[u] > tin[p]) bridge[parentEdge[u]] = true;

                if (low[u] >= tin[p]) {
                    ++separated[p];
                    int id;
                    do {
                        id = edgeStack.back();
                        edgeStack.pop_back();
                        bccId[id] = bccCount;
                    } while (id != parentEdge[u]);
                    ++bccCount;
                }
                continue;
            }

            AdjEdge e = g.adj[u][next[u]++];
            BENCH_INC(half_edges_scanned);
            int v = e.to;
            if (e.id == parentEdge[u]) continue; // Important for parallel edges.

            if (tin[v] == -1) { // DFS tree edge.
                parent[v] = u;
                parentEdge[v] = e.id;
                tin[v] = low[v] = timer++;
                BENCH_INC(vertices_visited);
                edgeStack.push_back(e.id);
                BENCH_INC(edge_stack_pushes);
                dfs.push_back(v);
                BENCH_MAX(max_stack_depth, dfs.size());
            } else if (tin[v] < tin[u]) { // Back edge to ancestor.
                low[u] = min(low[u], tin[v]);
                edgeStack.push_back(e.id);
                BENCH_INC(edge_stack_pushes);
            }
        }
    }

    //Articulation points, deletion counts, bridges.
    res.components_after_removal.resize(n);
    for (int v = 0; v < n; ++v) {
        int pieces = separated[v] + (parent[v] != -1);
        if (pieces >= 2) res.articulation_points.push_back(v);
        res.components_after_removal[v] = components - 1 + pieces;
    }
    for (int id = 0; id < m; ++id) {
        if (bridge[id]) {
            res.bridge_ids.push_back(id);
            res.bridges.push_back(g.edges[id]);
        }
    }

    //BCCs, numbered by smallest edge ID for deterministic output.
    vector<int> renamed(bccCount, -1);
    int count = 0;
    for (int id = 0; id < m; ++id) {
        int b = bccId[id];
        if (b != -1 && renamed[b] == -1) renamed[b] = count++;
    }

    res.bcc_edge_ids.resize(count);
    res.bcc_vertices.resize(count);
    for (int id = 0; id < m; ++id) {
        if (bccId[id] != -1)
            res.bcc_edge_ids[renamed[bccId[id]]].push_back(id);
    }

    vector<int> lastSeen(count, -1);
    for (int v = 0; v < n; ++v) {
        for (const AdjEdge& e : g.adj[v]) {
            if (bccId[e.id] == -1) continue;
            int b = renamed[bccId[e.id]];
            if (lastSeen[b] != v) {
                lastSeen[b] = v;
                res.bcc_vertices[b].push_back(v);
            }
        }
    }
    return res;
}

class TarjanSolver {
public:
    // Computes articulation points, bridges, and biconnected components
    // using an explicit heap-allocated stack (iterative DFS) in O(V + E) time.
    static StructAResult solve(const Graph& g) { return runStructA(g); }
};

#endif // TARJAN_LOWLINK_HPP
