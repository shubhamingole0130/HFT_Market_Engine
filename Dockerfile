# -----------------------------------------------------
# STAGE 1: Build the Engine 
# -----------------------------------------------------
FROM ubuntu:22.04 AS builder

# 1. Install System Dependencies 
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    libspdlog-dev \
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

# 2. Setup GTest 
WORKDIR /usr/src/gtest
RUN cmake CMakeLists.txt && make && cp lib/*.a /usr/lib

# 3. Copy Your Source Code 
WORKDIR /app
COPY . .

# 4. Compile the Project 
RUN mkdir build && cd build && \
    cmake .. && \
    make

# -----------------------------------------------------
# STAGE 2: The Runtime Image 
# -----------------------------------------------------
FROM ubuntu:22.04

# Install runtime libraries
RUN apt-get update && apt-get install -y \
    libspdlog1 \
    libboost-system1.74.0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/HFT_Engine .

# Command to run when the container starts
CMD ["./HFT_Engine"]