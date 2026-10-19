#include "DFSForest.h"
#include <stack>

DFSForest buildDFSForest(int n, const std::vector<std::pair<int, int>>& edges) {
    std::vector<std::vector<AdjEdge>> adj(n);
    int m = edges.size();

    // Build adjacency list with unique edge IDs
    for (int id = 0; id < m; id++) {
        int u = edges[id].first;
        int v = edges[id].second;
        adj[u].push_back({v, id});
        adj[v].push_back({u, id});
    }

    DFSForest res;
    res.parent.assign(n, -1);
    res.depth.assign(n, 0);
    res.parentEdge.assign(n, -1);
    res.visited.assign(n, false);
    res.isTreeEdge.assign(m, false);

    // Each stack entry stores (vertex, next adjacency index).
    // This simulates recursive DFS.
    std::stack<std::pair<int, int>> st;

    for (int root = 0; root < n; root++) {
        if (res.visited[root]) continue;

        st.push({root, 0});
        res.visited[root] = true;
        res.depth[root] = 0;
        res.parent[root] = -1;

        while (!st.empty()) {
            int v = st.top().first;
            int &i = st.top().second;

            // All neighbors of v have been processed
            if (i == (int)adj[v].size()) {
                st.pop();
                continue;
            }

            AdjEdge e = adj[v][i++];
            int u = e.to;
            int id = e.id;

            if (!res.visited[u]) {
                // This edge discovers a new vertex
                res.visited[u] = true;
                res.parent[u] = v;
                res.depth[u] = res.depth[v] + 1;
                res.parentEdge[u] = id;

                res.isTreeEdge[id] = true;
                res.treeEdgeIds.push_back(id);

                st.push({u, 0});
            }
        }
    }

    return res;
}
