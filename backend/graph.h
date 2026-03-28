#pragma once
#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

/**
 * @file graph.h
 * @brief Graph data model for the city road network.
 * 
 * DATA STRUCTURES:
 * 
 * 1. Node struct — Represents an intersection/landmark
 *    - id: unique integer identifier
 *    - name: human-readable name (e.g., "Tribune Chowk")
 *    - lat, lng: GPS coordinates (decimal degrees)
 * 
 * 2. Edge struct — Represents a road segment
 *    - source, dest: node IDs of connected intersections
 *    - weight: distance in meters (computed via Haversine formula)
 * 
 * 3. Adjacency List — std::unordered_map<int, std::vector<std::pair<int, double>>>
 *    WHY unordered_map?
 *      - O(1) average lookup by node ID (vs O(log n) for std::map)
 *      - Node IDs may not be contiguous, so a raw array would waste space
 *      - Hash-based structure is ideal for sparse graphs like road networks
 *    
 *    WHY vector<pair<int, double>>?
 *      - Each entry stores (neighbor_id, edge_weight)
 *      - Vector gives O(1) amortized insertion and cache-friendly iteration
 *    
 *    Space Complexity: O(V + E) where V = vertices, E = edges
 *    Lookup: O(1) average for finding a node's neighbors
 *    
 *    WHY NOT adjacency matrix?
 *      - Road networks are sparse: a city with 25 nodes has ~40 edges, not 625
 *      - Adjacency matrix would be O(V²) = 625 entries, mostly zeros
 *      - Adjacency list uses only O(V + 2E) ≈ 105 entries (undirected, each edge stored twice)
 */

namespace routeiq {

/**
 * Represents a city intersection or landmark.
 */
struct Node {
    int id;
    std::string name;
    double lat;
    double lng;

    Node() : id(-1), lat(0), lng(0) {}
    Node(int id, const std::string& name, double lat, double lng)
        : id(id), name(name), lat(lat), lng(lng) {}
};

/**
 * Represents a road connecting two intersections.
 * Weight = real-world distance in meters (via Haversine).
 */
struct Edge {
    int source;
    int dest;
    double weight; // distance in meters

    Edge() : source(-1), dest(-1), weight(0) {}
    Edge(int src, int dst, double w) : source(src), dest(dst), weight(w) {}
};

/**
 * Weighted undirected graph representing the city road network.
 * 
 * Key operations:
 *   addNode()  — O(1) average (hash map insertion)
 *   addEdge()  — O(1) average (vector push_back + hash map lookup)
 *   getNeighbors() — O(1) average (hash map lookup, then iterate neighbors)
 *   getNode()  — O(1) average (hash map lookup)
 */
class Graph {
public:
    /**
     * Add a node (intersection) to the graph.
     * @param node The node to add
     */
    void addNode(const Node& node);

    /**
     * Add a bidirectional road between two nodes.
     * Weight is auto-computed using Haversine distance from GPS coordinates.
     * Both directions (src→dst and dst→src) are stored for undirected graph.
     * @param src Source node ID
     * @param dst Destination node ID
     */
    void addEdge(int src, int dst);

    /**
     * Add a bidirectional road with an explicit weight.
     * Useful for testing or when distance is pre-computed.
     * @param src Source node ID
     * @param dst Destination node ID
     * @param weight Edge weight in meters
     */
    void addEdgeWithWeight(int src, int dst, double weight);

    /**
     * Get all neighbors of a node.
     * @param nodeId The node to query
     * @return Vector of (neighbor_id, weight) pairs
     */
    const std::vector<std::pair<int, double>>& getNeighbors(int nodeId) const;

    /**
     * Get a node by its ID.
     * @param nodeId The node ID
     * @return Reference to the Node struct
     */
    const Node& getNode(int nodeId) const;

    /**
     * Check if a node exists in the graph.
     * @param nodeId The node ID to check
     * @return true if the node exists
     */
    bool hasNode(int nodeId) const;

    /**
     * Get all nodes in the graph.
     * @return Reference to the node map
     */
    const std::unordered_map<int, Node>& getAllNodes() const;

    /**
     * Get all edges (each unique edge returned once, not both directions).
     * @return Vector of Edge structs
     */
    std::vector<Edge> getAllEdges() const;

    /**
     * Get the total number of nodes.
     */
    size_t nodeCount() const;

    /**
     * Get the total number of undirected edges.
     */
    size_t edgeCount() const;

private:
    // Node storage: O(1) lookup by ID
    std::unordered_map<int, Node> nodes_;

    /**
     * ADJACENCY LIST — The core graph data structure.
     * 
     * Key: node ID (int)
     * Value: vector of (neighbor_id, edge_weight) pairs
     * 
     * Example for Chandigarh:
     *   adj_list_[0] = [(1, 1200.5), (3, 850.2)]  // Node 0 connects to nodes 1 and 3
     *   adj_list_[1] = [(0, 1200.5), (2, 600.0)]   // Bidirectional: 1→0 mirrors 0→1
     * 
     * Space: O(V + 2E) — each undirected edge stored in both directions
     */
    std::unordered_map<int, std::vector<std::pair<int, double>>> adj_list_;

    // Track unique edges to avoid duplicates in getAllEdges()
    std::unordered_set<long long> edge_set_;

    // Helper: create a unique key for an edge (order-independent)
    static long long edgeKey(int a, int b);
};

} // namespace routeiq

#endif // GRAPH_H
