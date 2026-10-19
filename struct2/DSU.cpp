#include "DSU.h"
#include <numeric>

DSU::DSU(int n) {
    rep.resize(n);
    std::iota(rep.begin(), rep.end(), 0);
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
