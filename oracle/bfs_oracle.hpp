#ifndef BFS_ORACLE_HPP
#define BFS_ORACLE_HPP

#include "../include/graph.hpp"

class BfsOracle {
private :
    /* Counts connected components after optionally excluding a vertex
    and an edge. -1 means no deletion. */
    static int count_comp(const Graph &g, int delNode, int delEdge, 
                          vector<bool> &vis, vector<int> &que) {
        int V = g.V;

        fill(vis.begin(), vis.end(), false);

        int count = 0;

        for (int s = 0; s < V; s++) {
            if (vis[s] || s == delNode) continue;

            count++;
            vis[s] = true;
            
            // Use an array-backed queue to avoid reallocating a queue
            // for every connected component and deletion scenario.
            int head = 0, tail = 0;
            que[tail++] = s;
            
            while (head < tail) {
                int u = que[head++];

                for (const AdjEdge& e : g.adj[u]) {
                    if (e.id == delEdge || e.to == delNode || vis[e.to]) continue;

                    vis[e.to] = true;
                    que[tail++] = e.to;
                }
            }
        }
        return count;
    }

public : 
    /*Brute-force oracle for articulation points and bridges.
    Time: O((V + E)^2)*/
    static OracleResult solve(const Graph &g) {
        OracleResult res;

        int V = g.V;
        vector<bool> vis(V, false);
        vector<int> que(V);

        res.initial_components = count_comp(g, -1, -1, vis, que);
        int C0 = res.initial_components;

        res.components_after_removal.assign(V, 0);
        for (int v = 0; v < V; v++) {
            int c = count_comp(g, v, -1, vis, que);
            res.components_after_removal[v] = c;

            // A vertex is articulate point if deleting it increases the connected comp.
            if (c > C0) res.articulation_points.push_back(v); // sorted in asc order
        }

        for (const Edge& e : g.edges) {
            if (e.u == e.v) continue; // Self loop cannot be bridges

            // A edge is a bridge if deleting it increases the connected comp.
            // edge IDs are used to distinguish parallel edges.
            if (count_comp(g, -1, e.id, vis, que) > C0) {
                res.bridge_ids.push_back(e.id);
                res.bridges.push_back(e);
            }
        }
        return res;
    }

};

#endif