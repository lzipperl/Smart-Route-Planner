#pragma once
#ifndef ASTAR_H
#define ASTAR_H

#include "graph.h"
#include "dijkstra.h" // For PathResult struct

/**
 * @file astar.h
 * @brief A* (A-Star) shortest path algorithm with Haversine heuristic.
 * 
 * A* ALGORITHM OVERVIEW:
 * A* is an INFORMED search algorithm — it uses a heuristic to guide the search
 * towards the destination, making it faster than Dijkstra's in practice.
 * 
 * KEY DIFFERENCE FROM DIJKSTRA'S:
 *   Dijkstra's priority = g(n) = actual distance from source to n
 *   A*'s priority       = f(n) = g(n) + h(n)
 *   
 *   where h(n) is the HEURISTIC — an estimate of the remaining distance to the goal.
 *   For geographic routing, h(n) = haversine(n, destination) = straight-line distance.
 * 
 * WHY HAVERSINE IS ADMISSIBLE:
 *   An admissible heuristic NEVER OVERESTIMATES the true shortest path.
 *   Haversine gives the straight-line (great-circle) distance — no road can be
 *   shorter than the direct line between two points. Therefore:
 *     h(n) ≤ actual shortest path from n to dest  (always)
 *   This guarantees A* finds the OPTIMAL shortest path, not just any path.
 * 
 * WHEN A* IS FASTER THAN DIJKSTRA'S:
 *   - In geo-spatial graphs where the heuristic is informative
 *   - When source and destination are far apart (heuristic prunes many nodes)
 *   - Road networks are nearly planar, making haversine a tight estimate
 * 
 * WHEN A* DEGENERATES TO DIJKSTRA'S:
 *   - If h(n) = 0 for all n (no heuristic info), A* = Dijkstra's
 *   - In dense graphs where most nodes are equidistant from the goal
 * 
 * TIME COMPLEXITY: O((V + E) · log V) — same worst-case as Dijkstra's
 *   In practice, A* visits far fewer nodes due to guided search.
 * 
 * SPACE COMPLEXITY: O(V) — same as Dijkstra's
 */

namespace routeiq {

/**
 * Run the A* algorithm from src to dest on the given graph.
 * 
 * Uses haversine(n, dest) as the heuristic h(n) to guide search
 * towards the destination.
 * 
 * @param graph The city road network
 * @param src   Source node ID
 * @param dest  Destination node ID
 * @return PathResult containing the shortest path, distance, and stats
 */
PathResult astar(const Graph& graph, int src, int dest);

} // namespace routeiq

#endif // ASTAR_H
