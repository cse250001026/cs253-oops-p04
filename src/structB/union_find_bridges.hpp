#ifndef UNION_FIND_BRIDGES_HPP
#define UNION_FIND_BRIDGES_HPP

#include "../../include/graph.hpp"

using namespace std;

class UnionFindBridgeSolver {
public:
    // Builds an iterative DFS tree and marks tree edges on cycles using
    // custom Union-Find path compression in O((V + E) alpha(V)) time.
    static StructBResult solve(const Graph& g);
};

#endif // UNION_FIND_BRIDGES_HPP
