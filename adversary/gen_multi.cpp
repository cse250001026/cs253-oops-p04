#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
    int n = 1000;
    unsigned seed = 20260928;
    if (argc > 1) n = stoi(argv[1]);
    if (argc > 2) seed = stoul(argv[2]);
    if (n < 3) {
        cerr << "need n >= 3\n";
        return 1;
    }

    mt19937 rng(seed);
    vector<int> label(n);
    iota(label.begin(), label.end(), 0);
    shuffle(label.begin(), label.end(), rng);

    vector<pair<int, int>> edges;
    int single_edges = 0;
    for (int i = 0; i + 1 < n; i++) {
        edges.push_back({label[i], label[i + 1]});
        if (rng() % 2) {
            edges.push_back({label[i], label[i + 1]});
        } else {
            single_edges++;
        }
    }
    for (int i = 0; i < n; i++) {
        if (rng() % 10 == 0) edges.push_back({label[i], label[i]});
    }
    shuffle(edges.begin(), edges.end(), rng);

    cerr << "gen_multi n=" << n << " seed=" << seed
         << " expected_bridges=" << single_edges
         << " expected_articulation_points=" << n - 2 << "\n";
    cout << n << " " << edges.size() << "\n";
    for (auto& e : edges) cout << e.first << " " << e.second << "\n";
    return 0;
}
