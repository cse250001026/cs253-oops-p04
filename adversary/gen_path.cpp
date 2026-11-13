#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
    int n = 100000;
    unsigned seed = 20260928;
    if (argc > 1) n = stoi(argv[1]);
    if (argc > 2) seed = stoul(argv[2]);

    mt19937 rng(seed);
    vector<int> label(n);
    iota(label.begin(), label.end(), 0);
    shuffle(label.begin(), label.end(), rng);

    vector<pair<int, int>> edges;
    for (int i = 0; i + 1 < n; i++) {
        edges.push_back({label[i], label[i + 1]});
    }
    shuffle(edges.begin(), edges.end(), rng);
    for (auto& e : edges) {
        if (rng() % 2) swap(e.first, e.second);
    }

    cerr << "gen_path n=" << n << " seed=" << seed << "\n";
    cout << n << " " << edges.size() << "\n";
    for (auto& e : edges) cout << e.first << " " << e.second << "\n";
    return 0;
}
