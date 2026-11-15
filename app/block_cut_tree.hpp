#ifndef BLOCK_CUT_TREE_HPP
#define BLOCK_CUT_TREE_HPP

#include "../include/graph.hpp"

class BlockCutTree {
public:
    int n_orig = 0;
    int num_cuts = 0;      // C-nodes: [0 .. num_cuts - 1]
    int num_blocks = 0;    // B-nodes: [num_cuts .. num_cuts + num_blocks - 1]
    int total_nodes = 0;
    int initial_comps = 0; // C0 in the original graph

    vector<int> cut_id;    // cut_id[v] = C-node index if v is an AP, else -1
    vector<int> rep;       // representative node in BCT for vertex v (-1 if isolated)

    vector<vector<int>> adj;
    vector<vector<int>> tree_children;
    vector<int> parent;
    vector<int> comp;
    vector<int> tin, tout;
    vector<int> rem_comps;

    void build(const Graph& g, const StructAResult& a_res) {
        n_orig = g.V;
        num_cuts = (int)a_res.articulation_points.size();
        num_blocks = (int)a_res.bcc_vertices.size();
        total_nodes = num_cuts + num_blocks;
        rem_comps = a_res.components_after_removal;

        cut_id.assign(n_orig, -1);
        rep.assign(n_orig, -1);

        for (int i = 0; i < num_cuts; i++) {
            int v = a_res.articulation_points[i];
            cut_id[v] = i;
            rep[v] = i;
        }

        adj.assign(total_nodes, {});
        for (int b = 0; b < num_blocks; b++) {
            int b_node = num_cuts + b;
            for (int v : a_res.bcc_vertices[b]) {
                if (cut_id[v] != -1) {
                    int c_node = cut_id[v];
                    adj[c_node].push_back(b_node);
                    adj[b_node].push_back(c_node);
                } 
                else {
                    rep[v] = b_node;
                }
            }
        }

        // Recover initial connected components C0 from vertex 0's removal count
        if (n_orig == 0) {
            initial_comps = 0;
        } 
        else if (cut_id[0] != -1) {
            int deg = (int)adj[cut_id[0]].size();
            initial_comps = rem_comps[0] + 1 - deg;
        } 
        else if (g.adj[0].empty()) {
            initial_comps = rem_comps[0] + 1;
        } 
        else {
            initial_comps = rem_comps[0];
        }

        // Iterative DFS on the Block-Cut Forest to compute tin and tout
        tree_children.assign(total_nodes, {});
        parent.assign(total_nodes, -1);
        comp.assign(total_nodes, -1);
        tin.assign(total_nodes, 0);
        tout.assign(total_nodes, 0);

        vector<int> cur_edge(total_nodes, 0);
        vector<int> st;
        int timer = 0, cid = 0;

        for (int root = 0; root < total_nodes; root++) {
            if (comp[root] != -1) continue;

            comp[root] = cid++;
            tin[root] = ++timer;
            st.push_back(root);

            while (!st.empty()) {
                int u = st.back();
                if (cur_edge[u] == (int)adj[u].size()) {
                    tout[u] = timer;
                    st.pop_back();
                    continue;
                }
                int nxt = adj[u][cur_edge[u]++];
                if (nxt == parent[u]) continue;

                if (comp[nxt] == -1) {
                    parent[nxt] = u;
                    comp[nxt] = comp[u];
                    tree_children[u].push_back(nxt);
                    tin[nxt] = ++timer;
                    st.push_back(nxt);
                }
            }
        }
    }

    // True if u is in the ancestor path of w in the BCT DFS forest
    bool is_sub(int u, int w) const {
        return tin[u] <= tin[w] && tout[w] <= tout[u];
    }

    // Finds which direct child of u contains w in its subtree (using sorted tin/tout)
    int branch_of(int u, int w) const {
        const auto& ch = tree_children[u];
        int l = 0, r = (int)ch.size() - 1;
        while (l <= r) {
            int mid = l + (r - l) / 2;
            int c = ch[mid];
            if (tin[w] < tin[c]) r = mid - 1;
            else if (tin[w] > tout[c]) l = mid + 1;
            else return c;
        }
        return -1;
    }

    bool separates(int v, int x, int y) const {
        if (v < 0 || v >= n_orig || x < 0 || x >= n_orig || y < 0 || y >= n_orig)
            return false;
        if (v == x || v == y || x == y)
            return false;

        int c = cut_id[v];
        if (c == -1) return false; // non-cut vertex cannot separate anything

        int rx = rep[x], ry = rep[y];
        if (rx == -1 || ry == -1) return false;
        if (comp[rx] != comp[ry] || comp[c] != comp[rx]) return false;

        bool x_under = is_sub(c, rx);
        bool y_under = is_sub(c, ry);

        // One is inside c's subtree and the other is outside -> path must cross c
        if (x_under != y_under) return true;

        // Both outside c's subtree -> path never visits c
        if (!x_under && !y_under) return false;

        // Both inside c's subtree -> separated iff they lie in different child branches of c
        return branch_of(c, rx) != branch_of(c, ry);
    }

    vector<NodeRobustness> get_robustness_ranking() const {
        vector<NodeRobustness> res;
        res.reserve(n_orig);
        for (int v = 0; v < n_orig; v++) {
            int after = rem_comps[v];
            int pieces = after - initial_comps + 1;
            res.push_back(NodeRobustness(v, pieces, after));
        }
        sort(res.begin(), res.end());
        return res;
    }
};

#endif // BLOCK_CUT_TREE_HPP