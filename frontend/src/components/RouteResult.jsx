import React from 'react';

/**
 * @file RouteResult.jsx
 * @description Displays the computed route results including distance,
 * computation time, nodes visited, and the step-by-step path.
 */

/**
 * Format distance for display (meters → km if > 1000m)
 */
function formatDistance(meters) {
  if (meters >= 1000) {
    return { value: (meters / 1000).toFixed(2), unit: 'km' };
  }
  return { value: Math.round(meters), unit: 'm' };
}

/**
 * Format time for display
 */
function formatTime(ms) {
  if (ms < 1) {
    return { value: (ms * 1000).toFixed(0), unit: 'μs' };
  }
  return { value: ms.toFixed(2), unit: 'ms' };
}

export default function RouteResult({ result, nodes }) {
  if (!result || !result.found) return null;

  const dist = formatDistance(result.distance);
  const time = formatTime(result.time_ms);

  // Build path steps with node names
  const pathSteps = result.path_details || result.path.map(id => {
    const node = nodes.find(n => n.id === id);
    return node ? { id: node.id, name: node.name } : { id, name: `Node ${id}` };
  });

  return (
    <div className="route-results">
      <div className="result-card">
        {/* Header with route found badge */}
        <div className="result-header">
          <span className="result-badge">
            <span className="result-badge-icon">✓</span>
            Route Found
          </span>
          <span className={`algo-badge ${result.algorithm}`}>
            {result.algorithm === 'astar' ? 'A*' : "Dijkstra's"}
          </span>
        </div>

        {/* Stats Grid */}
        <div className="result-grid">
          <div className="result-item">
            <span className="result-item-label">Total Distance</span>
            <span className="result-item-value">
              {dist.value}
              <span className="result-item-unit">{dist.unit}</span>
            </span>
          </div>
          <div className="result-item">
            <span className="result-item-label">Compute Time</span>
            <span className="result-item-value">
              {time.value}
              <span className="result-item-unit">{time.unit}</span>
            </span>
          </div>
          <div className="result-item">
            <span className="result-item-label">Nodes Visited</span>
            <span className="result-item-value">{result.nodes_visited}</span>
          </div>
          <div className="result-item">
            <span className="result-item-label">Path Length</span>
            <span className="result-item-value">
              {result.path.length}
              <span className="result-item-unit">stops</span>
            </span>
          </div>
        </div>
      </div>

      {/* Path Steps (scrollable) */}
      <div className="sidebar-title" style={{ marginTop: 4 }}>Route Steps</div>
      <div className="path-steps" style={{ maxHeight: '200px', overflowY: 'auto' }}>
        {pathSteps.map((step, index) => {
          let dotClass = '';
          if (index === 0) dotClass = 'start';
          else if (index === pathSteps.length - 1) dotClass = 'end';

          return (
            <div key={step.id} className="path-step">
              <div className={`path-step-dot ${dotClass}`}>
                {index === 0 ? 'A' : index === pathSteps.length - 1 ? 'B' : (index)}
              </div>
              <span className="path-step-name">{step.name}</span>
            </div>
          );
        })}
      </div>
    </div>
  );
}
