#pragma once

#include <vector>

struct DSU {
    std::vector<int> rep;

    explicit DSU(int n);
    int find(int v);
};
