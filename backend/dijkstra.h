#pragma once
#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"
#include <vector>

/**
 * @file dijkstra.h
 * @brief Dijkstra's shortest path algorithm for weighted graphs.
 * 
 * ALGORITHM OVERVIEW:
 * Dijkstra's is a GREEDY algorithm that finds the shortest path from a source
 * node to all other nodes in a graph with non-negative edge weights.
 * 
 * KEY INSIGHT (Greedy Property):
 *   Once a node is "settled" (popped from the min-heap), its shortest distance
 *   is GUARANTEED to be optimal. This is because all remaining unsettled nodes
 *   have equal or greater distances (min-heap invariant), and all edge weights
 *   are non-negative (no way to find a shorter path through unsettled nodes).
 * 
 * DATA STRUCTURES USED:
 *   1. min-heap (priority_queue) — extracts the closest unsettled node in O(log V)
 *   2. dist[] array — tracks best-known distance to each node, init to infinity
 *   3. prev[] array — stores the predecessor on the shortest path, for reconstruction
 *   4. visited set — prevents reprocessing already-settled nodes
 * 
 * TIME COMPLEXITY: O((V + E) · log V)
 *   - Each vertex is extracted from the heap at most once: O(V · log V)
 *   - Each edge is relaxed at most once, potentially inserting into heap: O(E · log V)
 *   - Total: O((V + E) · log V)
 * 
 * SPACE COMPLEXITY: O(V)
 *   - dist[], prev[], visited: each O(V)
 *   - Heap can contain at most V entries (with lazy deletion)
 */

namespace routeiq {

/**
 * Result of a shortest path computation.
 */
struct PathResult {
    std::vector<int> path;     // Ordered list of node IDs from source to destination
    double distance;           // Total distance in meters
    double time_ms;            // Computation time in milliseconds
    int nodes_visited;         // Number of nodes popped from the priority queue
    bool found;                // Whether a path was found

    PathResult() : distance(0), time_ms(0), nodes_visited(0), found(false) {}
};

/**
 * Run Dijkstra's algorithm from src to dest on the given graph.
 * 
 * @param graph The city road network
 * @param src   Source node ID
 * @param dest  Destination node ID
 * @return PathResult containing the shortest path, distance, and stats
 */
PathResult dijkstra(const Graph& graph, int src, int dest);

} // namespace routeiq

#endif // DIJKSTRA_H
