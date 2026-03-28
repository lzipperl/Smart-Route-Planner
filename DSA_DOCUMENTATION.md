# 📚 DSA_DOCUMENTATION.md — RouteIQ Smart City Route Planner

> **Purpose**: This document explains every data structure and algorithm used in the RouteIQ project.  
> **Audience**: Students, interviewers, and anyone who wants to understand the DSA foundations behind a route planning system.

---

## 1. Project Architecture Overview

### High-Level Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          USER'S BROWSER                                 │
│                                                                         │
│  ┌──────────────┐    ┌──────────────────────────────────────────────┐   │
│  │   Sidebar     │    │              MapView (Leaflet)               │   │
│  │              │    │                                              │   │
│  │  [Source ▼]  │    │    ┌─────┐         ┌─────┐                  │   │
│  │  [Dest   ▼]  │    │    │Node │────────│Node │                  │   │
│  │              │    │    │  0  │  edge   │  1  │                  │   │
│  │  ◉ Dijkstra  │    │    └──┬──┘         └──┬──┘                  │   │
│  │  ○ A*        │    │       │               │                      │   │
│  │              │    │    ┌──┴──┐         ┌──┴──┐                  │   │
│  │ [Find Route] │    │    │Node │════════│Node │  ← Route         │   │
│  │              │    │    │  2  │ (thick) │  3  │                  │   │
│  │ Distance:    │    │    └─────┘         └─────┘                  │   │
│  │  4.2 km     │    │                                              │   │
│  │ Time: 0.05ms│    │         (OpenStreetMap tiles)                │   │
│  └──────────────┘    └──────────────────────────────────────────────┘   │
│                                                                         │
└────────────────────────────────┬────────────────────────────────────────┘
                                 │  HTTP (JSON)
                                 │
                    ┌────────────▼────────────┐
                    │   POST /route           │
                    │   { source: 0,          │
                    │     dest: 3,            │
                    │     algorithm: "astar"} │
                    └────────────┬────────────┘
                                 │
                    ┌────────────▼────────────────────────┐
                    │     C++ BACKEND (cpp-httplib)        │
                    │                                      │
                    │  ┌─────────────────────────────┐    │
                    │  │     Graph Engine             │    │
                    │  │                              │    │
                    │  │  city_data.h → Graph object  │    │
                    │  │       │                       │    │
                    │  │  dijkstra(src, dst)          │    │
                    │  │       OR                      │    │
                    │  │  astar(src, dst)             │    │
                    │  │       │                       │    │
                    │  │  PathResult {                │    │
                    │  │    path: [0,2,3],            │    │
                    │  │    distance: 4200.5,         │    │
                    │  │    time_ms: 0.05             │    │
                    │  │  }                           │    │
                    │  └─────────────────────────────┘    │
                    │                                      │
                    └────────────┬────────────────────────┘
                                 │
                    ┌────────────▼────────────┐
                    │   JSON Response          │
                    │   { path: [0,2,3],       │
                    │     distance: 4200.5,    │
                    │     time_ms: 0.05,       │
                    │     nodes_visited: 8,    │
                    │     path_details: [...] }│
                    └──────────────────────────┘
```

### Data Flow (Step by Step)

1. **User Click** → User selects source node "Sector 17 Plaza" and destination "Tribune Chowk" in the Sidebar dropdown, then clicks "Find Route"
2. **HTTP Request** → `routeAPI.js` sends `POST /route` with `{ source: 0, dest: 4, algorithm: "dijkstra" }` to the C++ backend
3. **Graph Lookup** → `main.cpp` receives the request, validates node IDs exist in the global `Graph` object
4. **Algorithm Execution** → `dijkstra.cpp` or `astar.cpp` runs on the adjacency list, using a min-heap priority queue
5. **Path Reconstruction** → The algorithm backtracks through the `prev[]` array from destination to source
6. **JSON Response** → Server returns `{ path: [0, 7, 6, ...], distance: 4200.5, time_ms: 0.05 }`
7. **Map Render** → `MapView.jsx` receives the path, draws an animated blue polyline on the Leaflet map, and auto-zooms to fit

---

## 2. Data Structures Used

### 2.1 Adjacency List

| Property | Detail |
|---|---|
| **Type** | `std::unordered_map<int, std::vector<std::pair<int, double>>>` |
| **File** | `graph.h` / `graph.cpp` — `Graph::adj_list_` member variable |
| **Purpose** | Stores the city road network — which intersections connect to which |

**Why this structure?**

| Alternative | Lookup | Space | Why Not? |
|---|---|---|---|
| **Adjacency Matrix** | O(1) | O(V²) | Chandigarh has 25 nodes → matrix = 625 cells, but only ~80 are non-zero. 87% wasted space. |
| **Edge List** | O(E) | O(E) | Finding neighbors requires scanning ALL edges. Too slow for pathfinding. |
| **Adjacency List (chosen)** | O(1) avg | O(V+E) | Hash map gives O(1) node lookup. Vector gives cache-friendly neighbor iteration. |

**Why `unordered_map` over `map`?**
- `unordered_map` uses a hash table → O(1) average lookup
- `map` uses a red-black tree → O(log V) lookup
- We don't need sorted keys, so hashing wins

**Code Declaration** (`graph.h`):
```cpp
// Key: node ID | Value: vector of (neighbor_id, edge_weight) pairs
std::unordered_map<int, std::vector<std::pair<int, double>>> adj_list_;
```

**Key Usage** (`graph.cpp`):
```cpp
// Adding a bidirectional edge (undirected road)
adj_list_[src].push_back({dst, weight});  // Forward direction
adj_list_[dst].push_back({src, weight});  // Reverse direction
```

**Complexity**:
- Insert edge: O(1) amortized
- Get neighbors: O(1) lookup + O(degree) iteration
- Space: O(V + 2E) — each undirected edge stored twice

---

### 2.2 Min-Heap Priority Queue

| Property | Detail |
|---|---|
| **Type** | `std::priority_queue<pair<double,int>, vector<...>, greater<...>>` |
| **File** | `dijkstra.cpp` line ~85, `astar.cpp` line ~65 |
| **Purpose** | Selects the closest unvisited node in O(log V) time — the GREEDY step |

**Why a min-heap?**

This is the **most critical** data structure in both algorithms. At each step, we need the unvisited node with the smallest distance. Without a heap:

| Approach | Extract-Min | Total Time |
|---|---|---|
| Unsorted array | O(V) per extraction | O(V²) |
| Sorted array | O(1) extract, O(V) insert | O(V²) |
| **Min-heap (chosen)** | **O(log V)** | **O((V+E) log V)** |
| Fibonacci heap | O(1) amortized | O(V log V + E) — theoretical, impractical |

For our graph (V=25, E=42), the difference is small, but for real cities with millions of nodes, the heap makes Dijkstra's feasible.

**Code** (`dijkstra.cpp`):
```cpp
// Element: (distance, node_id) — distance FIRST for lexicographic min ordering
using PQElement = std::pair<double, int>;

// std::greater makes it a MIN-heap (default C++ PQ is max-heap)
std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> min_heap;

min_heap.push({0.0, src});         // O(log V) insertion
auto [dist, node] = min_heap.top(); // O(1) peek at minimum
min_heap.pop();                     // O(log V) extraction
```

**Lazy Deletion Strategy**: Instead of implementing `decrease_key()` (which `std::priority_queue` doesn't support), when we find a shorter path to a node, we push a NEW entry with the shorter distance. Stale entries are skipped when popped (if the node is already visited). This is simpler and works well in practice.

---

### 2.3 Distance Array (`dist[]`)

| Property | Detail |
|---|---|
| **Type** | `std::unordered_map<int, double>` |
| **File** | `dijkstra.cpp` — variable `dist`, `astar.cpp` — variable `g_score` |
| **Purpose** | Tracks the best-known shortest distance from source to each node |

```cpp
// Initialize ALL distances to infinity — no paths known yet
std::unordered_map<int, double> dist;
for (const auto& [id, node] : graph.getAllNodes()) {
    dist[id] = std::numeric_limits<double>::max();  // ∞
}
dist[src] = 0.0;  // Distance from source to itself is 0
```

**Why `unordered_map` instead of `vector`?**
- Node IDs may not be contiguous (0, 1, 2, ..., N)
- Map handles sparse or non-sequential IDs gracefully
- For contiguous IDs, a vector would be slightly faster

**Operations**:
- Initialize: O(V)
- Read/Write: O(1) average
- Space: O(V)

---

### 2.4 Previous-Node Array (`prev[]`)

| Property | Detail |
|---|---|
| **Type** | `std::unordered_map<int, int>` |
| **File** | `dijkstra.cpp`, `astar.cpp` — variable `prev` |
| **Purpose** | Stores the predecessor of each node on the shortest path — used for path reconstruction |

**How path reconstruction works**:
```
Suppose we found: src=0, dest=4, and during relaxation:
  prev[4] = 15   (reached 4 from 15)
  prev[15] = 0   (reached 15 from 0)
  prev[0] = -1   (source has no predecessor)

Backtrack: 4 → 15 → 0 → done
Reverse:   0 → 15 → 4  ← this is the shortest path!
```

**Code** (`dijkstra.cpp`):
```cpp
// Backtrack from destination to source
int current = dest;
while (current != -1) {
    result.path.push_back(current);  // Add to path
    current = prev[current];          // Follow predecessor link
}
std::reverse(result.path.begin(), result.path.end());  // Reverse to get src→dst order
```

---

### 2.5 Visited Set

| Property | Detail |
|---|---|
| **Type** | `std::unordered_set<int>` |
| **File** | `dijkstra.cpp`, `astar.cpp` — variable `visited` |
| **Purpose** | Prevents reprocessing nodes whose shortest distance is already finalized |

```cpp
std::unordered_set<int> visited;  // O(1) lookup via hash set

// In main loop:
if (visited.count(u)) continue;  // Skip already-settled nodes
visited.insert(u);                // Mark as settled
```

**Why `unordered_set` over `set`?**
- `unordered_set`: O(1) lookup and insert (hash-based)
- `set`: O(log V) lookup and insert (tree-based)
- We only need membership testing, not ordering

---

### 2.6 Node and Edge Structs

| Property | Detail |
|---|---|
| **Type** | `struct Node { int id; string name; double lat, lng; }` |
| **File** | `graph.h` |
| **Purpose** | Stores intersection data with GPS coordinates for real-world mapping |

The `lat`/`lng` coordinates serve dual purposes:
1. **Frontend rendering** — placed as markers on the Leaflet map
2. **A* heuristic** — `haversine(node.lat, node.lng, dest.lat, dest.lng)` gives straight-line distance

---

## 3. Algorithms Explained

### 3.1 Dijkstra's Algorithm

#### Plain English Explanation

Imagine you're at an intersection in Chandigarh and want to find the shortest driving route to another. Dijkstra's algorithm works like this:

1. **Start at your location**. Mark it with distance 0. Every other intersection starts with distance "infinity" (unknown).
2. **Look at all roads from where you are**. For each neighboring intersection, calculate: "my distance + road length". If this is shorter than what we knew before, update it.
3. **Move to the nearest unvisited intersection** (the greedy choice). The one with the smallest distance so far.
4. **Repeat until you reach the destination** or run out of intersections.

The key insight: once you "visit" an intersection (pick it as the minimum), you'll never find a shorter way to reach it. This is because all remaining roads have non-negative lengths — you can't get closer by going further away first.

#### Why It Works (Greedy Proof Intuition)

When we extract node `u` from the min-heap with distance `d`:
- Every other node in the heap has distance ≥ `d`
- All edge weights are ≥ 0
- Any alternative path to `u` would go through an unvisited node (distance ≥ `d`) plus a non-negative edge
- Therefore, no alternative path can have distance < `d`
- So `d` is the true shortest distance to `u` ✓

#### Time Complexity: O((V + E) · log V)

Breaking it down:
- **V vertices** are each extracted from the heap at most once → V × O(log V) = O(V log V)
- **E edges** are each relaxed at most once, potentially inserting into the heap → E × O(log V) = O(E log V)
- **Total**: O(V log V + E log V) = **O((V + E) log V)**

For Chandigarh (V=25, E=42): O(67 × 5) ≈ 335 operations. Practically instant.

#### Space Complexity: O(V)
- `dist[]`: O(V)
- `prev[]`: O(V)
- `visited`: O(V)
- Heap: at most O(V) entries (with lazy deletion, could be O(E) in worst case)

#### Trace Example (5 Nodes from Chandigarh)

Let's trace Dijkstra from **Node 0 (Sector 17 Plaza)** to **Node 4 (Tribune Chowk)**:

```
Graph subset:
  0 (Sec 17) --950m-- 1 (Sec 22) --1100m-- 2 (Sec 34)
       |                                       |
      800m                                    900m
       |                                       |
  7 (Rose Garden) --1200m-- ... --700m-- 4 (Tribune Chowk)
       |
  0 --1050m-- 4 (direct road via Sec 17→Tribune)

Initial state:
  dist = {0:0, 1:∞, 2:∞, 4:∞, 7:∞}
  heap = [(0, 0)]

Step 1: Pop (0, node 0). Visit node 0.
  Relax 0→1: dist[1] = 0+950 = 950    → push (950, 1)
  Relax 0→7: dist[7] = 0+800 = 800    → push (800, 7)
  Relax 0→4: dist[4] = 0+1050 = 1050  → push (1050, 4)
  heap = [(800,7), (950,1), (1050,4)]

Step 2: Pop (800, node 7). Visit node 7.
  Relax 7→4: dist[4] = min(1050, 800+700) = 1050 vs 1500 → no update
  heap = [(950,1), (1050,4)]

Step 3: Pop (950, node 1). Visit node 1.
  Relax 1→2: dist[2] = 950+1100 = 2050  → push (2050, 2)
  heap = [(1050,4), (2050,2)]

Step 4: Pop (1050, node 4). Node 4 is destination → DONE!

Result: path = [0, 4], distance = 1050m, nodes_visited = 4
```

#### Code Walkthrough

See `dijkstra.cpp` — every line is annotated with comments explaining the algorithmic step it performs.

---

### 3.2 A* Algorithm

#### How A* Differs from Dijkstra's

| Aspect | Dijkstra's | A* |
|---|---|---|
| Priority | `g(n)` = actual distance from source | `f(n) = g(n) + h(n)` = actual + estimated remaining |
| Strategy | Explores uniformly in all directions | Biased TOWARDS the destination |
| Heuristic | None (equivalent to h(n)=0) | `haversine(n, dest)` = straight-line distance |
| Nodes visited | More (explores many irrelevant nodes) | Fewer (guided search) |
| Optimality | Always optimal | Optimal IF heuristic is admissible |

#### f(n) = g(n) + h(n) Explained

- **g(n)**: The actual distance from source to node n (same as Dijkstra's `dist[n]`)
- **h(n)**: A heuristic ESTIMATE of the remaining distance from n to the destination
- **f(n)**: The total estimated cost of the cheapest path from source through n to destination

The heap is sorted by `f(n)`, not `g(n)`. This means nodes that are CLOSER to the destination (by heuristic estimate) are processed first, even if their actual distance from the source is larger.

#### Why Haversine Is Used as h(n)

The haversine formula gives the **straight-line (great-circle) distance** between two GPS points on Earth's surface. This is the SHORTEST possible distance between two points — no road can be shorter than a direct line.

This makes haversine an **admissible heuristic**: it NEVER OVERESTIMATES the true shortest path.

```
h(n) = haversine(n, dest) ≤ actual_shortest_path(n, dest)    ALWAYS
```

Why? Because the actual shortest path follows roads which are curved, have turns, and are longer than a straight line in every case.

#### Admissibility Proof

A heuristic h(n) is admissible if: **h(n) ≤ h*(n)** for all n, where h*(n) is the true shortest distance.

For haversine:
- h(n) = straight-line distance (ignoring roads)
- h*(n) = actual shortest road path
- Roads are ALWAYS longer than or equal to straight-line distance
- Therefore: haversine(n, dest) ≤ actual_road_distance(n, dest) ✓

#### When A* is Faster vs When It Isn't

**A* is faster when:**
- Source and destination are far apart → heuristic has more "signal"
- The road network is geographically spread out → haversine is a tight estimate
- There are many nodes in "wrong" directions that Dijkstra would explore

**A* degenerates to Dijkstra's when:**
- h(n) = 0 for all n (no heuristic information)
- The graph is very small (like our 25-node Chandigarh graph)
- All nodes are equidistant from the destination

---

### 3.3 Haversine Formula

#### What It Computes

The Haversine formula calculates the **great-circle distance** between two points on a sphere, given their latitude and longitude. It accounts for the curvature of the Earth.

#### Mathematical Formula

```
Given: (lat₁, lon₁) and (lat₂, lon₂) in radians

a = sin²(Δlat/2) + cos(lat₁) · cos(lat₂) · sin²(Δlon/2)

c = 2 · atan2(√a, √(1−a))

d = R · c

where:
  Δlat = lat₂ − lat₁
  Δlon = lon₂ − lon₁
  R = 6,371,000 meters (Earth's mean radius)
  d = distance in meters
```

#### Why Euclidean Distance Would Be Wrong

At Chandigarh's latitude (~30.7°N):
- 1° latitude ≈ **111 km** (roughly constant everywhere)
- 1° longitude ≈ **96 km** (shrinks as you approach the poles: cos(30.7°) ≈ 0.86)

```
Euclidean: d = √((Δlat)² + (Δlon)²) — treats both as equal
Truth:     1° lat ≠ 1° lon in real-world distance

Example: (30.74, 76.78) to (30.74, 76.80)
  Euclidean: √(0² + 0.02²) = 0.02° ≈ would assume ~2.2 km
  Haversine: 0.02° × cos(30.7°) × 111 km/° ≈ 1.9 km
  Error: ~15% — significant for routing!
```

---

## 4. Why These Choices?

### Adjacency List over Adjacency Matrix

| Factor | Adjacency List | Adjacency Matrix |
|---|---|---|
| Space | O(V + E) = ~105 | O(V²) = 625 |
| Find neighbors | O(degree) ≈ O(3) avg | O(V) = 25 |
| Add edge | O(1) | O(1) |
| Check edge exists | O(degree) | O(1) |
| **Winner for routing** | ✅ | ❌ |

Road networks are **sparse**: each intersection connects to ~3-4 roads, not all 25. Adjacency lists exploit this sparsity.

### Min-Heap over Simple Array

Processing V nodes with extract-min:
- **Array**: O(V) per extraction × V extractions = **O(V²) = 625**
- **Heap**: O(log V) per extraction × V extractions = **O(V log V) ≈ 125**

For large cities (V=100,000): Array = 10 billion operations. Heap = 1.7 million. The heap is **5,800× faster**.

### unordered_map over map

- `unordered_map`: Hash table → O(1) average operations
- `map`: Red-black tree → O(log n) operations
- We never need sorted iteration, so O(1) hash wins

### A* over Dijkstra for Geo-Spatial Routing

Dijkstra's explores nodes in concentric circles from the source. If the destination is to the east, it wastes time exploring nodes to the north, south, and west.

A* with haversine pushes exploration towards the destination. In large road networks, A* typically visits **50-80% fewer nodes** than Dijkstra's.

---

## 5. Complexity Analysis Summary Table

| Algorithm | Time Complexity | Space Complexity | Best Case | Worst Case | Notes |
|---|---|---|---|---|---|
| **Dijkstra's** | O((V+E) log V) | O(V) | O(log V) — dest is a neighbor | O((V+E) log V) — dest is farthest | Guaranteed optimal for non-negative weights |
| **A*** | O((V+E) log V) | O(V) | O(log V) — heuristic guides directly | O((V+E) log V) — h(n)=0, degenerates to Dijkstra | Optimal if h(n) is admissible |
| **Haversine** | O(1) | O(1) | O(1) | O(1) | Fixed trigonometric operations |
| **Graph Build** | O(V + E) | O(V + E) | — | — | One-time startup cost |
| **Path Reconstruction** | O(P) | O(P) | — | O(V) — path visits all nodes | P = path length |

---

## 6. Logical Thinking & Design Decisions

### How the Road Network Is Modeled as a Graph

Real-world roads map naturally to graph theory:
- **Intersections** → Nodes (with latitude/longitude for positioning)
- **Road segments** → Edges (with weight = distance between intersections)
- **Two-way roads** → Undirected edges (each edge stored in both directions)

### How Bidirectional Edges Are Handled

When adding edge (A, B):
```cpp
adj_list_[A].push_back({B, weight});  // A → B
adj_list_[B].push_back({A, weight});  // B → A (reverse)
```
Both directions have the same weight (road distance doesn't change direction). An `edge_set_` tracks unique edges to prevent duplicates.

### How Edge Weights Are Calculated

Edge weights represent real-world road distance in meters, computed at graph-build time:
```cpp
void Graph::addEdge(int src, int dst) {
    double weight = haversine(nodes_[src].lat, nodes_[src].lng,
                              nodes_[dst].lat, nodes_[dst].lng);
    addEdgeWithWeight(src, dst, weight);
}
```
This ensures weights are consistent with the GPS coordinates visible on the map.

### How Path Reconstruction Works

The `prev[]` array forms a linked chain from destination back to source:

```
prev[dest] = C
prev[C]    = B
prev[B]    = A
prev[A]    = src
prev[src]  = -1  (terminus)

Backtrack: dest → C → B → A → src
Reverse:   [src, A, B, C, dest]  ← final path
```

This is a classic **backtracking** technique that works because each node's predecessor is recorded during the relaxation step.

### How C++ Backend Communicates with React Frontend

```
React (JS)                     C++ (cpp-httplib)
    │                               │
    │   POST /route                 │
    │   Content-Type: application/json
    │   {"source":0, "dest":4,      │
    │    "algorithm":"dijkstra"}     │
    │  ───────────────────────►     │
    │                               │  Parse JSON → run dijkstra()
    │                               │  Build response JSON
    │   200 OK                      │
    │   {"path":[0,15,4],           │
    │    "distance":4200.5,         │
    │    "nodes_visited":8}         │
    │  ◄───────────────────────     │
    │                               │
    │   Draw animated polyline     │
    │   on Leaflet map              │
```

CORS headers (`Access-Control-Allow-Origin: *`) enable the cross-origin request from the React dev server (port 5173) to the C++ server (port 8080).

### Challenges and Solutions

| Challenge | Solution |
|---|---|
| C++ lacks built-in JSON support | Used nlohmann/json single-header library |
| `std::priority_queue` has no `decrease_key` | Used lazy deletion (push new entry, skip stale ones) |
| Leaflet markers need custom styling | Created L.divIcon with HTML/CSS instead of default markers |
| Route animation on polyline | Used CSS `stroke-dasharray` + `stroke-dashoffset` animation |
| Haversine vs Euclidean precision | Haversine accounts for Earth's curvature (~15% more accurate) |

---

## 7. Learning Takeaways

### What Makes Dijkstra's Optimal for Non-Negative Weighted Graphs

Dijkstra's greedy strategy works because of the **monotonicity principle**: once we settle a node (pop it from the heap), all future paths to that node would go through unsettled nodes with equal or greater distances, plus non-negative edges. Therefore, the first time we reach a node is guaranteed to be the shortest.

This breaks if edges can be **negative**: a negative edge from a distant node could create a shorter path through it, violating the monotonicity assumption. (For negative edges, use Bellman-Ford instead.)

### What Breaks If You Use BFS Instead

BFS finds the shortest path in terms of **number of edges (hops)**, not **total weight**. It assumes all edges have equal weight.

```
Example: A --100m-- B --100m-- C
         A --------500m------- C

BFS path: A → C (1 hop) — distance 500m ❌
Dijkstra: A → B → C (2 hops) — distance 200m ✓
```

BFS would choose the direct road (fewer hops) even though it's 2.5× longer. For a route planner where roads have different lengths, BFS gives incorrect results.

### Why You Can't Use DFS for Shortest Path

DFS (Depth-First Search) explores one branch completely before backtracking. It does NOT guarantee finding the shortest path because:

1. **DFS may find a long path first**: It dives deep down one branch and returns that path, even if a shorter one exists in an unexplored branch.
2. **No notion of "distance priority"**: Unlike Dijkstra's (min-heap) or BFS (FIFO queue), DFS uses a stack and has no mechanism to prefer shorter paths.
3. **DFS is designed for graph traversal** (reachability, cycle detection), not shortest-path optimization.

```
Example: 
  A →(100m)→ B →(100m)→ C
  A →(50m)→ C

DFS might explore: A → B → C (200m) first and return it ❌
Correct shortest:  A → C (50m) ✓
```

---

## Appendix: File Reference

| File | Key DSA Contents |
|---|---|
| `backend/graph.h` | Adjacency list declaration, Node/Edge structs |
| `backend/graph.cpp` | Bidirectional edge insertion, neighbor lookup |
| `backend/dijkstra.h` | PathResult struct, Dijkstra function signature |
| `backend/dijkstra.cpp` | Full Dijkstra with min-heap, dist[], prev[], visited set |
| `backend/astar.h` | A* function signature, f(n)=g(n)+h(n) explanation |
| `backend/astar.cpp` | A* with haversine heuristic, guided search |
| `backend/haversine.h/.cpp` | Great-circle distance formula |
| `backend/city_data.h` | 25-node Chandigarh graph with real GPS coordinates |
| `backend/main.cpp` | REST API server, JSON handling, CORS |
