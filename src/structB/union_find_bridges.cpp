#include "union_find_bridges.hpp"
#include "DFSForest.hpp"
#include "DSU.hpp"
#include "makeCyclePath.hpp"
#include <algorithm>
#include <vector>

using namespace std;

StructBResult UnionFindBridgeSolver::solve(const Graph& g) {
    StructBResult result;

    if (g.V == 0) {
        return result;
    }

    // 1. Build iterative DFS forest
    DFSForest forest = buildDFSForest(g);

    // 2. Union-Find for marking cycles along tree edges
    DSU dsu(g.V);
    vector<bool> marked(g.E, false);

    // 3. Mark cycle paths for all non-tree edges
    for (int id = 0; id < g.E; id++) {
        if (forest.isTreeEdge[id]) continue;

        int u = g.edges[id].u;
        int v = g.edges[id].v;
        if (u == v) continue; // Self-loops do not create cycle paths along tree edges

        markCyclePath(
            u, v,
            forest.parent,
            forest.depth,
            forest.parentEdge,
            dsu,
            marked
        );
    }

    // 4. Collect bridge edge IDs and canonical Edge objects
    // An edge is a bridge iff it is a tree edge and was never marked on any cycle.
    for (int id = 0; id < g.E; id++) {
        if (forest.isTreeEdge[id] && !marked[id]) {
            result.bridge_ids.push_back(id);
            result.bridges.push_back(g.edges[id]);
        }
    }

    // 5. Compute 2-Edge-Connected Components (2-ECCs)
    // Non-bridge edges connect vertices within the same 2-ECC.
    DSU ecc_dsu(g.V);
    for (int id = 0; id < g.E; id++) {
        if (g.edges[id].u != g.edges[id].v && (!forest.isTreeEdge[id] || marked[id])) {
            ecc_dsu.unite(g.edges[id].u, g.edges[id].v);
        }
    }

    // Group vertices by their 2-ECC representative in a single pass.
    // Order of components is determined by the minimum vertex ID in each component.
    vector<int> root_to_comp(g.V, -1);
    for (int v = 0; v < g.V; v++) {
        int r = ecc_dsu.find(v);
        if (root_to_comp[r] == -1) {
            root_to_comp[r] = result.two_eccs.size();
            result.two_eccs.push_back({});
        }
        result.two_eccs[root_to_comp[r]].push_back(v);
    }

    return result;
}
