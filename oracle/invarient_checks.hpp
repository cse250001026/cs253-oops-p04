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
