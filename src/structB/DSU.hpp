#pragma once

#include <vector>

using namespace std;

struct DSU {
    vector<int> rep;
    vector<int> rank;

    explicit DSU(int n);
    int find(int v);
    void unite(int u, int v);
};
