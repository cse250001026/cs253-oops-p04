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

    // Process tree edges from v upward until reaching u.
    // By querying get_highest(v), we jump directly to the shallowest
    // vertex in v's current 2-edge connected component.
    while (true) {
        int x = dsu.get_highest(v);

        if (depth[x] <= depth[u]) break;

        // The edge from x to its parent lies on the cycle.
        if (parentEdge[x] != -1) {
            marked[parentEdge[x]] = true;
        }

        // Skip processed tree edge by uniting x's component with parent[x]'s
        // component using union-by-rank. This maintains the O((V+E) alpha(V)) bound.
        int p = parent[x];
        dsu.unite(x, p, depth);
        v = p;
    }
}
