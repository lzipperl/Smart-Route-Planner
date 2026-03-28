# 🗺️ RouteIQ — Smart City Route Planner

<div align="center">

**A Google Maps-style route planning web application powered by C++ pathfinding algorithms**

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![React](https://img.shields.io/badge/React-18-61dafb.svg)](https://react.dev/)
[![Leaflet](https://img.shields.io/badge/Leaflet-1.9-199900.svg)](https://leafletjs.com/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

</div>

---

## ✨ Features

- 🗺️ **Interactive Map UI** — Full-screen Leaflet map with OpenStreetMap tiles (free, no API key)
- 🔍 **Dual Pathfinding** — Dijkstra's algorithm and A* search with haversine heuristic
- 🏙️ **Real City Data** — 25 intersections in Chandigarh, India with actual GPS coordinates
- ⚡ **Instant Results** — Sub-millisecond route computation in C++
- 🎨 **Beautiful Design** — Google Maps-inspired UI with smooth animations
- 📱 **Responsive** — Works on desktop and tablet
- 🚀 **Deployable** — Docker + Vercel/Render configs included

---

## 🏗️ Architecture

```
┌──────────────┐     HTTP/JSON      ┌──────────────────┐
│  React App   │ ◄────────────────► │  C++ REST API    │
│  (Leaflet)   │   POST /route      │  (cpp-httplib)   │
│  Port 5173   │                    │  Port 8080       │
└──────────────┘                    └──────────────────┘
     Frontend                           Backend
  Vite + Tailwind                  Dijkstra's + A*
```

---

## 🚀 Quick Start (Local Development)

### Prerequisites

- **Node.js** ≥ 18 (for the frontend)
- **CMake** ≥ 3.14 + **GCC/Clang/MSVC** (for the C++ backend)
- **Git** (for CMake FetchContent dependency downloads)

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/smart-route-planner.git
cd smart-route-planner
```

### 2. Build & Run the C++ Backend

```bash
cd backend
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Run the server
./routeiq_server        # Linux/Mac
.\Release\routeiq_server.exe  # Windows
```

The backend starts at **http://localhost:8080**. Verify with:
```bash
curl http://localhost:8080/health
# → {"service":"routeiq-backend","status":"ok"}
```

### 3. Run the React Frontend

```bash
cd frontend
npm install
npm run dev
```

Open **http://localhost:5173** in your browser. 🎉

### 4. Use the App

1. Select a **Source** from the dropdown (or click a node on the map)
2. Select a **Destination** (or click another node)
3. Choose **Dijkstra's** or **A*** algorithm
4. Click **🔍 Find Shortest Route**
5. Watch the route animate on the map! ✨

---

## 🐳 Docker (Full Stack)

Run both services with one command:

```bash
docker-compose up --build
```

- Frontend: http://localhost:5173
- Backend: http://localhost:8080

---

## 🌐 Deployment

### Backend → Render.com (Free Tier)

1. Push your repo to GitHub
2. Go to [render.com](https://render.com) → **New** → **Web Service**
3. Connect your GitHub repository
4. Configure:
   - **Root Directory**: `backend`
   - **Environment**: Docker
   - **Environment Variable**: `PORT=8080`
5. Render will auto-build from the Dockerfile
6. Note the deployed URL (e.g., `https://routeiq-backend.onrender.com`)

### Frontend → Vercel

```bash
cd frontend

# Set the backend URL
echo "VITE_API_URL=https://routeiq-backend.onrender.com" > .env.production

# Build and deploy
npm run build
npx vercel --prod
```

Or connect your GitHub repo at [vercel.com](https://vercel.com) and set:
- **Framework Preset**: Vite
- **Root Directory**: `frontend`
- **Environment Variable**: `VITE_API_URL=https://your-backend-url.onrender.com`

### Connecting Frontend ↔ Backend

The frontend reads the backend URL from the `VITE_API_URL` environment variable:

| Environment | VITE_API_URL |
|---|---|
| Local development | `http://localhost:8080` (default) |
| Production (Vercel) | `https://routeiq-backend.onrender.com` |

---

## 📁 Project Structure

```
smart-route-planner/
├── backend/
│   ├── main.cpp              # REST API server (cpp-httplib)
│   ├── graph.h / graph.cpp   # Graph class (adjacency list)
│   ├── dijkstra.h/.cpp       # Dijkstra's algorithm
│   ├── astar.h/.cpp          # A* algorithm
│   ├── haversine.h/.cpp      # GPS distance formula
│   ├── city_data.h           # Chandigarh city graph data
│   ├── CMakeLists.txt        # CMake build config
│   └── Dockerfile            # Docker build for deployment
├── frontend/
│   ├── src/
│   │   ├── App.jsx           # Root component
│   │   ├── components/
│   │   │   ├── Sidebar.jsx   # Route input panel
│   │   │   ├── MapView.jsx   # Leaflet map
│   │   │   ├── RouteResult.jsx # Results display
│   │   │   └── NodeMarker.jsx  # Custom map markers
│   │   ├── hooks/
│   │   │   └── useRouteAPI.js  # State management hook
│   │   ├── api/
│   │   │   └── routeAPI.js   # API client
│   │   └── index.css         # Design system
│   ├── index.html
│   ├── vite.config.js
│   ├── vercel.json
│   └── package.json
├── DSA_DOCUMENTATION.md      # ← Detailed DSA explanations
├── docker-compose.yml
├── render.yaml
└── README.md
```

---

## 📊 API Reference

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/health` | Health check → `{ status: "ok" }` |
| `GET` | `/nodes` | All city intersections → `[{ id, name, lat, lng }]` |
| `GET` | `/edges` | All road segments → `[{ source, dest, weight }]` |
| `POST` | `/route` | Find shortest path (see below) |

### POST /route

**Request:**
```json
{
  "source": 0,
  "dest": 4,
  "algorithm": "dijkstra"
}
```

**Response:**
```json
{
  "found": true,
  "algorithm": "dijkstra",
  "path": [0, 15, 4],
  "distance": 4200.5,
  "time_ms": 0.042,
  "nodes_visited": 8,
  "path_details": [
    { "id": 0, "name": "Sector 17 Plaza", "lat": 30.7416, "lng": 76.783 },
    { "id": 15, "name": "Sector 26 Crossing", "lat": 30.7344, "lng": 76.792 },
    { "id": 4, "name": "Tribune Chowk", "lat": 30.728, "lng": 76.787 }
  ]
}
```

---

## 🧠 Algorithms

| Algorithm | Time Complexity | Space | When to Use |
|---|---|---|---|
| **Dijkstra's** | O((V+E) log V) | O(V) | General shortest path, guaranteed optimal |
| **A*** | O((V+E) log V) | O(V) | Geo-spatial routing, faster in practice |

See [DSA_DOCUMENTATION.md](DSA_DOCUMENTATION.md) for detailed explanations, proofs, and trace examples.

---

## 🛠️ Tech Stack

| Layer | Technology |
|---|---|
| **Backend** | C++17, cpp-httplib, nlohmann/json |
| **Frontend** | React 18, Vite 6, Tailwind CSS 4 |
| **Map** | Leaflet + react-leaflet + OpenStreetMap |
| **Build** | CMake (C++), npm (JS) |
| **Deploy** | Docker, Render.com, Vercel |

---

## 📄 License

MIT License — see [LICENSE](LICENSE) for details.

---

<div align="center">

**Built with ❤️ using C++ algorithms and React**

*RouteIQ — Finding the shortest path, one intersection at a time*

</div>
