#pragma once

#include <vector>
#include "DSU.h"

void markCyclePath(
    int v,
    int u,
    const std::vector<int>& parent,
    const std::vector<int>& depth,
    const std::vector<int>& parentEdge,
    DSU& dsu,
    std::vector<bool>& marked
);
