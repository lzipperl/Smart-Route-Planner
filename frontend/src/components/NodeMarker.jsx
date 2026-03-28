import React, { useMemo } from 'react';
import { Marker, Tooltip } from 'react-leaflet';
import L from 'leaflet';

/**
 * @file NodeMarker.jsx
 * @description Renders a single city intersection node on the Leaflet map
 * with custom styling based on its role (source, destination, path, or default).
 */

// Create custom div icons for different node states
function createNodeIcon(className) {
  return L.divIcon({
    className: 'custom-node-marker',
    html: `<div class="node-dot ${className}"></div>`,
    iconSize: [20, 20],
    iconAnchor: [10, 10],
  });
}

const ICONS = {
  default: createNodeIcon(''),
  source: createNodeIcon('source-node'),
  dest: createNodeIcon('dest-node'),
  path: createNodeIcon('path-node'),
};

export default function NodeMarker({ node, role, onClick }) {
  // Determine which icon to use based on the node's role
  const icon = useMemo(() => {
    switch (role) {
      case 'source': return ICONS.source;
      case 'dest': return ICONS.dest;
      case 'path': return ICONS.path;
      default: return ICONS.default;
    }
  }, [role]);

  return (
    <Marker
      position={[node.lat, node.lng]}
      icon={icon}
      eventHandlers={{
        click: () => onClick && onClick(node.id),
      }}
    >
      <Tooltip
        className="node-tooltip"
        direction="top"
        offset={[0, -12]}
        opacity={0.95}
      >
        <strong>{node.name}</strong>
        {role === 'source' && ' 🟢 Source'}
        {role === 'dest' && ' 🔴 Destination'}
      </Tooltip>
    </Marker>
  );
}
