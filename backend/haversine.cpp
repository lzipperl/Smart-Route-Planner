#include "haversine.h"
#include <cmath>

/**
 * @file haversine.cpp
 * @brief Implementation of the Haversine distance formula.
 * 
 * Used in two critical places:
 *   1. graph.cpp — computing edge weights when building the city graph
 *   2. astar.cpp — computing h(n) heuristic for guided search
 */

namespace routeiq {

// Earth's mean radius in meters
static constexpr double EARTH_RADIUS_M = 6371000.0;

// Convert degrees to radians
static inline double toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

double haversine(double lat1, double lon1, double lat2, double lon2) {
    // Step 1: Convert all coordinates from degrees to radians
    double lat1_rad = toRadians(lat1);
    double lat2_rad = toRadians(lat2);
    double dlat = toRadians(lat2 - lat1);  // Δlat in radians
    double dlon = toRadians(lon2 - lon1);  // Δlon in radians

    // Step 2: Apply the Haversine formula
    // a = sin²(Δlat/2) + cos(lat1) · cos(lat2) · sin²(Δlon/2)
    double a = std::sin(dlat / 2.0) * std::sin(dlat / 2.0)
             + std::cos(lat1_rad) * std::cos(lat2_rad)
             * std::sin(dlon / 2.0) * std::sin(dlon / 2.0);

    // Step 3: Compute the angular distance in radians
    // c = 2 · atan2(√a, √(1−a))
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

    // Step 4: Multiply by Earth's radius to get distance in meters
    return EARTH_RADIUS_M * c;
}

} // namespace routeiq
