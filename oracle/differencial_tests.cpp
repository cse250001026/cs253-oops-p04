#include "../src/structA/tarjan_lowlink.hpp"
#include "../src/structB/union_find_bridges.hpp"
#include "bfs_oracle.hpp"

bool compare(const Graph& g) {
    StructAResult a = runStructA(g);
    StructBResult b = UnionFindBridgeSolver::solve(g);
    OracleResult o = BfsOracle::solve(g);

    if (a.articulation_points != o.articulation_points)
        return false;

    if (a.bridge_ids != b.bridge_ids ||
        a.bridge_ids != o.bridge_ids)
        return false;

    if (a.components_after_removal != o.components_after_removal)
        return false;

    return true;
}
