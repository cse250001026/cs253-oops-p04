#include "makeCyclePath.hpp"
#include <utility>
using namespace std;

void markCyclePath(
    int v,
    int u,
    const vector<int>& parent,
    const vector<int>& depth,
    const vector<int>& parentEdge,
    DSU& dsu,
    vector<bool>& marked
) {
    if (v == u) return;

    // In an undirected DFS, every non-tree edge
    // connects a vertex to one of its ancestors.
    // Make v the deeper endpoint.
    if (depth[v] < depth[u]) {
        swap(v, u);
    }

    // Process tree edges from v upward until u.
    while (true) {
        int x = dsu.find(v);

        if (depth[x] <= depth[u]) break;

        // The edge from x to its parent lies on the cycle.
        marked[parentEdge[x]] = true;

        // Skip this vertex in future path traversals.
        dsu.rep[x] = dsu.find(parent[x]);
        v = dsu.rep[x];
    }
}
