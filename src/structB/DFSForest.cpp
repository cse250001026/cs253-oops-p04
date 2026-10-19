#include "DFSForest.hpp"
#include <vector>

using namespace std;

DFSForest buildDFSForest(const Graph& g) {
    int n = g.V;
    int m = g.E;

    DFSForest res;
    res.parent.assign(n, -1);
    res.depth.assign(n, 0);
    res.parentEdge.assign(n, -1);
    res.visited.assign(n, false);
    res.isTreeEdge.assign(m, false);

    vector<int> st;
    st.reserve(n);
    vector<int> edge_idx(n, 0);

    for (int root = 0; root < n; root++) {
        if (res.visited[root]) continue;

        res.visited[root] = true;
        res.depth[root] = 0;
        res.parent[root] = -1;
        res.parentEdge[root] = -1;
        st.push_back(root);

        while (!st.empty()) {
            int v = st.back();

            if (edge_idx[v] == (int)g.adj[v].size()) {
                st.pop_back();
                continue;
            }

            const AdjEdge& e = g.adj[v][edge_idx[v]++];
            int u = e.to;
            int id = e.id;

            if (!res.visited[u]) {
                res.visited[u] = true;
                res.parent[u] = v;
                res.depth[u] = res.depth[v] + 1;
                res.parentEdge[u] = id;

                res.isTreeEdge[id] = true;
                res.treeEdgeIds.push_back(id);

                st.push_back(u);
            }
        }
    }

    return res;
}

DFSForest buildDFSForest(int n, const vector<pair<int, int>>& edges) {
    Graph g(n);
    for (const auto& p : edges) {
        g.add_edge(p.first, p.second);
    }
    return buildDFSForest(g);
}
