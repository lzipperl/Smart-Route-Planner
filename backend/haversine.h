#pragma once
#ifndef HAVERSINE_H
#define HAVERSINE_H

/**
 * @file haversine.h
 * @brief Haversine distance formula for GPS coordinates.
 * 
 * The Haversine formula calculates the great-circle distance between two points
 * on a sphere given their latitude and longitude in decimal degrees.
 * 
 * This is used as:
 *   1. Edge weight calculation — real-world road distance between intersections
 *   2. A* heuristic h(n) — straight-line distance to destination (admissible: never overestimates)
 * 
 * WHY NOT Euclidean distance?
 *   Euclidean distance treats lat/lng as flat Cartesian coordinates, which introduces
 *   significant error because the Earth is a sphere. At Chandigarh's latitude (~30.7°N),
 *   1° of longitude ≈ 96 km but 1° of latitude ≈ 111 km. Euclidean distance would
 *   distort east-west distances by ~13%.
 * 
 * Mathematical formula:
 *   a = sin²(Δlat/2) + cos(lat1) · cos(lat2) · sin²(Δlng/2)
 *   c = 2 · atan2(√a, √(1−a))
 *   d = R · c
 * where R = 6371000 meters (Earth's mean radius)
 * 
 * Time Complexity: O(1) — fixed number of trigonometric operations
 * Space Complexity: O(1)
 */

namespace routeiq {

/**
 * Compute the great-circle distance between two GPS coordinates.
 * @param lat1 Latitude of point 1 (degrees)
 * @param lon1 Longitude of point 1 (degrees)
 * @param lat2 Latitude of point 2 (degrees)
 * @param lon2 Longitude of point 2 (degrees)
 * @return Distance in meters
 */
double haversine(double lat1, double lon1, double lat2, double lon2);

} // namespace routeiq

#endif // HAVERSINE_H
