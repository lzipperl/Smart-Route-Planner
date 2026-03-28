#include "dijkstra.h"
#include <queue>
#include <unordered_set>
#include <climits>
#include <algorithm>
#include <chrono>

/**
 * @file dijkstra.cpp
 * @brief Implementation of Dijkstra's shortest path algorithm.
 * 
 * STEP-BY-STEP ALGORITHM:
 * 1. Initialize dist[src] = 0, all others = infinity
 * 2. Push source node into the min-heap with priority 0
 * 3. While the heap is not empty:
 *    a. Extract the node u with minimum distance (greedy choice)
 *    b. If u is the destination, we're done (early termination)
 *    c. If u is already visited, skip it (lazy deletion strategy)
 *    d. Mark u as visited
 *    e. For each neighbor v of u:
 *       - If dist[u] + weight(u,v) < dist[v]:
 *         - Update dist[v] = dist[u] + weight(u,v)
 *         - Set prev[v] = u (for path reconstruction)
 *         - Push v into the heap with new distance
 * 4. Reconstruct path by backtracking through prev[] from dest to src
 * 
 * WHY GREEDY WORKS:
 * When we pop node u from the min-heap, every other node in the heap has
 * distance ≥ dist[u]. Since all edge weights are non-negative, no future
 * relaxation can find a shorter path to u. This is the "optimal substructure" 
 * that makes the greedy approach correct.
 */

namespace routeiq {

PathResult dijkstra(const Graph& graph, int src, int dest) {
    PathResult result;
    
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();

    // Validate inputs
    if (!graph.hasNode(src) || !graph.hasNode(dest)) {
        result.found = false;
        return result;
    }

    // ════════════════════════════════════════════
    // DATA STRUCTURE 1: Distance array (dist[])
    // ════════════════════════════════════════════
    // Stores the best-known distance from source to each node.
    // Initialized to infinity (max double value) — no paths known yet.
    // Space: O(V) where V = number of nodes
    // 
    // WHY unordered_map instead of vector?
    //   Node IDs may not be contiguous (0,1,2,...). Using a map allows
    //   sparse ID spaces without wasting memory.
    std::unordered_map<int, double> dist;

    // ════════════════════════════════════════════
    // DATA STRUCTURE 2: Previous-node array (prev[])
    // ════════════════════════════════════════════
    // Stores the predecessor of each node on the shortest path.
    // Used for PATH RECONSTRUCTION by backtracking from destination.
    // prev[dest] → ... → prev[prev[src]] → src
    // Space: O(V)
    std::unordered_map<int, int> prev;

    // ════════════════════════════════════════════
    // DATA STRUCTURE 3: Visited set
    // ════════════════════════════════════════════
    // Tracks which nodes have been "settled" (finalized).
    // Once a node is visited, its shortest distance is guaranteed optimal.
    // Using unordered_set for O(1) lookup.
    // Space: O(V)
    std::unordered_set<int> visited;

    // Initialize distances to infinity
    for (const auto& [id, node] : graph.getAllNodes()) {
        dist[id] = std::numeric_limits<double>::max();
        prev[id] = -1; // -1 means "no predecessor" (backtracking terminus)
    }
    dist[src] = 0.0; // Distance from source to itself is 0

    // ════════════════════════════════════════════
    // DATA STRUCTURE 4: Min-Heap Priority Queue
    // ════════════════════════════════════════════
    // The HEART of Dijkstra's algorithm.
    // 
    // WHY a min-heap?
    //   We need to repeatedly extract the unvisited node with the SMALLEST
    //   distance. A sorted array would give O(V) extraction. A min-heap
    //   gives O(log V) extraction — this is what makes Dijkstra efficient.
    // 
    // Implementation: std::priority_queue with std::greater<> for min-heap.
    //   Default C++ priority_queue is a MAX-heap, so we reverse the comparison.
    // 
    // Element type: pair<double, int> = (distance, node_id)
    //   Putting distance first because pair comparison is lexicographic —
    //   this gives us min-distance ordering automatically.
    // 
    // LAZY DELETION strategy:
    //   When we find a shorter path to a node, we don't update its existing
    //   heap entry. Instead, we push a NEW entry with the shorter distance.
    //   The old (stale) entry will eventually be popped and discarded when
    //   we see the node is already visited. This avoids expensive decrease-key.
    // 
    // Heap operations:
    //   push(): O(log V)
    //   pop(): O(log V)
    //   top(): O(1)
    
    using PQElement = std::pair<double, int>; // (distance, node_id)
    std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> min_heap;

    // Push source with distance 0
    min_heap.push({0.0, src});

    // ════════════════════════════════════════════
    // MAIN LOOP: Greedy extraction + relaxation
    // ════════════════════════════════════════════
    while (!min_heap.empty()) {
        // GREEDY STEP: Extract the closest unvisited node
        auto [current_dist, u] = min_heap.top();
        min_heap.pop();

        // Skip if already visited (lazy deletion — stale heap entry)
        if (visited.count(u)) continue;

        // Mark as visited — this node's shortest distance is now FINAL
        visited.insert(u);
        result.nodes_visited++;

        // EARLY TERMINATION: If we've reached the destination, stop.
        // All remaining heap entries have distance ≥ current_dist,
        // so no shorter path to dest can exist.
        if (u == dest) break;

        // RELAXATION: Try to improve distances to all neighbors of u
        for (const auto& [v, weight] : graph.getNeighbors(u)) {
            // Skip already-settled nodes
            if (visited.count(v)) continue;

            // Compute candidate distance through u
            double new_dist = dist[u] + weight;

            // If this path is shorter, update
            if (new_dist < dist[v]) {
                dist[v] = new_dist;
                prev[v] = u;                  // Record predecessor for path reconstruction
                min_heap.push({new_dist, v});  // Push new (shorter) entry into heap
            }
        }
    }

    // ════════════════════════════════════════════
    // PATH RECONSTRUCTION via backtracking through prev[]
    // ════════════════════════════════════════════
    // Starting from destination, follow prev[] pointers back to source.
    // This gives the path in REVERSE order, so we reverse it at the end.
    // 
    // Example: if prev = {dest→C, C→B, B→A, A→src}
    //   Backtrack: dest → C → B → A → src
    //   Reverse:   src → A → B → C → dest  ← final path
    
    if (dist[dest] < std::numeric_limits<double>::max()) {
        result.found = true;
        result.distance = dist[dest];

        // Backtrack from destination to source
        int current = dest;
        while (current != -1) {
            result.path.push_back(current);
            current = prev[current];
        }
        // Reverse to get source→destination order
        std::reverse(result.path.begin(), result.path.end());
    }

    // Stop timing
    auto end_time = std::chrono::high_resolution_clock::now();
    result.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return result;
}

} // namespace routeiq
