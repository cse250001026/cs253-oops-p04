#include "makeCyclePath.h"
#include <utility>

void markCyclePath(
    int v,
    int u,
    const std::vector<int>& parent,
    const std::vector<int>& depth,
    const std::vector<int>& parentEdge,
    DSU& dsu,
    std::vector<bool>& marked
) {
    // In an undirected DFS, every non-tree edge
    // connects a vertex to one of its ancestors.
    // Make v the deeper endpoint.
    if (depth[v] < depth[u]) {
        std::swap(v, u);
    }

    // Process tree edges from v upward until u.
    while (true) {
        int x = dsu.find(v);

        if (depth[x] <= depth[u]) break;

        // The edge from x to its parent lies on the cycle.
        marked[parentEdge[x]] = true;

        // Skip this vertex in future path traversals.
        dsu.rep[x] = dsu.find(parent[x]);
    }
}
