#pragma once

#include <vector>

using namespace std;

struct DSU {
    vector<int> rep;
    vector<int> rank;
    vector<int> highest; // Vertex with minimum DFS depth in this equivalence class

    explicit DSU(int n);
    int find(int v);
    void unite(int u, int v, const vector<int>* depth = nullptr);
    inline void unite(int u, int v, const vector<int>& depth) { unite(u, v, &depth); }
    int get_highest(int v);
};
