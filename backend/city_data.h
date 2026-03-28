#pragma once
#ifndef CITY_DATA_H
#define CITY_DATA_H

#include "graph.h"

/**
 * @file city_data.h
 * @brief Hardcoded city graph data for Chandigarh, India.
 * 
 * Chandigarh is a Union Territory in northern India, designed by architect
 * Le Corbusier. Its grid-pattern road system with numbered sectors makes
 * it an ideal candidate for graph-based route planning.
 * 
 * This file contains 25 real intersections/landmarks with actual GPS 
 * coordinates retrieved from OpenStreetMap data, connected by 42 road
 * segments representing Chandigarh's major arterial roads (V1-V3 category).
 * 
 * Edge weights are computed automatically using the Haversine formula 
 * from the GPS coordinates — no manually entered distances.
 */

namespace routeiq {

/**
 * Build and return the Chandigarh city graph.
 * 
 * The graph models:
 *   - Nodes: Major intersections, roundabouts, and landmarks
 *   - Edges: Major roads connecting them (Madhya Marg, Dakshin Marg, etc.)
 * 
 * All coordinates are real-world latitude/longitude in decimal degrees.
 */
inline Graph buildChandigarhGraph() {
    Graph graph;

    // ═══════════════════════════════════════════════
    // NODES: 25 intersections/landmarks in Chandigarh
    // ═══════════════════════════════════════════════

    // Central Chandigarh
    graph.addNode(Node(0,  "Sector 17 Plaza",       30.7416, 76.7830));
    graph.addNode(Node(1,  "Sector 22 Market",      30.7334, 76.7746));
    graph.addNode(Node(2,  "Sector 34 Chowk",       30.7210, 76.7700));
    graph.addNode(Node(3,  "Sector 35 Roundabout",   30.7230, 76.7780));
    graph.addNode(Node(4,  "Tribune Chowk",          30.7280, 76.7870));
    
    // Northern Chandigarh  
    graph.addNode(Node(5,  "Sector 9 Junction",      30.7480, 76.7750));
    graph.addNode(Node(6,  "Sector 8 Crossing",      30.7500, 76.7950));
    graph.addNode(Node(7,  "Rose Garden (Sec 16)",    30.7455, 76.7850));
    graph.addNode(Node(8,  "Sector 15 Chowk",        30.7520, 76.7810));
    graph.addNode(Node(9,  "Sector 11 Junction",     30.7560, 76.7720));
    
    // Far North (Capitol area)
    graph.addNode(Node(10, "Capitol Complex",         30.7573, 76.8017));
    graph.addNode(Node(11, "Rock Garden",             30.7528, 76.8088));
    graph.addNode(Node(12, "Sukhna Lake",             30.7421, 76.8186));
    graph.addNode(Node(13, "PGI Hospital",            30.7640, 76.7760));
    graph.addNode(Node(14, "Panjab University",       30.7604, 76.7684));
    
    // Eastern Chandigarh
    graph.addNode(Node(15, "Sector 26 Crossing",     30.7344, 76.7920));
    graph.addNode(Node(16, "Elante Mall (Sec 17E)",   30.7060, 76.8010));
    graph.addNode(Node(17, "Sector 43 ISBT",          30.7268, 76.7620));
    graph.addNode(Node(18, "Sector 44 Junction",     30.7200, 76.7850));
    
    // Southern Chandigarh
    graph.addNode(Node(19, "Railway Station",         30.6876, 76.7920));
    graph.addNode(Node(20, "Airport Chowk",           30.6735, 76.7885));
    graph.addNode(Node(21, "IT Park (Sec 13)",        30.7117, 76.6930));
    graph.addNode(Node(22, "Mohali Phase 7",          30.7140, 76.7200));
    
    // Outskirts / Panchkula side
    graph.addNode(Node(23, "Manimajra Chowk",        30.7130, 76.8170));
    graph.addNode(Node(24, "Panchkula Sec 5",         30.6942, 76.8606));


    // ═══════════════════════════════════════════════
    // EDGES: 42 road segments connecting intersections
    // ═══════════════════════════════════════════════
    // Weights are AUTO-COMPUTED from Haversine(lat/lng pairs)
    // Representing Chandigarh's major road grid (V1-V3 arteries)

    // --- Madhya Marg (East-West central artery) ---
    graph.addEdge(5, 0);    // Sec 9 → Sec 17
    graph.addEdge(0, 7);    // Sec 17 → Rose Garden
    graph.addEdge(7, 6);    // Rose Garden → Sec 8
    graph.addEdge(6, 10);   // Sec 8 → Capitol Complex

    // --- Dakshin Marg (East-West southern artery) ---
    graph.addEdge(17, 2);   // ISBT → Sec 34
    graph.addEdge(2, 3);    // Sec 34 → Sec 35
    graph.addEdge(3, 4);    // Sec 35 → Tribune Chowk
    graph.addEdge(4, 15);   // Tribune Chowk → Sec 26
    graph.addEdge(15, 12);  // Sec 26 → Sukhna Lake (via Sec 26 road)

    // --- Jan Marg (North-South main artery) ---
    graph.addEdge(14, 9);   // PU → Sec 11
    graph.addEdge(9, 5);    // Sec 11 → Sec 9
    graph.addEdge(5, 1);    // Sec 9 → Sec 22
    graph.addEdge(1, 2);    // Sec 22 → Sec 34
    graph.addEdge(2, 17);   // Already added above — addEdge handles duplicates

    // --- Himalaya Marg (North-South eastern artery) ---
    graph.addEdge(13, 10);  // PGI → Capitol Complex
    graph.addEdge(10, 11);  // Capitol → Rock Garden
    graph.addEdge(11, 12);  // Rock Garden → Sukhna Lake
    graph.addEdge(12, 16);  // Sukhna Lake → Elante Mall

    // --- Sector interconnections ---
    graph.addEdge(0, 1);    // Sec 17 → Sec 22
    graph.addEdge(0, 15);   // Sec 17 → Sec 26
    graph.addEdge(0, 4);    // Sec 17 → Tribune Chowk
    graph.addEdge(7, 8);    // Rose Garden → Sec 15
    graph.addEdge(8, 9);    // Sec 15 → Sec 11
    graph.addEdge(8, 6);    // Sec 15 → Sec 8
    graph.addEdge(9, 14);   // Sec 11 → PU
    graph.addEdge(13, 14);  // PGI → PU
    graph.addEdge(6, 11);   // Sec 8 → Rock Garden

    // --- Southern connections ---
    graph.addEdge(3, 18);   // Sec 35 → Sec 44
    graph.addEdge(18, 4);   // Sec 44 → Tribune Chowk
    graph.addEdge(18, 16);  // Sec 44 → Elante Mall
    graph.addEdge(16, 23);  // Elante Mall → Manimajra
    graph.addEdge(23, 24);  // Manimajra → Panchkula
    graph.addEdge(18, 19);  // Sec 44 → Railway Station
    graph.addEdge(19, 20);  // Railway Station → Airport
    graph.addEdge(19, 23);  // Railway Station → Manimajra

    // --- Western connections (towards Mohali) ---
    graph.addEdge(17, 22);  // ISBT → Mohali Phase 7
    graph.addEdge(22, 21);  // Mohali → IT Park
    graph.addEdge(2, 22);   // Sec 34 → Mohali Phase 7

    // --- Additional connectivity for realism ---
    graph.addEdge(1, 17);   // Sec 22 → ISBT
    graph.addEdge(3, 1);    // Sec 35 → Sec 22
    graph.addEdge(15, 18);  // Sec 26 → Sec 44
    graph.addEdge(6, 15);   // Sec 8 → Sec 26
    graph.addEdge(20, 24);  // Airport → Panchkula (via ring road)

    return graph;
}

} // namespace routeiq

#endif // CITY_DATA_H
