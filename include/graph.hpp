#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <bits/stdc++.h>
using namespace std;

// 1. Half-edge stored inside a vertex's adjacency list adj[u]
class AdjEdge {
public:
    int to;   // Neighbor vertex ID in [0, V-1]
    int id;   // Unique undirected edge ID in [0, E-1]

    AdjEdge(int to_v = 0, int edge_id = 0) : to(to_v), id(edge_id) {}
};

// 2. Full undirected edge stored in edges[id]
class Edge {
public:
    int u;    // Canonical smaller endpoint: min(u, v)
    int v;    // Canonical larger endpoint: max(u, v)
    int id;   // Unique edge ID in [0, E-1]

    Edge(int u_v = 0, int v_v = 0, int edge_id = 0) : u(u_v), v(v_v), id(edge_id) {}

    bool operator<(const Edge& other) const {
        if (u != other.u) return u < other.u;
        if (v != other.v) return v < other.v;
        return id < other.id;
    }

    bool operator==(const Edge& other) const {
        return u == other.u && v == other.v && id == other.id;
    }
};

// 3. Common Undirected Graph Class (Scale: V <= 100,000, E <= 1,000,000)
class Graph {
public:
    int V;
    int E;
    vector<vector<AdjEdge>> adj;
    vector<Edge> edges;

    explicit Graph(int vertices = 0) : V(vertices), E(0), adj(vertices) {}

    // Adds an undirected edge between u and v and assigns sequential ID [0 .. E-1]
    int add_edge(int u, int v) {
        int id = E++;
        int cu = min(u, v);
        int cv = max(u, v);
        edges.push_back(Edge(cu, cv, id));

        // Self-loops (u == v) are stored in edges[] to preserve contiguous IDs,
        // but skipped in adj[][] since they never affect connectivity or cuts.
        if (u != v) {
            adj[u].push_back(AdjEdge(v, id));
            adj[v].push_back(AdjEdge(u, id));
        }
        return id;
    }
};

// 4. Output Class for Structure A (D1: Chetna - Iterative Tarjan Low-Link DFS)
class StructAResult {
public:
    vector<int> articulation_points;       // Sorted ascending vertex IDs
    vector<int> bridge_ids;                // Sorted ascending edge IDs
    vector<Edge> bridges;                  // Sorted canonical Edge objects
    vector<vector<int>> bcc_edge_ids;      // Each BCC as a list of edge IDs
    vector<vector<int>> bcc_vertices;      // Each BCC as sorted unique vertex IDs
    vector<int> components_after_removal;  // Size V: total graph components if v is deleted
};

// 5. Output Class for Structure B (D2: Mayank - Union-Find Path Marking)
class StructBResult {
public:
    vector<int> bridge_ids;                // Sorted ascending edge IDs
    vector<Edge> bridges;                  // Sorted canonical Edge objects
    vector<vector<int>> two_eccs;          // 2-edge-connected components (sorted vertex IDs)
};

// 6. Output Class for Brute-Force BFS Oracle (D4: Shubham)
class OracleResult {
public:
    int initial_components = 0;            // C_0: connected components in original graph
    vector<int> articulation_points;       // Sorted ascending vertex IDs
    vector<int> bridge_ids;                // Sorted ascending edge IDs
    vector<Edge> bridges;                  // Sorted canonical Edge objects
    vector<int> components_after_removal;  // Size V: total graph components if v is deleted
};

// 7. Robustness Ranking Entry Class for Application (D3: Sufiyan)
class NodeRobustness {
public:
    int vertex;
    int split_pieces;                      // Pieces v's own component splits into when v fails
    int total_components_after;            // Total components across entire graph when v fails

    NodeRobustness(int v = 0, int pieces = 0, int total = 0)
        : vertex(v), split_pieces(pieces), total_components_after(total) {}

    // Sort descending by total_components_after, tie-break by smaller vertex ID
    bool operator<(const NodeRobustness& other) const {
        if (total_components_after != other.total_components_after)
            return total_components_after > other.total_components_after;
        return vertex < other.vertex;
    }
};

#endif // GRAPH_HPP