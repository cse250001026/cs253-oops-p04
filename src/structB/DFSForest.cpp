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
    res.isTreeEdge.assign(m, false);

    vector<bool> visited(n, false);
    vector<int> st;
    st.reserve(n);
    vector<int> edge_idx(n, 0);

    for (int root = 0; root < n; root++) {
        if (visited[root]) continue;

        visited[root] = true;
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

            if (!visited[u]) {
                visited[u] = true;
                res.parent[u] = v;
                res.depth[u] = res.depth[v] + 1;
                res.parentEdge[u] = id;
                res.isTreeEdge[id] = true;
                st.push_back(u);
            }
        }
    }

    return res;
}
