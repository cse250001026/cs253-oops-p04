#include "failure_shrinker.hpp"

string compare(const Graph& g);

vector<pair<int, int>> shrink_failure(int V, vector<pair<int, int>> ops,
                                     const string& mismatch) {
    int i = 0;

    while (i < (int)ops.size()) {
        Graph g(V);
        
        for (int j = 0; j < (int)ops.size(); j++) {
            if (i == j) continue;
            g.add_edge(ops[j].first, ops[j].second);
        }

        if (compare(g) == mismatch) {
            ops.erase(ops.begin() + i);
            i = 0;
        } else {
            i++;
        }
    }

    return ops;
}
