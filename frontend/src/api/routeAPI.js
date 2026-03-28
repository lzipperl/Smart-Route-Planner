/**
 * @file routeAPI.js
 * @description API client for the RouteIQ C++ backend.
 * 
 * All API calls go to the C++ server (default: http://localhost:8080).
 * The base URL can be configured via VITE_API_URL environment variable
 * for production deployment.
 */

// API base URL — configurable for deployment
const API_BASE = import.meta.env.VITE_API_URL || 'http://localhost:8080';

/**
 * Fetch all city nodes (intersections) from the backend.
 * @returns {Promise<Array<{id: number, name: string, lat: number, lng: number}>>}
 */
export async function fetchNodes() {
  const response = await fetch(`${API_BASE}/nodes`);
  if (!response.ok) {
    throw new Error(`Failed to fetch nodes: ${response.status} ${response.statusText}`);
  }
  return response.json();
}

/**
 * Fetch all edges (road segments) from the backend.
 * @returns {Promise<Array<{source: number, dest: number, weight: number}>>}
 */
export async function fetchEdges() {
  const response = await fetch(`${API_BASE}/edges`);
  if (!response.ok) {
    throw new Error(`Failed to fetch edges: ${response.status} ${response.statusText}`);
  }
  return response.json();
}

/**
 * Compute the shortest route between two nodes.
 * @param {number} source - Source node ID
 * @param {number} dest - Destination node ID
 * @param {string} algorithm - 'dijkstra' or 'astar'
 * @returns {Promise<{
 *   found: boolean,
 *   path: number[],
 *   path_details: Array<{id: number, name: string, lat: number, lng: number}>,
 *   distance: number,
 *   time_ms: number,
 *   nodes_visited: number,
 *   algorithm: string
 * }>}
 */
export async function findRoute(source, dest, algorithm = 'dijkstra') {
  const response = await fetch(`${API_BASE}/route`, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({ source, dest, algorithm }),
  });

  if (!response.ok) {
    const errorData = await response.json().catch(() => ({}));
    throw new Error(errorData.error || `Failed to find route: ${response.status}`);
  }

  return response.json();
}

/**
 * Check if the backend server is healthy.
 * @returns {Promise<{status: string}>}
 */
export async function checkHealth() {
  const response = await fetch(`${API_BASE}/health`);
  if (!response.ok) {
    throw new Error('Backend server is not reachable');
  }
  return response.json();
}
