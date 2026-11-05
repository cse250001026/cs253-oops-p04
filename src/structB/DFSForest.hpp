#pragma once

#include <vector>
#include "../../include/graph.hpp"

using namespace std;

struct DFSForest {
    vector<int> parent;
    vector<int> depth;
    vector<int> parentEdge;
    vector<bool> isTreeEdge;
};

DFSForest buildDFSForest(const Graph& g);
