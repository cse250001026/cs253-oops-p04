// graph is given in format of a adjecency list g
#include <vector>
#include <deque>
#include <utility>
using namespace std;

int count_components(vector<vector<int>> &g, int delNode=-1, pair<int, int> delEdge={-1,-1}) {
    int Nodes = g.size();

    vector<bool> vis(Nodes, false);

    deque<int> dq;
    dq.push_back(1); // should we exclude root node too ?

    int connected_comp= 0;

    for (int i = 0;i < Nodes; i++) {
        if (vis[i] || i == delNode) 
            continue;

        connected_comp ++;

        vis[i] = 1;
        deque<int> dq;
        dq.push_back(i);

        while (!dq.empty()) {
            int curNode = dq.front();
            dq.pop_front();


            for (auto child : g[curNode]) {
                if (vis[child] || child == delNode) 
                    continue;
                
                vis[child] = 1;
                dq.push_back(child);
            }

        }
    }

    return connected_comp;
}

