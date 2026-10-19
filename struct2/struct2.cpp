#include <iostream>
#include <vector>
#include <utility>

#include "DSU.h"
#include "DFSForest.h"
#include "makeCyclePath.h"

using namespace std;

int main() {
    int n = 10;
    vector<pair<int, int>> edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5},
        {5, 6}, {6, 2}, {5, 2}, {6, 7}, {6, 7}, {8, 9}
    }; 

    DFSForest res = buildDFSForest(n, edges);
    DSU dsu(n);

    vector<bool> marked(edges.size(), false);

    for (int id = 0; id < (int)edges.size(); id++) {
        if (res.isTreeEdge[id]) continue;

        int a = edges[id].first;
        int b = edges[id].second;

        // For a non-tree edge in an undirected DFS,
        // one endpoint is an ancestor of the other
        markCyclePath(
            a, b,
            res.parent,
            res.depth,
            res.parentEdge,
            dsu,
            marked
        );
    }

    // A tree edge is a bridge if it was never marked.
    for (int id : res.treeEdgeIds) {
        if (!marked[id]) {
            cout << "Bridge: edge ID " << id << '\n';
        }
    }

    return 0;
}