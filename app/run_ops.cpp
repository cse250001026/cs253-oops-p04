#include "../include/graph.hpp"
#include "../src/structA/tarjan_lowlink.hpp"
#include "../src/structB/union_find_bridges.hpp"
#include "block_cut_tree.hpp"

using namespace std;

static void print_vec(const vector<int>& a) {
    for (size_t i = 0; i < a.size(); i++) {
        if (i) cout << ' ';
        cout << a[i];
    }
    cout << '\n';
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string mode = "A";
    for (int i = 1; i < argc; i++) {
        string a = argv[i];
        if ((a == "--structure" || a == "-s") && i + 1 < argc) {
            mode = argv[++i];
        }
    }

    int n, m;
    if (!(cin >> n >> m)) return 0;

    Graph g(n);
    for (int i = 0; i < m; i++) {
        int u, v;
        cin >> u >> v;
        g.add_edge(u, v);
    }

    vector<string> cmds;
    string tok;
    while (cin >> tok) cmds.push_back(tok);

    if (mode == "B") {
        StructBResult resB = UnionFindBridgeSolver::solve(g);
        if (cmds.empty()) {
            cout << "BRIDGES " << resB.bridge_ids.size() << "\n";
            print_vec(resB.bridge_ids);
            cout << "TWO_ECCS " << resB.two_eccs.size() << "\n";
            for (const auto& c : resB.two_eccs) print_vec(c);
        } else {
            size_t i = 0;
            while (i < cmds.size()) {
                string c = cmds[i++];
                if (c == "BRIDGES" || c == "bridges") {
                    print_vec(resB.bridge_ids);
                } else if (c == "TWO_ECCS" || c == "two_eccs") {
                    cout << resB.two_eccs.size() << "\n";
                    for (const auto& comp : resB.two_eccs) print_vec(comp);
                }
            }
        }
    } else {
        StructAResult resA = TarjanSolver::solve(g);
        BlockCutTree bct;
        bct.build(g, resA);

        if (cmds.empty()) {
            cout << "ARTICULATION_POINTS " << resA.articulation_points.size() << "\n";
            print_vec(resA.articulation_points);
            cout << "BRIDGES " << resA.bridge_ids.size() << "\n";
            print_vec(resA.bridge_ids);
            cout << "BCCS " << resA.bcc_edge_ids.size() << "\n";
            for (const auto& b : resA.bcc_edge_ids) print_vec(b);
        } else {
            size_t i = 0;
            while (i < cmds.size()) {
                string c = cmds[i++];
                if (c == "ARTICULATION_POINTS" || c == "articulation_points" || c == "AP") {
                    print_vec(resA.articulation_points);
                } else if (c == "BRIDGES" || c == "bridges") {
                    print_vec(resA.bridge_ids);
                } else if (c == "BCCS" || c == "bccs") {
                    cout << resA.bcc_edge_ids.size() << "\n";
                    for (const auto& b : resA.bcc_edge_ids) print_vec(b);
                } else if (c == "ROBUSTNESS" || c == "robustness") {
                    print_vec(resA.components_after_removal);
                } else if ((c == "SEPARATES" || c == "separates" || c == "SEP") && i + 2 < cmds.size()) {
                    int v = stoi(cmds[i++]);
                    int x = stoi(cmds[i++]);
                    int y = stoi(cmds[i++]);
                    cout << (bct.separates(v, x, y) ? 1 : 0) << "\n";
                }
            }
        }
    }

    return 0;
}