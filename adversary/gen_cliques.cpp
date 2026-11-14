#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
    int k = 4;
    int s = 700;
    unsigned seed = 20260928;
    if (argc > 1) k = stoi(argv[1]);
    if (argc > 2) s = stoi(argv[2]);
    if (argc > 3) seed = stoul(argv[3]);
    if (k < 2 || s < 2) {
        cerr << "need k >= 2 and s >= 2\n";
        return 1;
    }

    int n = 1 + k * (s - 1);
    mt19937 rng(seed);
    vector<int> label(n);
    iota(label.begin(), label.end(), 0);
    shuffle(label.begin(), label.end(), rng);
    int hub = label[0];

    vector<pair<int, int>> edges;
    for (int c = 0; c < k; c++) {
        vector<int> members;
        members.push_back(hub);
        for (int j = 0; j < s - 1; j++) {
            members.push_back(label[1 + c * (s - 1) + j]);
        }
        for (int a = 0; a < s; a++) {
            for (int b = a + 1; b < s; b++) {
                edges.push_back({members[a], members[b]});
            }
        }
    }
    shuffle(edges.begin(), edges.end(), rng);
    for (auto& e : edges) {
        if (rng() % 2) swap(e.first, e.second);
    }

    cerr << "gen_cliques k=" << k << " s=" << s << " seed=" << seed
         << " hub=" << hub << " edges=" << edges.size() << "\n";
    if (edges.size() > 1000000) cerr << "warning: more than 10^6 edges\n";
    cout << n << " " << edges.size() << "\n";
    for (auto& e : edges) cout << e.first << " " << e.second << "\n";
    return 0;
}
