#pragma once

#include <vector>
#include <utility>
#include "../../include/graph.hpp"

using namespace std;

struct DFSForest {
    vector<int> parent;
    vector<int> depth;
    vector<int> parentEdge;
    vector<bool> visited;
    vector<bool> isTreeEdge;
    vector<int> treeEdgeIds;
};

DFSForest buildDFSForest(const Graph& g);
DFSForest buildDFSForest(int n, const vector<pair<int, int>>& edges);
