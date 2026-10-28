#include "DSU.hpp"
#include <numeric>
using namespace std;

DSU::DSU(int n) {
    rep.resize(n);
    iota(rep.begin(), rep.end(), 0);
    rank.assign(n, 0);
}

int DSU::find(int v) {
    // Step 1: Find the root
    int root = v;
    while (rep[root] != root) {
        root = rep[root];
    }

    // Step 2: Compress the path
    while (rep[v] != v) {
        int next = rep[v];
        rep[v] = root;
        v = next;
    }

    return root;
}

void DSU::unite(int u, int v) {
    int root_u = find(u);
    int root_v = find(v);
    if (root_u != root_v) {
        if (rank[root_u] < rank[root_v]) {
            rep[root_u] = root_v;
        } else if (rank[root_u] > rank[root_v]) {
            rep[root_v] = root_u;
        } else {
            rep[root_u] = root_v;
            rank[root_v]++;
        }
    }
}
