#ifndef BFS_ORACLE_HPP
#define BFS_ORACLE_HPP

#include "../include/graph.hpp"

class BfsOracle {
private :
    static int count_comp(Graph &g, int delNode, int delEdge, 
                          vector<bool> vis, vector<int> que) {
        int V = g.V;

        vis = vector<bool> (vis.size(), false);

        int count = 0;

        for (int s = 0; s < V; s++) {
            if (vis[s] || s == delNode) continue;

            count++;
            vis[s] = true;
            
            int head = 0, tail = 0;
            que[tail++] = s;
            
            while (head < tail) {
                int u = que[head++];

                for (AdjEdge& e : g.adj[u]) {
                    if (e.to == delNode || vis[e.to]) continue;

                    vis[e.to] = true;
                    que[tail++] = e.to;
                }
            }
        }

        return count;
    }

public : 


};

#endif