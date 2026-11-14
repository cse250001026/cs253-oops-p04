#include "../include/graph.hpp"
#include "../src/structA/tarjan_lowlink.hpp"
#include "../src/structB/union_find_bridges.hpp"
#include "block_cut_tree.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string infile;
    int top_k = 10;

    for (int i = 1; i < argc; i++) {
        string a = argv[i];
        if (a == "--input" && i + 1 < argc) infile = argv[++i];
        else if (a == "--top" && i + 1 < argc) top_k = stoi(argv[++i]);
    }

    istream* in = &cin;
    ifstream fin;
    if (!infile.empty()) {
        fin.open(infile);
        if (!fin) {
            cerr << "app: cannot open " << infile << "\n";
            return 1;
        }
        in = &fin;
    }

    int n, m;
    if (!(*in >> n >> m)) {
        cerr << "app: expected 'V E' header\n";
        return 1;
    }

    Graph g(n);
    for (int i = 0; i < m; i++) {
        int u, v;
        *in >> u >> v;
        g.add_edge(u, v);
    }

    StructAResult resA = TarjanSolver::solve(g);
    StructBResult resB = UnionFindBridgeSolver::solve(g);

    BlockCutTree bct;
    bct.build(g, resA);

    cout << "Network Summary\n";
    cout << "  Vertices:              " << g.V << "\n";
    cout << "  Edges:                 " << g.E << "\n";
    cout << "  Connected components:  " << bct.initial_comps << "\n";
    cout << "  Articulation points:   " << resA.articulation_points.size() << "\n";
    cout << "  Bridges (A / B):       " << resA.bridge_ids.size() << " / " << resB.bridge_ids.size()
         << (resA.bridge_ids == resB.bridge_ids ? " [OK]\n" : " [MISMATCH]\n");
    cout << "  Biconnected blocks:    " << resA.bcc_vertices.size() << "\n";
    cout << "  Block-cut tree nodes:  " << bct.total_nodes
         << " (" << bct.num_cuts << " cut nodes, " << bct.num_blocks << " block nodes)\n\n";

    vector<NodeRobustness> rank_list = bct.get_robustness_ranking();
    int show = min((int)rank_list.size(), top_k);
    cout << "Top " << show << " Critical Nodes (by components after removal):\n";
    cout << "  Rank  Vertex  Component_Pieces  Total_Components\n";
    for (int i = 0; i < show; i++) {
        cout << "  " << (i + 1) << "\t" << rank_list[i].vertex << "\t"
             << rank_list[i].split_pieces << "\t\t  "
             << rank_list[i].total_components_after << "\n";
    }

    // Process optional separation queries at the end of the file/stream:
    // Format: separates v x y
    string op;
    bool header_printed = false;
    while (*in >> op) {
        if (op == "separates" || op == "SEP" || op == "q") {
            int v, x, y;
            if (*in >> v >> x >> y) {
                if (!header_printed) {
                    cout << "\nSeparation Queries:\n";
                    header_printed = true;
                }
                cout << "  separates(" << v << ", " << x << ", " << y << ") -> "
                     << (bct.separates(v, x, y) ? "YES" : "NO") << "\n";
            }
        }
    }

    return 0;
}