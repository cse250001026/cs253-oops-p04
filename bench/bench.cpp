#include "../include/graph.hpp"
#include "../src/structA/tarjan_lowlink.hpp"
#include "../src/structB/union_find_bridges.hpp"
#include "../oracle/bfs_oracle.hpp"
#include "counters.hpp"

namespace {

[[noreturn]] void die(const string& msg) {
    cerr << "bench: " << msg << '\n';
    exit(2);
}

Graph random_graph(int V, long long E, unsigned long long seed) {
    if (V <= 0 && E > 0) die("--random needs V > 0 when E > 0");
    mt19937_64 rng(seed);
    Graph g(V);
    g.edges.reserve(E);
    for (long long i = 0; i < E; ++i) {
        int u = (int)(rng() % (unsigned long long)V);
        int v = (int)(rng() % (unsigned long long)V);
        g.add_edge(u, v);
    }
    return g;
}

Graph read_graph(const string& path) {
    ifstream in(path);
    if (!in) die("cannot open " + path);
    long long n, m;
    if (!(in >> n >> m) || n < 0 || m < 0) die(path + ": bad header, expected \"n m\"");
    Graph g((int)n);
    g.edges.reserve(m);
    long long u, v, read = 0;
    while (in >> u >> v) {
        if (u < 0 || v < 0 || u >= n || v >= n) die(path + ": edge out of range");
        g.add_edge((int)u, (int)v);
        ++read;
    }
    if (read != m) die(path + ": header says " + to_string(m) + " edges, read " + to_string(read));
    return g;
}

struct GraphFacts {
    long long self_loops = 0;
    int components = 0;
};

GraphFacts graph_facts(const Graph& g) {
    GraphFacts f;
    vector<int> rep(g.V);
    iota(rep.begin(), rep.end(), 0);
    auto find = [&](int x) {
        while (rep[x] != x) x = rep[x] = rep[rep[x]];
        return x;
    };
    f.components = g.V;
    for (const Edge& e : g.edges) {
        if (e.u == e.v) { ++f.self_loops; continue; }
        int a = find(e.u), b = find(e.v);
        if (a != b) { rep[a] = b; --f.components; }
    }
    return f;
}

template <class F>
double time_ms(F&& f) {
    auto t0 = chrono::steady_clock::now();
    f();
    auto t1 = chrono::steady_clock::now();
    return chrono::duration<double, milli>(t1 - t0).count();
}

volatile size_t g_sink = 0;

}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);

    string mode, input_path, experiment = "adhoc", name, structures = "A,B";
    int V = 0, reps = 5;
    long long E = 0, oracle_max_v = 5000;
    double oracle_max_work = 4e9;
    unsigned long long seed = 0;
    bool header = false;

    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        auto next = [&]() -> string {
            if (i + 1 >= argc) die("missing value after " + a);
            return argv[++i];
        };
        if (a == "--random") {
            mode = "random";
            V = stoi(next());
            E = stoll(next());
            seed = stoull(next());
        } else if (a == "--input") {
            mode = "input";
            input_path = next();
        } else if (a == "--structures") structures = next();
        else if (a == "--reps") reps = stoi(next());
        else if (a == "--oracle-max-v") oracle_max_v = stoll(next());
        else if (a == "--oracle-max-work") oracle_max_work = stod(next());
        else if (a == "--experiment") experiment = next();
        else if (a == "--name") name = next();
        else if (a == "--header") header = true;
        else die("unknown option " + a);
    }
    if (mode.empty()) die("give --random V E SEED or --input FILE");
    if (reps < 1) die("--reps must be at least 1");

    Graph g = (mode == "random") ? random_graph(V, E, seed) : read_graph(input_path);
    if (name.empty()) name = (mode == "random") ? "random" : input_path.substr(input_path.find_last_of('/') + 1);
    GraphFacts facts = graph_facts(g);
    long long Ep = g.E - facts.self_loops;

    bool runA = false, runB = false, runO = false;
    {
        stringstream ss(structures);
        string s;
        while (getline(ss, s, ',')) {
            if (s == "A") runA = true;
            else if (s == "B") runB = true;
            else if (s == "oracle") runO = true;
            else if (s != "none") die("unknown structure " + s);
        }
    }
    double oracle_work = 2.0 * (double)Ep * ((double)g.V + (double)Ep);
    if (runO && (g.V > oracle_max_v || oracle_work > oracle_max_work)) {
        cerr << "bench: skipping oracle on " << name << " (V = " << g.V
             << ", predicted " << oracle_work << " half-edge reads)\n";
        runO = false;
    }

    StructAResult refA = TarjanSolver::solve(g);
    StructBResult warmB;
    OracleResult warmO;
    string agreeB = "NA", agreeO = "NA";
    if (runB) {
        warmB = UnionFindBridgeSolver::solve(g);
        agreeB = (warmB.bridge_ids == refA.bridge_ids) ? "1" : "0";
    }
    if (runO) {
        warmO = BfsOracle::solve(g);
        bool ok = warmO.articulation_points == refA.articulation_points &&
                  warmO.bridge_ids == refA.bridge_ids &&
                  warmO.components_after_removal == refA.components_after_removal;
        agreeO = ok ? "1" : "0";
    }
    long long bridges = (long long)refA.bridge_ids.size();
    if (agreeB == "0") cerr << "bench: Structure B disagrees with A on " << name << '\n';
    if (agreeO == "0") cerr << "bench: the oracle disagrees with A on " << name << '\n';

    if (header) {
        cout << "experiment,input,structure,V,E,self_loops,components,bridges,seed,rep,time_ms,agree,"
                "half_edges_scanned,vertices_visited,max_stack_depth,edge_stack_pushes,"
                "uf_finds,uf_find_steps,uf_unions,bfs_runs\n";
    }
    string seed_col = (mode == "random") ? to_string(seed) : "NA";
    cout << setprecision(6) << fixed;

    auto emit = [&](const string& structure, int rep, double ms, const string& agree) {
        cout << experiment << ',' << name << ',' << structure << ',' << g.V << ',' << g.E << ','
             << facts.self_loops << ',' << facts.components << ',' << bridges << ',' << seed_col << ','
             << rep << ',' << ms << ',' << agree << ',' << g_ops.half_edges_scanned << ','
             << g_ops.vertices_visited << ',' << g_ops.max_stack_depth << ',' << g_ops.edge_stack_pushes
             << ',' << g_ops.uf_finds << ',' << g_ops.uf_find_steps << ',' << g_ops.uf_unions << ','
             << g_ops.bfs_runs << '\n';
    };

    for (int r = 1; r <= reps; ++r) {
        if (runA) {
            g_ops.reset();
            double ms = time_ms([&] { g_sink = g_sink + TarjanSolver::solve(g).bridge_ids.size(); });
            emit("A", r, ms, "ref");
        }
        if (runB) {
            g_ops.reset();
            double ms = time_ms([&] { g_sink = g_sink + UnionFindBridgeSolver::solve(g).bridge_ids.size(); });
            emit("B", r, ms, agreeB);
        }
        if (runO) {
            g_ops.reset();
            double ms = time_ms([&] { g_sink = g_sink + BfsOracle::solve(g).bridge_ids.size(); });
            emit("oracle", r, ms, agreeO);
        }
    }
    return (agreeB == "0" || agreeO == "0") ? 1 : 0;
}
