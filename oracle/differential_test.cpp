#include <bits/stdc++.h>
using namespace std;

#include "../src/structA/tarjan_lowlink.hpp"
#include "../src/structB/union_find_bridges.hpp"
#include "bfs_oracle.hpp"
#include "failure_shrinker.hpp"

string compare(const Graph& g) {
    StructAResult a = runStructA(g);
    StructBResult b = UnionFindBridgeSolver::solve(g);
    OracleResult o = BfsOracle::solve(g);

    // if (g.E > 0) return "bridge_ids"; 

    if (a.articulation_points != o.articulation_points) return "articulation_points";
    if (a.bridge_ids != b.bridge_ids || a.bridge_ids != o.bridge_ids) return "bridge_ids";
    if (a.components_after_removal != o.components_after_removal) return "components_after_removal";

    return "";
}

void print_vec(const vector<int>& a) {
    for (int x : a) cerr << x << ' ';
    cerr << '\n';
}

void show_failure(const Graph& g, const vector<pair<int, int>>& ops,
                  unsigned int seed, int tc, int step, const string& mismatch) {
    StructAResult a = runStructA(g);
    StructBResult b = UnionFindBridgeSolver::solve(g);
    OracleResult o = BfsOracle::solve(g);

    cerr << "DIFFERENTIAL TEST FAILED\n";
    cerr << "Seed: " << seed << ", test: " << tc << ", step: " << step << '\n';
    cerr << "V = " << g.V << '\n';
    cerr << "Mismatch: " << mismatch << '\n';
    cerr << "Edges (in order):\n";

    for (auto [u, v] : ops) cerr << u << ' ' << v << '\n';

    cerr << "Articulation points\nA: ";
    print_vec(a.articulation_points);
    cerr << "Oracle: ";
    print_vec(o.articulation_points);

    cerr << "Bridge IDs\nA: ";
    print_vec(a.bridge_ids);
    cerr << "B: ";
    print_vec(b.bridge_ids);
    cerr << "Oracle: ";
    print_vec(o.bridge_ids);

    cerr << "Components after removal\nA: ";
    print_vec(a.components_after_removal);
    cerr << "Oracle: ";
    print_vec(o.components_after_removal);

    auto small = shrink_failure(g.V, ops, mismatch);

    cerr << "\nShrunk from " << ops.size() << " to "
        << small.size() << " edges\n";

    cerr << "Remaining edges (in order):\n";

    for (auto [u, v] : small)
        cerr << u << ' ' << v << '\n';
}

int main(int argc, char* argv[]) {
    long long target = (argc > 1) ? stoll(argv[1]) : 100000;
    unsigned int seed = (argc > 2) ? stoul(argv[2]) : 20260928;

    if (target < 0) {
        cerr << "Invalid number of operations\n";
        return 1;
    }

    mt19937 rng(seed);
    long long done = 0;
    int tc = 0;

    Graph empty(0);
    string mismatch = compare(empty);

    if (!mismatch.empty()) {
        show_failure(empty, {}, seed, 0, 0, mismatch);
        return 1;
    }

    while (done < target) {
        tc++;

        int V = rng() % 30 + 1;
        int m = rng() % 60 + 1;

        Graph g(V);
        vector<pair<int, int>> ops;

        mismatch = compare(g);

        if (!mismatch.empty()) {
            show_failure(g, ops, seed, tc, 0, mismatch);
            return 1;
        }

        for (int i = 1; i <= m && done < target; i++) {
            int u = rng() % V;
            int v = rng() % V;

            g.add_edge(u, v);
            ops.push_back({u, v});
            done++;

            mismatch = compare(g);

            if (!mismatch.empty()) {
                show_failure(g, ops, seed, tc, i, mismatch);
                return 1;
            }
        }
    }

    cout << "DIFFERENTIAL TEST PASSED: " << done
         << " operations, seed " << seed << '\n';

    return 0;
}
