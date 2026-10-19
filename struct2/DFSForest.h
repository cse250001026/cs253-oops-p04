#pragma once

#include <vector>
#include <utility>

struct AdjEdge {
    int to;
    int id;
};

struct DFSForest {
    std::vector<int> parent;
    std::vector<int> depth;
    std::vector<int> parentEdge;
    std::vector<bool> visited;
    std::vector<bool> isTreeEdge;
    std::vector<int> treeEdgeIds;
};

DFSForest buildDFSForest(int n, const std::vector<std::pair<int, int>>& edges);
