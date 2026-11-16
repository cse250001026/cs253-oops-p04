#include <iostream>
#include <string>
#include "../include/graph.hpp"
#include "../src/structA/tarjan_lowlink.hpp"
#include "../src/structB/union_find_bridges.hpp"
#include "../oracle/bfs_oracle.hpp"
using namespace std;

int main(int argc, char* argv[]) {
    bool useOracle = argc > 1 && string(argv[1]) == "oracle";

    int n, m;
    if (!(cin >> n >> m)) {
        cerr << "could not read the header line\n";
        return 2;
    }
    Graph g(n);
    for (int i = 0; i < m; i++) {
        int u, v;
        if (!(cin >> u >> v)) {
            cerr << "header says " << m << " edges, read " << i << "\n";
            return 2;
        }
        if (u < 0 || u >= n || v < 0 || v >= n) {
            cerr << "edge " << i << " out of range\n";
            return 2;
        }
        g.add_edge(u, v);
    }

    StructAResult a = TarjanSolver::solve(g);
    StructBResult b = UnionFindBridgeSolver::solve(g);

    bool ok = true;
    cout << "V=" << n << " E=" << m
         << " A: articulation=" << a.articulation_points.size()
         << " bridges=" << a.bridge_ids.size()
         << " blocks=" << a.bcc_edge_ids.size()
         << " | B: bridges=" << b.bridge_ids.size()
         << " 2ecc=" << b.two_eccs.size();

    if (a.bridge_ids != b.bridge_ids) {
        cout << " | MISMATCH A vs B bridges";
        ok = false;
    }

    if (useOracle) {
        OracleResult o = BfsOracle::solve(g);
        cout << " | O: articulation=" << o.articulation_points.size()
             << " bridges=" << o.bridge_ids.size();
        if (a.articulation_points != o.articulation_points) {
            cout << " | MISMATCH A vs oracle articulation points";
            ok = false;
        }
        if (a.bridge_ids != o.bridge_ids) {
            cout << " | MISMATCH A vs oracle bridges";
            ok = false;
        }
        if (b.bridge_ids != o.bridge_ids) {
            cout << " | MISMATCH B vs oracle bridges";
            ok = false;
        }
        if (a.components_after_removal != o.components_after_removal) {
            cout << " | MISMATCH A vs oracle components after removal";
            ok = false;
        }
    }

    cout << (ok ? " | OK" : " | FAIL") << "\n";
    return ok ? 0 : 1;
}
