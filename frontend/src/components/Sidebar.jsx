import React from 'react';
import RouteResult from './RouteResult';

/**
 * @file Sidebar.jsx
 * @description Left sidebar panel with source/destination selectors,
 * algorithm toggle, find route button, and results display.
 * 
 * Designed to match Google Maps' clean white sidebar aesthetic.
 */

export default function Sidebar({
  nodes,
  sourceId,
  destId,
  algorithm,
  routeResult,
  routeLoading,
  routeError,
  graphLoading,
  graphError,
  collapsed,
  onToggle,
  setSourceId,
  setDestId,
  setAlgorithm,
  onFindRoute,
  onSwap,
}) {
  const canFindRoute = sourceId !== null && destId !== null && sourceId !== destId;

  return (
    <>
      {/* Sidebar Panel */}
      <aside className={`sidebar ${collapsed ? 'collapsed' : ''}`}>
        
        {/* Route Input Section */}
        <div className="sidebar-section">
          <div className="sidebar-title">Plan Your Route</div>

          {/* Source Selection */}
          <div className="form-group">
            <label className="form-label" htmlFor="source-select">
              <span className="form-label-dot source"></span>
              Starting Point
            </label>
            <select
              id="source-select"
              className="form-select"
              value={sourceId ?? ''}
              onChange={(e) => setSourceId(e.target.value ? Number(e.target.value) : null)}
              disabled={graphLoading}
            >
              <option value="">Select starting point...</option>
              {nodes.map(node => (
                <option key={node.id} value={node.id}>
                  {node.name}
                </option>
              ))}
            </select>
          </div>

          {/* Swap Button */}
          <div style={{ display: 'flex', justifyContent: 'center' }}>
            <button
              className="swap-btn"
              onClick={onSwap}
              title="Swap source and destination"
              disabled={sourceId === null && destId === null}
            >
              ⇅
            </button>
          </div>

          {/* Destination Selection */}
          <div className="form-group">
            <label className="form-label" htmlFor="dest-select">
              <span className="form-label-dot dest"></span>
              Destination
            </label>
            <select
              id="dest-select"
              className="form-select"
              value={destId ?? ''}
              onChange={(e) => setDestId(e.target.value ? Number(e.target.value) : null)}
              disabled={graphLoading}
            >
              <option value="">Select destination...</option>
              {nodes.map(node => (
                <option key={node.id} value={node.id}>
                  {node.name}
                </option>
              ))}
            </select>
          </div>

          {/* Algorithm Toggle */}
          <div className="form-group">
            <label className="form-label">
              ⚡ Algorithm
            </label>
            <div className="algo-toggle">
              <button
                className={`algo-toggle-btn ${algorithm === 'dijkstra' ? 'active' : ''}`}
                onClick={() => setAlgorithm('dijkstra')}
              >
                Dijkstra's
              </button>
              <button
                className={`algo-toggle-btn ${algorithm === 'astar' ? 'active' : ''}`}
                onClick={() => setAlgorithm('astar')}
              >
                A* Search
              </button>
            </div>
          </div>

          {/* Find Route Button */}
          <button
            id="find-route-btn"
            className="find-route-btn"
            onClick={onFindRoute}
            disabled={!canFindRoute || routeLoading}
          >
            {routeLoading ? (
              <>
                <div className="spinner" style={{ width: 18, height: 18, borderWidth: 2 }}></div>
                Computing...
              </>
            ) : (
              <>
                🔍 Find Shortest Route
              </>
            )}
          </button>
        </div>

        {/* Error Display */}
        {(routeError || graphError) && (
          <div className="sidebar-section">
            <div className="error-banner">
              <span className="error-icon">⚠</span>
              <span className="error-text">
                {graphError || routeError}
              </span>
            </div>
          </div>
        )}

        {/* Results Section */}
        {routeResult && routeResult.found && (
          <div className="sidebar-section">
            <div className="sidebar-title">Route Details</div>
            <RouteResult result={routeResult} nodes={nodes} />
          </div>
        )}

        {/* Graph Loading State */}
        {graphLoading && (
          <div className="sidebar-section" style={{ textAlign: 'center', padding: '40px 20px' }}>
            <div className="spinner" style={{ margin: '0 auto 12px' }}></div>
            <p style={{ fontSize: '0.85rem', color: 'var(--routeiq-text-secondary)' }}>
              Loading city graph data...
            </p>
          </div>
        )}

        {/* Tip section */}
        {!graphLoading && !routeResult && !routeError && (
          <div className="sidebar-section">
            <div style={{
              background: 'var(--routeiq-blue-light)',
              borderRadius: '10px',
              padding: '14px 16px',
              fontSize: '0.78rem',
              color: 'var(--routeiq-text-secondary)',
              lineHeight: 1.6,
            }}>
              <strong style={{ color: 'var(--routeiq-blue)' }}>💡 Tip:</strong> You can also click 
              directly on map nodes to set source and destination. First click sets source (green), 
              second sets destination (red).
            </div>
          </div>
        )}

        {/* Footer */}
        <div style={{
          marginTop: 'auto',
          padding: '16px 20px',
          borderTop: '1px solid var(--routeiq-border)',
          fontSize: '0.65rem',
          color: 'var(--routeiq-text-muted)',
          textAlign: 'center',
        }}>
          Powered by C++ • Dijkstra's & A* Algorithms
          <br />
          Chandigarh City Graph • {nodes.length} intersections
        </div>
      </aside>

      {/* Sidebar Toggle Button */}
      <button
        className={`sidebar-toggle ${collapsed ? 'collapsed' : ''}`}
        onClick={onToggle}
        title={collapsed ? 'Show sidebar' : 'Hide sidebar'}
      >
        {collapsed ? '▶' : '◀'}
      </button>
    </>
  );
}
