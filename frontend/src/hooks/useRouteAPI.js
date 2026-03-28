import { useState, useEffect, useCallback } from 'react';
import { fetchNodes, fetchEdges, findRoute } from '../api/routeAPI';

/**
 * @file useRouteAPI.js
 * @description Custom React hook that manages all state for the route planner.
 * 
 * Responsibilities:
 *   - Loads graph data (nodes + edges) on mount
 *   - Manages source/destination selection state
 *   - Manages algorithm selection (dijkstra vs astar)
 *   - Handles route computation requests
 *   - Tracks loading/error states
 */

export function useRouteAPI() {
  // ─── Graph Data State ───
  const [nodes, setNodes] = useState([]);
  const [edges, setEdges] = useState([]);
  const [graphLoading, setGraphLoading] = useState(true);
  const [graphError, setGraphError] = useState(null);

  // ─── Selection State ───
  const [sourceId, setSourceId] = useState(null);
  const [destId, setDestId] = useState(null);
  const [algorithm, setAlgorithm] = useState('dijkstra');

  // ─── Route Result State ───
  const [routeResult, setRouteResult] = useState(null);
  const [routeLoading, setRouteLoading] = useState(false);
  const [routeError, setRouteError] = useState(null);

  // ─── Load graph data on mount ───
  useEffect(() => {
    let cancelled = false;

    async function loadGraph() {
      try {
        setGraphLoading(true);
        setGraphError(null);
        
        // Fetch nodes and edges in parallel
        const [nodesData, edgesData] = await Promise.all([
          fetchNodes(),
          fetchEdges()
        ]);

        if (!cancelled) {
          setNodes(nodesData);
          setEdges(edgesData);
          setGraphLoading(false);
        }
      } catch (err) {
        if (!cancelled) {
          console.error('Failed to load graph data:', err);
          setGraphError(err.message);
          setGraphLoading(false);
        }
      }
    }

    loadGraph();
    return () => { cancelled = true; };
  }, []);

  // ─── Find Route ───
  const computeRoute = useCallback(async () => {
    if (sourceId === null || destId === null) {
      setRouteError('Please select both source and destination');
      return;
    }

    if (sourceId === destId) {
      setRouteError('Source and destination must be different');
      return;
    }

    try {
      setRouteLoading(true);
      setRouteError(null);
      setRouteResult(null);

      const result = await findRoute(sourceId, destId, algorithm);
      
      setRouteResult(result);
      
      if (!result.found) {
        setRouteError('No path found between selected nodes');
      }
    } catch (err) {
      console.error('Route computation failed:', err);
      setRouteError(err.message);
    } finally {
      setRouteLoading(false);
    }
  }, [sourceId, destId, algorithm]);

  // ─── Swap source and destination ───
  const swapSourceDest = useCallback(() => {
    setSourceId(destId);
    setDestId(sourceId);
    setRouteResult(null);
    setRouteError(null);
  }, [sourceId, destId]);

  // ─── Clear route ───
  const clearRoute = useCallback(() => {
    setRouteResult(null);
    setRouteError(null);
  }, []);

  // ─── Handle node click on map ───
  const handleNodeClick = useCallback((nodeId) => {
    if (sourceId === null) {
      // First click: set source
      setSourceId(nodeId);
    } else if (destId === null) {
      // Second click: set destination (if different from source)
      if (nodeId !== sourceId) {
        setDestId(nodeId);
      }
    } else {
      // Both set: start over with new source
      setSourceId(nodeId);
      setDestId(null);
      setRouteResult(null);
      setRouteError(null);
    }
  }, [sourceId, destId]);

  // ─── Helper: get node by ID ───
  const getNodeById = useCallback((id) => {
    return nodes.find(n => n.id === id) || null;
  }, [nodes]);

  return {
    // Graph data
    nodes,
    edges,
    graphLoading,
    graphError,

    // Selection
    sourceId,
    destId,
    algorithm,
    setSourceId,
    setDestId,
    setAlgorithm,

    // Route
    routeResult,
    routeLoading,
    routeError,
    computeRoute,
    
    // Actions
    swapSourceDest,
    clearRoute,
    handleNodeClick,
    getNodeById,
  };
}
