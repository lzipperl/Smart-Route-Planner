import React, { useMemo, useEffect, useRef } from 'react';
import { MapContainer, TileLayer, Polyline, useMap } from 'react-leaflet';
import L from 'leaflet';
import NodeMarker from './NodeMarker';

/**
 * @file MapView.jsx
 * @description Full-screen Leaflet map showing the city graph and computed routes.
 * 
 * Features:
 *   - OpenStreetMap tiles (free, no API key)
 *   - All nodes rendered as interactive markers
 *   - All edges rendered as thin gray polylines
 *   - Computed route highlighted as animated blue polyline
 *   - Auto-fit bounds when route is computed
 */

// Chandigarh city center coordinates
const CHANDIGARH_CENTER = [30.7333, 76.7794];
const DEFAULT_ZOOM = 13;

/**
 * Helper component that auto-fits the map view to show the entire route path.
 */
function FitBounds({ routeResult, nodes }) {
  const map = useMap();

  useEffect(() => {
    if (routeResult && routeResult.found && routeResult.path_details) {
      const pathCoords = routeResult.path_details.map(n => [n.lat, n.lng]);
      if (pathCoords.length > 0) {
        const bounds = L.latLngBounds(pathCoords);
        // Smooth animation to fit the route in view with padding
        map.fitBounds(bounds, {
          padding: [60, 60],
          maxZoom: 15,
          animate: true,
          duration: 1.0,
        });
      }
    }
  }, [routeResult, map]);

  return null;
}

export default function MapView({
  nodes,
  edges,
  sourceId,
  destId,
  routeResult,
  onNodeClick,
  sidebarCollapsed,
}) {
  const mapRef = useRef(null);

  // ─── Determine node roles for styling ───
  const pathNodeIds = useMemo(() => {
    if (!routeResult || !routeResult.found) return new Set();
    return new Set(routeResult.path);
  }, [routeResult]);

  const getNodeRole = (nodeId) => {
    if (nodeId === sourceId) return 'source';
    if (nodeId === destId) return 'dest';
    if (pathNodeIds.has(nodeId)) return 'path';
    return 'default';
  };

  // ─── Build edge polylines ───
  const edgeLines = useMemo(() => {
    if (!nodes.length || !edges.length) return [];

    const nodeMap = {};
    nodes.forEach(n => { nodeMap[n.id] = n; });

    return edges.map((edge, i) => {
      const src = nodeMap[edge.source];
      const dst = nodeMap[edge.dest];
      if (!src || !dst) return null;
      return {
        key: `edge-${edge.source}-${edge.dest}`,
        positions: [[src.lat, src.lng], [dst.lat, dst.lng]],
      };
    }).filter(Boolean);
  }, [nodes, edges]);

  // ─── Build route polyline ───
  const routePolyline = useMemo(() => {
    if (!routeResult || !routeResult.found || !routeResult.path_details) return null;
    return routeResult.path_details.map(n => [n.lat, n.lng]);
  }, [routeResult]);

  // ─── Invalidate map size when sidebar toggles ───
  useEffect(() => {
    if (mapRef.current) {
      setTimeout(() => {
        mapRef.current.invalidateSize({ animate: true });
      }, 400); // Wait for sidebar animation
    }
  }, [sidebarCollapsed]);

  return (
    <div className={`map-container ${sidebarCollapsed ? 'sidebar-collapsed' : ''}`}>
      <MapContainer
        center={CHANDIGARH_CENTER}
        zoom={DEFAULT_ZOOM}
        zoomControl={true}
        ref={mapRef}
        style={{ width: '100%', height: '100%' }}
      >
        {/* OpenStreetMap tile layer — free, no API key required */}
        <TileLayer
          attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a>'
          url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
        />

        {/* Auto-fit bounds when route changes */}
        <FitBounds routeResult={routeResult} nodes={nodes} />

        {/* Render all edges as thin gray lines */}
        {edgeLines.map(edge => (
          <Polyline
            key={edge.key}
            positions={edge.positions}
            pathOptions={{
              color: '#9aa0a6',
              weight: 2,
              opacity: 0.4,
              dashArray: '6 4',
            }}
            className="edge-line"
          />
        ))}

        {/* Render the computed route as a thick animated polyline */}
        {routePolyline && (
          <>
            {/* Shadow/glow line behind the main route */}
            <Polyline
              positions={routePolyline}
              pathOptions={{
                color: '#1a73e8',
                weight: 8,
                opacity: 0.2,
                lineCap: 'round',
                lineJoin: 'round',
              }}
            />
            {/* Main route line */}
            <Polyline
              positions={routePolyline}
              pathOptions={{
                color: '#1a73e8',
                weight: 5,
                opacity: 0.9,
                lineCap: 'round',
                lineJoin: 'round',
              }}
              className="route-path-animated"
            />
          </>
        )}

        {/* Render all nodes as interactive markers */}
        {nodes.map(node => (
          <NodeMarker
            key={node.id}
            node={node}
            role={getNodeRole(node.id)}
            onClick={onNodeClick}
          />
        ))}
      </MapContainer>

      {/* Loading overlay for route computation */}
    </div>
  );
}
