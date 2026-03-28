cmake_minimum_required(VERSION 3.14)
project(routeiq-backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ═══════════════════════════════════════════
# FETCH DEPENDENCIES (no manual downloads needed)
# ═══════════════════════════════════════════
include(FetchContent)

# cpp-httplib: Single-header HTTP server library
# Used instead of Crow for simplicity — no Boost/ASIO dependency
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.3
)

# nlohmann/json: Single-header JSON library for C++
# Industry-standard JSON parser used for all API request/response handling
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

FetchContent_MakeAvailable(httplib json)

# ═══════════════════════════════════════════
# BUILD THE SERVER EXECUTABLE
# ═══════════════════════════════════════════
add_executable(routeiq_server
    main.cpp
    graph.cpp
    dijkstra.cpp
    astar.cpp
    haversine.cpp
)

# Link dependencies
target_link_libraries(routeiq_server PRIVATE 
    httplib::httplib
    nlohmann_json::nlohmann_json
)

# Include current directory for our headers
target_include_directories(routeiq_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# On Linux/macOS, we need pthread for httplib's multi-threading
if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(routeiq_server PRIVATE Threads::Threads)
endif()

# ═══════════════════════════════════════════
# OPTIONAL: Install target for deployment
# ═══════════════════════════════════════════
install(TARGETS routeiq_server DESTINATION bin)
