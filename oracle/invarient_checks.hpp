#include "../app/block_cut_tree.hpp"
#pragma once

#include "../include/graph.hpp"
#include <vector>
using namespace std;

inline bool check_graph_invariants(const Graph& g) {
    if ((int)g.adj.size() != g.V || (int)g.edges.size() != g.E)
        return false;

    vector<int> from_u(g.E, 0), from_v(g.E, 0);

    for (int u = 0; u < g.V; u++) {
        for (const auto& a : g.adj[u]) {
            if (a.id < 0 || a.id >= g.E || a.to < 0 || a.to >= g.V)
                return false;

            const auto& e = g.edges[a.id];
            if (e.id != a.id || e.u == e.v)
                return false;

            if (e.u == u && e.v == a.to)
                from_u[a.id]++;
            else if (e.v == u && e.u == a.to)
                from_v[a.id]++;
            else
                return false;
        }
    }

    for (int i = 0; i < g.E; i++) {
        const auto& e = g.edges[i];

        if (e.id != i || e.u < 0 || e.u >= g.V ||
            e.v < 0 || e.v >= g.V)
            return false;

        if (e.u != e.v && (from_u[i] != 1 || from_v[i] != 1))
            return false;
    }

    return true;
}

inline bool check_bct_invariants(const Graph& g, const StructAResult& a_res, const BlockCutTree& bct) {
    // 1. Bipartite invariant: C-nodes are [0 .. num_cuts-1], B-nodes are [num_cuts .. total_nodes-1]
    for (int u = 0; u < bct.total_nodes; u++) {
        bool u_is_cut = (u < bct.num_cuts);
        for (int w : bct.adj[u]) {
            if (w < 0 || w >= bct.total_nodes) return false;
            bool w_is_cut = (w < bct.num_cuts);
            if (u_is_cut == w_is_cut) return false; // Must connect C-node and B-node
        }
    }

    // 2. Acyclic forest invariant: in each BCT component, edges == nodes - 1
    int num_comps = 0;
    for (int u = 0; u < bct.total_nodes; u++) {
        if (bct.comp[u] + 1 > num_comps) num_comps = bct.comp[u] + 1;
    }
    vector<int> nodes_in_comp(num_comps, 0);
    vector<long long> deg_in_comp(num_comps, 0);
    for (int u = 0; u < bct.total_nodes; u++) {
        int c = bct.comp[u];
        if (c < 0) return false;
        nodes_in_comp[c]++;
        deg_in_comp[c] += bct.adj[u].size();
    }
    for (int c = 0; c < num_comps; c++) {
        if (deg_in_comp[c] % 2 != 0) return false;
        long long edges_in_comp = deg_in_comp[c] / 2;
        if (edges_in_comp != nodes_in_comp[c] - 1) return false;
    }

    // 3. Separation equivalence on small graphs (V <= 15) against BFS in G \ {v}
    if (g.V <= 15) {
        for (int v = 0; v < g.V; v++) {
            // Compute BFS reachability labels in G \ {v}
            vector<int> vis(g.V, -1);
            vector<int> q(g.V);
            int cid = 0;
            for (int s = 0; s < g.V; s++) {
                if (s == v || vis[s] != -1) continue;
                cid++;
                int head = 0, tail = 0;
                vis[s] = cid;
                q[tail++] = s;
                while (head < tail) {
                    int u = q[head++];
                    for (const auto& e : g.adj[u]) {
                        if (e.to == v || vis[e.to] != -1) continue;
                        vis[e.to] = cid;
                        q[tail++] = e.to;
                    }
                }
            }
            // Compare initial reachability (without deleting v)
            vector<int> orig_vis(g.V, -1);
            int ocid = 0;
            for (int s = 0; s < g.V; s++) {
                if (orig_vis[s] != -1) continue;
                ocid++;
                int head = 0, tail = 0;
                orig_vis[s] = ocid;
                q[tail++] = s;
                while (head < tail) {
                    int u = q[head++];
                    for (const auto& e : g.adj[u]) {
                        if (orig_vis[e.to] != -1) continue;
                        orig_vis[e.to] = ocid;
                        q[tail++] = e.to;
                    }
                }
            }
            for (int x = 0; x < g.V; x++) {
                for (int y = x + 1; y < g.V; y++) {
                    if (x == v || y == v) continue;
                    bool bfs_sep = (orig_vis[x] == orig_vis[y]) && (vis[x] != vis[y]);
                    if (bct.separates(v, x, y) != bfs_sep) return false;
                }
            }
        }
    }
    return true;
}