#include "astar.h"
#include "haversine.h"
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <climits>
#include <algorithm>
#include <chrono>

/**
 * @file astar.cpp
 * @brief Implementation of the A* shortest path algorithm.
 * 
 * A* uses f(n) = g(n) + h(n) as the priority in the min-heap:
 *   g(n) = actual distance from source to n (same as Dijkstra's dist[])
 *   h(n) = heuristic estimate from n to destination (haversine distance)
 *   f(n) = total estimated cost of the cheapest path through n
 * 
 * The heap is ordered by f(n), not g(n). This biases the search TOWARDS
 * the destination, visiting fewer nodes than Dijkstra's in most cases.
 * 
 * IMPORTANT: We still store and update g(n) (actual distances), and the
 * final path uses g(n) for the true distance. The heuristic only affects
 * the ORDER in which nodes are processed, not the distances computed.
 */

namespace routeiq {

PathResult astar(const Graph& graph, int src, int dest) {
    PathResult result;

    auto start_time = std::chrono::high_resolution_clock::now();

    if (!graph.hasNode(src) || !graph.hasNode(dest)) {
        result.found = false;
        return result;
    }

    // Get destination coordinates for heuristic computation
    const Node& destNode = graph.getNode(dest);

    // g(n): actual distance from source to n
    std::unordered_map<int, double> g_score;

    // f(n) = g(n) + h(n): estimated total cost through n
    // We store f_score for reference, but the heap entries carry their own f values
    std::unordered_map<int, double> f_score;

    // Previous-node map for path reconstruction (same as Dijkstra's prev[])
    std::unordered_map<int, int> prev;

    // Visited (closed) set — nodes whose optimal distance is finalized
    std::unordered_set<int> visited;

    // Initialize all distances to infinity
    for (const auto& [id, node] : graph.getAllNodes()) {
        g_score[id] = std::numeric_limits<double>::max();
        f_score[id] = std::numeric_limits<double>::max();
        prev[id] = -1;
    }

    // Source node: g(src) = 0, f(src) = h(src) = haversine(src, dest)
    g_score[src] = 0.0;
    f_score[src] = haversine(
        graph.getNode(src).lat, graph.getNode(src).lng,
        destNode.lat, destNode.lng
    );

    // ════════════════════════════════════════════
    // MIN-HEAP ordered by f(n) = g(n) + h(n)
    // ════════════════════════════════════════════
    // Key difference from Dijkstra's: priority is f(n), not g(n).
    // This means nodes CLOSER to the destination (by heuristic estimate)
    // are processed first, even if their actual distance g(n) is larger.
    //
    // Element: (f_score, node_id)
    using PQElement = std::pair<double, int>;
    std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> open_set;

    open_set.push({f_score[src], src});

    // ════════════════════════════════════════════
    // MAIN LOOP: Guided greedy extraction
    // ════════════════════════════════════════════
    while (!open_set.empty()) {
        auto [current_f, u] = open_set.top();
        open_set.pop();

        // Skip stale entries (lazy deletion)
        if (visited.count(u)) continue;

        visited.insert(u);
        result.nodes_visited++;

        // Early termination: destination reached
        if (u == dest) break;

        // Relax all neighbors of u
        for (const auto& [v, weight] : graph.getNeighbors(u)) {
            if (visited.count(v)) continue;

            // Compute tentative g(v) through u
            double tentative_g = g_score[u] + weight;

            if (tentative_g < g_score[v]) {
                // Found a better path to v through u
                g_score[v] = tentative_g;
                prev[v] = u;

                // Compute h(v) = haversine distance from v to destination
                // This is the KEY difference from Dijkstra's:
                // The heuristic GUIDES search towards the destination
                const Node& vNode = graph.getNode(v);
                double h_v = haversine(vNode.lat, vNode.lng, destNode.lat, destNode.lng);

                // f(v) = g(v) + h(v)
                // g(v) = what we've actually paid to reach v
                // h(v) = optimistic estimate of what's left to pay
                f_score[v] = tentative_g + h_v;

                open_set.push({f_score[v], v});
            }
        }
    }

    // ════════════════════════════════════════════
    // PATH RECONSTRUCTION (identical to Dijkstra's)
    // ════════════════════════════════════════════
    if (g_score[dest] < std::numeric_limits<double>::max()) {
        result.found = true;
        result.distance = g_score[dest]; // Use g(dest), not f(dest)!
        
        int current = dest;
        while (current != -1) {
            result.path.push_back(current);
            current = prev[current];
        }
        std::reverse(result.path.begin(), result.path.end());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    result.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return result;
}

} // namespace routeiq
