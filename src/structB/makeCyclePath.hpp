#pragma once

#include <vector>
#include "DSU.hpp"
using namespace std;


void markCyclePath(
    int v,
    int u,
    const vector<int>& parent,
    const vector<int>& depth,
    const vector<int>& parentEdge,
    DSU& dsu,
    vector<bool>& marked
);
