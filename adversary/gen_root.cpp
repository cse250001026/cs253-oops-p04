#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <string>
using namespace std;

vector<pair<int, int>> edges;

void add_cycle(const vector<int>& vs) {
    int k = vs.size();
    for (int i = 0; i < k; i++) {
        edges.push_back({vs[i], vs[(i + 1) % k]});
    }
}

void add_path(const vector<int>& vs) {
    for (size_t i = 0; i + 1 < vs.size(); i++) {
        edges.push_back({vs[i], vs[i + 1]});
    }
}

void add_clique(const vector<int>& vs) {
    for (size_t a = 0; a < vs.size(); a++) {
        for (size_t b = a + 1; b < vs.size(); b++) {
            edges.push_back({vs[a], vs[b]});
        }
    }
}

int main(int argc, char* argv[]) {
    string mode = "cycle";
    int n = 10;
    unsigned seed = 20260928;
    if (argc > 1) mode = argv[1];
    if (argc > 2) n = stoi(argv[2]);
    if (argc > 3) seed = stoul(argv[3]);
    if (n < 5) {
        cerr << "need n >= 5\n";
        return 1;
    }

    mt19937 rng(seed);
    vector<int> rest(n - 1);
    iota(rest.begin(), rest.end(), 1);
    shuffle(rest.begin(), rest.end(), rng);
    vector<int> all;
    all.push_back(0);
    for (int v : rest) all.push_back(v);

    if (mode == "cycle") {
        add_cycle(all);
    } else if (mode == "path") {
        add_path(all);
    } else if (mode == "clique") {
        add_clique(all);
    } else if (mode == "star") {
        for (int i = 1; i < n; i++) edges.push_back({0, all[i]});
    } else if (mode == "twocycles") {
        int half = (n - 1) / 2;
        vector<int> first = {0};
        vector<int> second = {0};
        for (int i = 1; i <= half; i++) first.push_back(all[i]);
        for (int i = half + 1; i < n; i++) second.push_back(all[i]);
        add_cycle(first);
        add_cycle(second);
    } else if (mode == "comps") {
        for (int g = 0; g + 2 < n; g += 3) {
            add_cycle({all[g], all[g + 1], all[g + 2]});
        }
    } else {
        cerr << "unknown mode " << mode << "\n";
        return 1;
    }

    shuffle(edges.begin(), edges.end(), rng);
    for (auto& e : edges) {
        if (rng() % 2) swap(e.first, e.second);
    }

    cerr << "gen_root mode=" << mode << " n=" << n << " seed=" << seed << "\n";
    cout << n << " " << edges.size() << "\n";
    for (auto& e : edges) cout << e.first << " " << e.second << "\n";
    return 0;
}
