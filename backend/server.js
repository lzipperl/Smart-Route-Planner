/**
 * @file server.js
 * @description RouteIQ Backend — Node.js implementation
 * 
 * This is a JavaScript implementation of the same REST API and algorithms
 * as the C++ backend, allowing the full app to run without C++ build tools.
 * 
 * The algorithms (Dijkstra's, A*) and data structures (adjacency list,
 * min-heap, dist[], prev[]) are identical in logic to the C++ version.
 * 
 * API Endpoints:
 *   GET  /health → { status: "ok" }
 *   GET  /nodes  → [{ id, name, lat, lng }]
 *   GET  /edges  → [{ source, dest, weight }]
 *   POST /route  → { path, distance, time_ms, nodes_visited, algorithm }
 */

import http from 'http';

// ═══════════════════════════════════════════
// HAVERSINE FORMULA — GPS distance in meters
// ═══════════════════════════════════════════
function haversine(lat1, lon1, lat2, lon2) {
  const R = 6371000;
  const toRad = (deg) => (deg * Math.PI) / 180;
  const dLat = toRad(lat2 - lat1);
  const dLon = toRad(lon2 - lon1);
  const a =
    Math.sin(dLat / 2) ** 2 +
    Math.cos(toRad(lat1)) * Math.cos(toRad(lat2)) * Math.sin(dLon / 2) ** 2;
  const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
  return R * c;
}

// ═══════════════════════════════════════════
// GRAPH — Adjacency list representation
// ═══════════════════════════════════════════
class Graph {
  constructor() {
    this.nodes = new Map();     // id → { id, name, lat, lng }
    this.adjList = new Map();   // id → [{ neighbor, weight }]
    this.edgeSet = new Set();   // "min-max" keys to track unique edges
  }

  addNode(node) {
    this.nodes.set(node.id, node);
    if (!this.adjList.has(node.id)) {
      this.adjList.set(node.id, []);
    }
  }

  addEdge(src, dst) {
    const srcNode = this.nodes.get(src);
    const dstNode = this.nodes.get(dst);
    if (!srcNode || !dstNode) return;
    const weight = haversine(srcNode.lat, srcNode.lng, dstNode.lat, dstNode.lng);
    this.addEdgeWithWeight(src, dst, weight);
  }

  addEdgeWithWeight(src, dst, weight) {
    const key = `${Math.min(src, dst)}-${Math.max(src, dst)}`;
    if (this.edgeSet.has(key)) return;
    this.edgeSet.add(key);
    this.adjList.get(src)?.push({ neighbor: dst, weight });
    this.adjList.get(dst)?.push({ neighbor: src, weight });
  }

  getNeighbors(nodeId) {
    return this.adjList.get(nodeId) || [];
  }

  getNode(nodeId) {
    return this.nodes.get(nodeId);
  }

  hasNode(nodeId) {
    return this.nodes.has(nodeId);
  }

  getAllNodes() {
    return [...this.nodes.values()].sort((a, b) => a.id - b.id);
  }

  getAllEdges() {
    const edges = [];
    for (const key of this.edgeSet) {
      const [src, dst] = key.split('-').map(Number);
      const srcNode = this.nodes.get(src);
      const dstNode = this.nodes.get(dst);
      const weight = haversine(srcNode.lat, srcNode.lng, dstNode.lat, dstNode.lng);
      edges.push({ source: src, dest: dst, weight });
    }
    return edges;
  }
}

// ═══════════════════════════════════════════
// MIN-HEAP — Priority Queue for Dijkstra/A*
// ═══════════════════════════════════════════
class MinHeap {
  constructor() {
    this.heap = [];
  }
  push(item) {
    this.heap.push(item);
    this._bubbleUp(this.heap.length - 1);
  }
  pop() {
    if (this.heap.length === 0) return null;
    const top = this.heap[0];
    const last = this.heap.pop();
    if (this.heap.length > 0) {
      this.heap[0] = last;
      this._sinkDown(0);
    }
    return top;
  }
  get size() { return this.heap.length; }
  _bubbleUp(i) {
    while (i > 0) {
      const parent = Math.floor((i - 1) / 2);
      if (this.heap[parent].priority <= this.heap[i].priority) break;
      [this.heap[parent], this.heap[i]] = [this.heap[i], this.heap[parent]];
      i = parent;
    }
  }
  _sinkDown(i) {
    const n = this.heap.length;
    while (true) {
      let smallest = i;
      const left = 2 * i + 1;
      const right = 2 * i + 2;
      if (left < n && this.heap[left].priority < this.heap[smallest].priority) smallest = left;
      if (right < n && this.heap[right].priority < this.heap[smallest].priority) smallest = right;
      if (smallest === i) break;
      [this.heap[smallest], this.heap[i]] = [this.heap[i], this.heap[smallest]];
      i = smallest;
    }
  }
}

// ═══════════════════════════════════════════
// DIJKSTRA'S ALGORITHM
// ═══════════════════════════════════════════
function dijkstra(graph, src, dest) {
  const start = performance.now();
  const dist = new Map();
  const prev = new Map();
  const visited = new Set();
  let nodesVisited = 0;

  for (const [id] of graph.nodes) {
    dist.set(id, Infinity);
    prev.set(id, -1);
  }
  dist.set(src, 0);

  const heap = new MinHeap();
  heap.push({ priority: 0, node: src });

  while (heap.size > 0) {
    const { priority: currentDist, node: u } = heap.pop();
    if (visited.has(u)) continue;
    visited.add(u);
    nodesVisited++;
    if (u === dest) break;

    for (const { neighbor: v, weight } of graph.getNeighbors(u)) {
      if (visited.has(v)) continue;
      const newDist = dist.get(u) + weight;
      if (newDist < dist.get(v)) {
        dist.set(v, newDist);
        prev.set(v, u);
        heap.push({ priority: newDist, node: v });
      }
    }
  }

  const timeTaken = performance.now() - start;
  return buildResult(graph, dist, prev, src, dest, nodesVisited, timeTaken);
}

// ═══════════════════════════════════════════
// A* ALGORITHM
// ═══════════════════════════════════════════
function astar(graph, src, dest) {
  const start = performance.now();
  const destNode = graph.getNode(dest);
  const gScore = new Map();
  const fScore = new Map();
  const prev = new Map();
  const visited = new Set();
  let nodesVisited = 0;

  for (const [id] of graph.nodes) {
    gScore.set(id, Infinity);
    fScore.set(id, Infinity);
    prev.set(id, -1);
  }

  gScore.set(src, 0);
  const srcNode = graph.getNode(src);
  fScore.set(src, haversine(srcNode.lat, srcNode.lng, destNode.lat, destNode.lng));

  const heap = new MinHeap();
  heap.push({ priority: fScore.get(src), node: src });

  while (heap.size > 0) {
    const { node: u } = heap.pop();
    if (visited.has(u)) continue;
    visited.add(u);
    nodesVisited++;
    if (u === dest) break;

    for (const { neighbor: v, weight } of graph.getNeighbors(u)) {
      if (visited.has(v)) continue;
      const tentativeG = gScore.get(u) + weight;
      if (tentativeG < gScore.get(v)) {
        gScore.set(v, tentativeG);
        prev.set(v, u);
        const vNode = graph.getNode(v);
        const h = haversine(vNode.lat, vNode.lng, destNode.lat, destNode.lng);
        fScore.set(v, tentativeG + h);
        heap.push({ priority: fScore.get(v), node: v });
      }
    }
  }

  const timeTaken = performance.now() - start;
  return buildResult(graph, gScore, prev, src, dest, nodesVisited, timeTaken);
}

// ═══════════════════════════════════════════
// PATH RECONSTRUCTION
// ═══════════════════════════════════════════
function buildResult(graph, dist, prev, src, dest, nodesVisited, timeTaken) {
  const result = {
    found: false,
    path: [],
    path_details: [],
    distance: 0,
    time_ms: timeTaken,
    nodes_visited: nodesVisited,
  };

  if (dist.get(dest) < Infinity) {
    result.found = true;
    result.distance = dist.get(dest);
    let current = dest;
    while (current !== -1) {
      result.path.push(current);
      current = prev.get(current);
    }
    result.path.reverse();
    result.path_details = result.path.map((id) => {
      const n = graph.getNode(id);
      return { id: n.id, name: n.name, lat: n.lat, lng: n.lng };
    });
  }
  return result;
}

// ═══════════════════════════════════════════
// CHANDIGARH CITY GRAPH — 25 nodes, 42 edges
// ═══════════════════════════════════════════
function buildChandigarhGraph() {
  const graph = new Graph();

  // Nodes
  graph.addNode({ id: 0,  name: "Sector 17 Plaza",      lat: 30.7416, lng: 76.7830 });
  graph.addNode({ id: 1,  name: "Sector 22 Market",     lat: 30.7334, lng: 76.7746 });
  graph.addNode({ id: 2,  name: "Sector 34 Chowk",      lat: 30.7210, lng: 76.7700 });
  graph.addNode({ id: 3,  name: "Sector 35 Roundabout",  lat: 30.7230, lng: 76.7780 });
  graph.addNode({ id: 4,  name: "Tribune Chowk",         lat: 30.7280, lng: 76.7870 });
  graph.addNode({ id: 5,  name: "Sector 9 Junction",     lat: 30.7480, lng: 76.7750 });
  graph.addNode({ id: 6,  name: "Sector 8 Crossing",     lat: 30.7500, lng: 76.7950 });
  graph.addNode({ id: 7,  name: "Rose Garden (Sec 16)",   lat: 30.7455, lng: 76.7850 });
  graph.addNode({ id: 8,  name: "Sector 15 Chowk",       lat: 30.7520, lng: 76.7810 });
  graph.addNode({ id: 9,  name: "Sector 11 Junction",    lat: 30.7560, lng: 76.7720 });
  graph.addNode({ id: 10, name: "Capitol Complex",        lat: 30.7573, lng: 76.8017 });
  graph.addNode({ id: 11, name: "Rock Garden",            lat: 30.7528, lng: 76.8088 });
  graph.addNode({ id: 12, name: "Sukhna Lake",            lat: 30.7421, lng: 76.8186 });
  graph.addNode({ id: 13, name: "PGI Hospital",           lat: 30.7640, lng: 76.7760 });
  graph.addNode({ id: 14, name: "Panjab University",      lat: 30.7604, lng: 76.7684 });
  graph.addNode({ id: 15, name: "Sector 26 Crossing",    lat: 30.7344, lng: 76.7920 });
  graph.addNode({ id: 16, name: "Elante Mall (Sec 17E)",  lat: 30.7060, lng: 76.8010 });
  graph.addNode({ id: 17, name: "Sector 43 ISBT",         lat: 30.7268, lng: 76.7620 });
  graph.addNode({ id: 18, name: "Sector 44 Junction",    lat: 30.7200, lng: 76.7850 });
  graph.addNode({ id: 19, name: "Railway Station",        lat: 30.6876, lng: 76.7920 });
  graph.addNode({ id: 20, name: "Airport Chowk",          lat: 30.6735, lng: 76.7885 });
  graph.addNode({ id: 21, name: "IT Park (Sec 13)",       lat: 30.7117, lng: 76.6930 });
  graph.addNode({ id: 22, name: "Mohali Phase 7",         lat: 30.7140, lng: 76.7200 });
  graph.addNode({ id: 23, name: "Manimajra Chowk",       lat: 30.7130, lng: 76.8170 });
  graph.addNode({ id: 24, name: "Panchkula Sec 5",        lat: 30.6942, lng: 76.8606 });

  // Edges — Madhya Marg
  graph.addEdge(5, 0);  graph.addEdge(0, 7);  graph.addEdge(7, 6);  graph.addEdge(6, 10);
  // Dakshin Marg
  graph.addEdge(17, 2); graph.addEdge(2, 3);  graph.addEdge(3, 4);  graph.addEdge(4, 15);
  graph.addEdge(15, 12);
  // Jan Marg
  graph.addEdge(14, 9); graph.addEdge(9, 5);  graph.addEdge(5, 1);  graph.addEdge(1, 2);
  // Himalaya Marg
  graph.addEdge(13, 10); graph.addEdge(10, 11); graph.addEdge(11, 12); graph.addEdge(12, 16);
  // Interconnections
  graph.addEdge(0, 1);  graph.addEdge(0, 15); graph.addEdge(0, 4);
  graph.addEdge(7, 8);  graph.addEdge(8, 9);  graph.addEdge(8, 6);
  graph.addEdge(9, 14); graph.addEdge(13, 14); graph.addEdge(6, 11);
  // Southern
  graph.addEdge(3, 18); graph.addEdge(18, 4); graph.addEdge(18, 16);
  graph.addEdge(16, 23); graph.addEdge(23, 24); graph.addEdge(18, 19);
  graph.addEdge(19, 20); graph.addEdge(19, 23);
  // Western
  graph.addEdge(17, 22); graph.addEdge(22, 21); graph.addEdge(2, 22);
  // Additional
  graph.addEdge(1, 17); graph.addEdge(3, 1); graph.addEdge(15, 18);
  graph.addEdge(6, 15); graph.addEdge(20, 24);

  return graph;
}

// ═══════════════════════════════════════════
// HTTP SERVER
// ═══════════════════════════════════════════
const cityGraph = buildChandigarhGraph();
console.log(`[RouteIQ] Graph loaded: ${cityGraph.nodes.size} nodes, ${cityGraph.edgeSet.size} edges`);

const PORT = process.env.PORT || 8080;

const server = http.createServer((req, res) => {
  // CORS headers
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type, Accept');
  res.setHeader('Content-Type', 'application/json');

  // Preflight
  if (req.method === 'OPTIONS') {
    res.writeHead(200);
    res.end();
    return;
  }

  const url = new URL(req.url, `http://localhost:${PORT}`);

  // GET /health
  if (req.method === 'GET' && url.pathname === '/health') {
    res.writeHead(200);
    res.end(JSON.stringify({ status: 'ok', service: 'routeiq-backend-node' }));
    return;
  }

  // GET /nodes
  if (req.method === 'GET' && url.pathname === '/nodes') {
    res.writeHead(200);
    res.end(JSON.stringify(cityGraph.getAllNodes()));
    return;
  }

  // GET /edges
  if (req.method === 'GET' && url.pathname === '/edges') {
    res.writeHead(200);
    res.end(JSON.stringify(cityGraph.getAllEdges()));
    return;
  }

  // POST /route
  if (req.method === 'POST' && url.pathname === '/route') {
    let body = '';
    req.on('data', (chunk) => { body += chunk; });
    req.on('end', () => {
      try {
        const data = JSON.parse(body);
        const { source, dest, algorithm: algo } = data;

        if (source === undefined || dest === undefined) {
          res.writeHead(400);
          res.end(JSON.stringify({ error: "Missing 'source' or 'dest' field" }));
          return;
        }
        if (!cityGraph.hasNode(source)) {
          res.writeHead(400);
          res.end(JSON.stringify({ error: `Source node ${source} not found` }));
          return;
        }
        if (!cityGraph.hasNode(dest)) {
          res.writeHead(400);
          res.end(JSON.stringify({ error: `Destination node ${dest} not found` }));
          return;
        }
        if (source === dest) {
          res.writeHead(400);
          res.end(JSON.stringify({ error: 'Source and destination must be different' }));
          return;
        }

        let result;
        let algorithmUsed;
        if (algo === 'astar' || algo === 'a*') {
          result = astar(cityGraph, source, dest);
          algorithmUsed = 'astar';
        } else {
          result = dijkstra(cityGraph, source, dest);
          algorithmUsed = 'dijkstra';
        }

        const response = {
          found: result.found,
          algorithm: algorithmUsed,
          path: result.path,
          path_details: result.path_details,
          distance: result.distance,
          time_ms: result.time_ms,
          nodes_visited: result.nodes_visited,
        };

        console.log(`[RouteIQ] ${algorithmUsed}(${source} → ${dest}): ${result.found ? 'found' : 'not found'} | distance=${result.distance.toFixed(1)}m | visited=${result.nodes_visited} | time=${result.time_ms.toFixed(3)}ms`);

        res.writeHead(200);
        res.end(JSON.stringify(response));
      } catch (err) {
        res.writeHead(400);
        res.end(JSON.stringify({ error: 'Invalid JSON: ' + err.message }));
      }
    });
    return;
  }

  // 404
  res.writeHead(404);
  res.end(JSON.stringify({ error: 'Not found' }));
});

server.listen(PORT, () => {
  console.log('');
  console.log('╔══════════════════════════════════════╗');
  console.log(`║   RouteIQ Backend Server v1.0        ║`);
  console.log(`║   Running on http://localhost:${PORT}     ║`);
  console.log('╚══════════════════════════════════════╝');
  console.log('');
});
