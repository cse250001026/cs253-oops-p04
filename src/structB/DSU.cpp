#include "DSU.hpp"
#include <numeric>
using namespace std;

DSU::DSU(int n) {
    rep.resize(n);
    iota(rep.begin(), rep.end(), 0);
    rank.assign(n, 0);
    highest.resize(n);
    iota(highest.begin(), highest.end(), 0);
}

int DSU::find(int v) {
    int root = v;
    while (rep[root] != root) {
        root = rep[root];
    }
    while (rep[v] != v) {
        int next = rep[v];
        rep[v] = root;
        v = next;
    }
    return root;
}

void DSU::unite(int u, int v, const vector<int>* depth) {
    int root_u = find(u);
    int root_v = find(v);
    if (root_u == root_v) return;

    int top = -1;
    if (depth) {
        top = ((*depth)[highest[root_u]] < (*depth)[highest[root_v]])
                  ? highest[root_u]
                  : highest[root_v];
    }

    if (rank[root_u] < rank[root_v]) {
        rep[root_u] = root_v;
        if (depth) highest[root_v] = top;
    } else {
        rep[root_v] = root_u;
        if (depth) highest[root_u] = top;
        if (rank[root_u] == rank[root_v]) rank[root_u]++;
    }
}

int DSU::get_highest(int v) {
    return highest[find(v)];
}
