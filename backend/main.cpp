/**
 * @file main.cpp
 * @brief RouteIQ C++ REST API Server
 * 
 * Entry point for the Smart City Route Planner backend.
 * Uses cpp-httplib (single-header HTTP library) to serve a REST API
 * that exposes graph data and pathfinding algorithms.
 * 
 * API Endpoints:
 *   GET  /health  → { "status": "ok" }
 *   GET  /nodes   → [{ id, name, lat, lng }, ...]
 *   GET  /edges   → [{ source, dest, weight }, ...]
 *   POST /route   → { path: [int], distance: float, time_ms: float, nodes_visited: int }
 * 
 * CORS headers are enabled on all routes for cross-origin frontend access.
 */

#include "httplib.h"
#include <nlohmann/json.hpp>
#include "graph.h"
#include "dijkstra.h"
#include "astar.h"
#include "haversine.h"
#include "city_data.h"

#include <iostream>
#include <string>
#include <cstdlib>

using json = nlohmann::json;

// Global graph instance — built once at startup
static routeiq::Graph cityGraph;

/**
 * Set CORS headers on every response.
 * This allows the React frontend (running on a different origin) to call our API.
 */
void setCORS(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Accept");
}

int main() {
    // ════════════════════════════════════════
    // BUILD THE CITY GRAPH
    // ════════════════════════════════════════
    std::cout << "[RouteIQ] Building Chandigarh city graph..." << std::endl;
    cityGraph = routeiq::buildChandigarhGraph();
    std::cout << "[RouteIQ] Graph loaded: " 
              << cityGraph.nodeCount() << " nodes, "
              << cityGraph.edgeCount() << " edges" << std::endl;

    // ════════════════════════════════════════
    // CREATE HTTP SERVER
    // ════════════════════════════════════════
    httplib::Server svr;

    // ─── CORS Preflight Handler ───
    // Browsers send OPTIONS requests before POST requests (CORS preflight).
    // We must respond with 200 and the allowed headers.
    svr.Options("/(.*)", [](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        res.set_header("Access-Control-Max-Age", "86400"); // Cache preflight for 24h
        res.status = 200;
    });

    // ─── GET /health ───
    // Health check endpoint for deployment platforms (Render, Railway).
    // Returns 200 with { "status": "ok" } to indicate the server is running.
    svr.Get("/health", [](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        json response = {{"status", "ok"}, {"service", "routeiq-backend"}};
        res.set_content(response.dump(), "application/json");
    });

    // ─── GET /nodes ───
    // Returns all city nodes (intersections) as a JSON array.
    // Used by the frontend to render markers on the map.
    svr.Get("/nodes", [](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        json nodes_json = json::array();

        for (const auto& [id, node] : cityGraph.getAllNodes()) {
            nodes_json.push_back({
                {"id", node.id},
                {"name", node.name},
                {"lat", node.lat},
                {"lng", node.lng}
            });
        }

        // Sort by id for consistent ordering
        std::sort(nodes_json.begin(), nodes_json.end(),
            [](const json& a, const json& b) { return a["id"] < b["id"]; });

        res.set_content(nodes_json.dump(), "application/json");
    });

    // ─── GET /edges ───
    // Returns all road segments as a JSON array.
    // Used by the frontend to draw polylines between connected nodes.
    svr.Get("/edges", [](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        json edges_json = json::array();

        for (const auto& edge : cityGraph.getAllEdges()) {
            edges_json.push_back({
                {"source", edge.source},
                {"dest", edge.dest},
                {"weight", edge.weight}
            });
        }

        res.set_content(edges_json.dump(), "application/json");
    });

    // ─── POST /route ───
    // Computes the shortest path between two nodes.
    // 
    // Request body: { "source": int, "dest": int, "algorithm": "dijkstra"|"astar" }
    // Response: { "path": [int], "distance": float, "time_ms": float, 
    //             "nodes_visited": int, "found": bool, "algorithm": string }
    // 
    // Error responses:
    //   400 — Invalid JSON, missing fields, or invalid node IDs
    //   500 — Internal server error
    svr.Post("/route", [](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);

        try {
            // Parse request body
            json body = json::parse(req.body);

            // Validate required fields
            if (!body.contains("source") || !body.contains("dest")) {
                res.status = 400;
                json error = {{"error", "Missing 'source' or 'dest' field"},
                              {"usage", "POST /route with body: { source: int, dest: int, algorithm?: 'dijkstra'|'astar' }"}};
                res.set_content(error.dump(), "application/json");
                return;
            }

            int source = body["source"].get<int>();
            int dest = body["dest"].get<int>();
            std::string algorithm = body.value("algorithm", "dijkstra");

            // Validate node IDs exist in the graph
            if (!cityGraph.hasNode(source)) {
                res.status = 400;
                json error = {{"error", "Source node " + std::to_string(source) + " not found"}};
                res.set_content(error.dump(), "application/json");
                return;
            }
            if (!cityGraph.hasNode(dest)) {
                res.status = 400;
                json error = {{"error", "Destination node " + std::to_string(dest) + " not found"}};
                res.set_content(error.dump(), "application/json");
                return;
            }
            if (source == dest) {
                res.status = 400;
                json error = {{"error", "Source and destination must be different"}};
                res.set_content(error.dump(), "application/json");
                return;
            }

            // ════════════════════════════════════════
            // RUN THE SELECTED PATHFINDING ALGORITHM
            // ════════════════════════════════════════
            routeiq::PathResult result;

            if (algorithm == "astar" || algorithm == "a*") {
                result = routeiq::astar(cityGraph, source, dest);
                algorithm = "astar";
            } else {
                result = routeiq::dijkstra(cityGraph, source, dest);
                algorithm = "dijkstra";
            }

            // Build response
            json response;
            response["found"] = result.found;
            response["algorithm"] = algorithm;
            response["nodes_visited"] = result.nodes_visited;
            response["time_ms"] = result.time_ms;

            if (result.found) {
                response["path"] = result.path;
                response["distance"] = result.distance;
                
                // Also include path node details for the frontend
                json path_details = json::array();
                for (int nodeId : result.path) {
                    const auto& node = cityGraph.getNode(nodeId);
                    path_details.push_back({
                        {"id", node.id},
                        {"name", node.name},
                        {"lat", node.lat},
                        {"lng", node.lng}
                    });
                }
                response["path_details"] = path_details;
            } else {
                response["path"] = json::array();
                response["distance"] = 0;
                response["error"] = "No path found between nodes " + 
                    std::to_string(source) + " and " + std::to_string(dest);
            }

            std::cout << "[RouteIQ] " << algorithm << "(" << source << " → " << dest << "): "
                      << (result.found ? "found" : "not found") 
                      << " | distance=" << result.distance << "m"
                      << " | nodes_visited=" << result.nodes_visited
                      << " | time=" << result.time_ms << "ms" << std::endl;

            res.set_content(response.dump(), "application/json");

        } catch (const json::exception& e) {
            res.status = 400;
            json error = {{"error", "Invalid JSON: " + std::string(e.what())}};
            res.set_content(error.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            json error = {{"error", "Internal server error: " + std::string(e.what())}};
            res.set_content(error.dump(), "application/json");
        }
    });

    // ════════════════════════════════════════
    // START THE SERVER
    // ════════════════════════════════════════
    int port = 8080;
    // Allow port override via environment variable (for deployment)
    const char* port_env = std::getenv("PORT");
    if (port_env) {
        port = std::atoi(port_env);
    }

    std::cout << "\n╔══════════════════════════════════════╗" << std::endl;
    std::cout << "║   RouteIQ Backend Server v1.0        ║" << std::endl;
    std::cout << "║   Running on http://0.0.0.0:" << port << "     ║" << std::endl;
    std::cout << "╚══════════════════════════════════════╝\n" << std::endl;

    svr.listen("0.0.0.0", port);

    return 0;
}
