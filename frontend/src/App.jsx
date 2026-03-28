import React, { useState } from 'react';
import { useRouteAPI } from './hooks/useRouteAPI';
import Sidebar from './components/Sidebar';
import MapView from './components/MapView';

/**
 * @file App.jsx
 * @description Root component for RouteIQ — Smart City Route Planner.
 * 
 * Layout:
 *   ┌─────────────────────────────────────────────┐
 *   │  TopBar (logo + app name + algo badge)       │
 *   ├──────────┬──────────────────────────────────┤
 *   │ Sidebar  │                                    │
 *   │ (source, │         MapView                    │
 *   │  dest,   │    (Leaflet + OpenStreetMap)       │
 *   │  algo,   │                                    │
 *   │  results)│                                    │
 *   └──────────┴──────────────────────────────────┘
 */

export default function App() {
  const [sidebarCollapsed, setSidebarCollapsed] = useState(false);

  const {
    nodes,
    edges,
    graphLoading,
    graphError,
    sourceId,
    destId,
    algorithm,
    setSourceId,
    setDestId,
    setAlgorithm,
    routeResult,
    routeLoading,
    routeError,
    computeRoute,
    swapSourceDest,
    handleNodeClick,
  } = useRouteAPI();

  return (
    <div className="app-container">
      {/* ─── Top Navigation Bar ─── */}
      <header className="topbar">
        <div className="topbar-logo">
          <div className="topbar-logo-icon">R</div>
          RouteIQ
        </div>
        <span className="topbar-subtitle">Smart City Route Planner</span>
        <div className="topbar-right">
          {routeResult && routeResult.found && (
            <span className={`algo-badge ${routeResult.algorithm}`}>
              {routeResult.algorithm === 'astar' ? 'A* Search' : "Dijkstra's"}
              {' · '}
              {routeResult.time_ms < 1
                ? `${(routeResult.time_ms * 1000).toFixed(0)}μs`
                : `${routeResult.time_ms.toFixed(2)}ms`}
            </span>
          )}
        </div>
      </header>

      {/* ─── Sidebar ─── */}
      <Sidebar
        nodes={nodes}
        sourceId={sourceId}
        destId={destId}
        algorithm={algorithm}
        routeResult={routeResult}
        routeLoading={routeLoading}
        routeError={routeError}
        graphLoading={graphLoading}
        graphError={graphError}
        collapsed={sidebarCollapsed}
        onToggle={() => setSidebarCollapsed(!sidebarCollapsed)}
        setSourceId={setSourceId}
        setDestId={setDestId}
        setAlgorithm={setAlgorithm}
        onFindRoute={computeRoute}
        onSwap={swapSourceDest}
      />

      {/* ─── Map View ─── */}
      <MapView
        nodes={nodes}
        edges={edges}
        sourceId={sourceId}
        destId={destId}
        routeResult={routeResult}
        onNodeClick={handleNodeClick}
        sidebarCollapsed={sidebarCollapsed}
      />

      {/* ─── Global Loading Overlay ─── */}
      {routeLoading && (
        <div className="spinner-overlay">
          <div className="spinner"></div>
          <span className="spinner-text">
            Computing shortest path with {algorithm === 'astar' ? 'A*' : "Dijkstra's"}...
          </span>
        </div>
      )}
    </div>
  );
}
