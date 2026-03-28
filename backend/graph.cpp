#include "graph.h"
#include "haversine.h"
#include <algorithm>
#include <stdexcept>

/**
 * @file graph.cpp
 * @brief Implementation of the Graph class for city road network modeling.
 * 
 * The graph is UNDIRECTED — every road allows travel in both directions.
 * Each call to addEdge() inserts TWO entries in the adjacency list:
 *   adj_list_[src].push_back({dst, weight})
 *   adj_list_[dst].push_back({src, weight})
 * 
 * Edge weights are computed automatically using the Haversine formula
 * from the GPS coordinates of the connected nodes.
 */

namespace routeiq {

// Static empty vector returned when a node has no neighbors
static const std::vector<std::pair<int, double>> EMPTY_NEIGHBORS;

void Graph::addNode(const Node& node) {
    nodes_[node.id] = node;
    // Initialize empty adjacency list entry for this node
    if (adj_list_.find(node.id) == adj_list_.end()) {
        adj_list_[node.id] = {};
    }
}

void Graph::addEdge(int src, int dst) {
    // Validate that both nodes exist
    if (nodes_.find(src) == nodes_.end() || nodes_.find(dst) == nodes_.end()) {
        throw std::runtime_error("addEdge: node " + std::to_string(src) + " or " + std::to_string(dst) + " not found");
    }

    // Compute real-world distance using Haversine formula
    const Node& srcNode = nodes_[src];
    const Node& dstNode = nodes_[dst];
    double weight = haversine(srcNode.lat, srcNode.lng, dstNode.lat, dstNode.lng);

    addEdgeWithWeight(src, dst, weight);
}

void Graph::addEdgeWithWeight(int src, int dst, double weight) {
    // Create unique key for this edge (order-independent)
    long long key = edgeKey(src, dst);

    // Avoid duplicate edges
    if (edge_set_.count(key)) return;
    edge_set_.insert(key);

    // BIDIRECTIONAL: add edge in both directions
    // This is how we model undirected roads — a road from A→B is also B→A
    adj_list_[src].push_back({dst, weight});
    adj_list_[dst].push_back({src, weight});
}

const std::vector<std::pair<int, double>>& Graph::getNeighbors(int nodeId) const {
    auto it = adj_list_.find(nodeId);
    if (it == adj_list_.end()) return EMPTY_NEIGHBORS;
    return it->second;
}

const Node& Graph::getNode(int nodeId) const {
    auto it = nodes_.find(nodeId);
    if (it == nodes_.end()) {
        throw std::runtime_error("getNode: node " + std::to_string(nodeId) + " not found");
    }
    return it->second;
}

bool Graph::hasNode(int nodeId) const {
    return nodes_.find(nodeId) != nodes_.end();
}

const std::unordered_map<int, Node>& Graph::getAllNodes() const {
    return nodes_;
}

std::vector<Edge> Graph::getAllEdges() const {
    std::vector<Edge> edges;
    // To avoid duplicates in undirected graph, only return edge where src < dst
    for (const auto& [nodeId, neighbors] : adj_list_) {
        for (const auto& [neighborId, weight] : neighbors) {
            if (nodeId < neighborId) {
                edges.emplace_back(nodeId, neighborId, weight);
            }
        }
    }
    return edges;
}

size_t Graph::nodeCount() const {
    return nodes_.size();
}

size_t Graph::edgeCount() const {
    return edge_set_.size();
}

long long Graph::edgeKey(int a, int b) {
    // Ensure order-independence: edgeKey(3,7) == edgeKey(7,3)
    if (a > b) std::swap(a, b);
    return static_cast<long long>(a) * 100000 + b;
}

} // namespace routeiq
