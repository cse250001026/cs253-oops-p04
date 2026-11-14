#ifndef FAILURE_SHRINKER_HPP
#define FAILURE_SHRINKER_HPP

#include "../include/graph.hpp"

vector<pair<int, int>> shrink_failure(int V, vector<pair<int, int>> ops,
                                     const string& mismatch);

#endif
