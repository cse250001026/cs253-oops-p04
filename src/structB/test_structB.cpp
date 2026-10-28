#include <iostream>
#include <vector>
#include "../../include/graph.hpp"
#include "union_find_bridges.hpp"

using namespace std;

#if defined(STRUCTB_STANDALONE) || defined(STRUCTB_MAIN)
int main() {
    int n = 10;
    Graph g(n);

    // Sample graph with parallel edges, bridges, and cycles
    vector<pair<int, int>> edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5},
        {5, 6}, {6, 2}, {5, 2}, {6, 7}, {6, 7}, {8, 9}
    };

    for (const auto& e : edges) {
        g.add_edge(e.first, e.second);
    }

    StructBResult res = UnionFindBridgeSolver::solve(g);

    cout << "=== Bridge Edge IDs ===\n";
    for (int id : res.bridge_ids) {
        cout << "Edge ID: " << id << '\n';
    }

    cout << "\n=== Bridge Edge Structs ===\n";
    for (const auto& bridge : res.bridges) {
        cout << "Bridge: (" << bridge.u << ", " << bridge.v << ") [ID: " << bridge.id << "]\n";
    }

    cout << "\n=== 2-Edge-Connected Components (2-ECCs) ===\n";
    cout << "Total Components: " << res.two_eccs.size() << '\n';
    for (size_t i = 0; i < res.two_eccs.size(); i++) {
        cout << "Component " << i << " (size " << res.two_eccs[i].size() << "): { ";
        for (int v : res.two_eccs[i]) {
            cout << v << " ";
        }
        cout << "}\n";
    }

    return 0;
}
#endif