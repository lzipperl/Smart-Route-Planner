# ═══════════════════════════════════════════
# RouteIQ Backend — Multi-stage Docker Build
# ═══════════════════════════════════════════
# Stage 1: Build the C++ server with CMake + GCC
# Stage 2: Copy the binary to a minimal runtime image
# 
# This keeps the final image small (~100MB vs ~1.5GB for the build image)

# ─── Stage 1: Builder ───
FROM gcc:12 AS builder

# Install CMake and git (needed for FetchContent)
RUN apt-get update && apt-get install -y \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source files
COPY . .

# Build with CMake
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --parallel $(nproc)

# ─── Stage 2: Runtime ───
FROM debian:bookworm-slim

# Install minimal runtime dependencies
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the compiled binary from the builder stage
COPY --from=builder /app/build/routeiq_server .

# Set default port (can be overridden by deployment platform)
ENV PORT=8080
EXPOSE 8080

# Health check for container orchestrators
HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Run the server
CMD ["./routeiq_server"]
