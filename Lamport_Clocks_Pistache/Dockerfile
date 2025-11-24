FROM ubuntu:22.04

# 1. Install dependencies
RUN apt-get update && apt-get install -y \
    software-properties-common && \
    add-apt-repository ppa:pistache+team/unstable -y && \
    apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libpistache-dev \
    libjsoncpp-dev \
    && rm -rf /var/lib/apt/lists/*


# 2. Copy app sources
WORKDIR /app
COPY . /app

# 3. Configure & build
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j"$(nproc)"

EXPOSE 8080

# Default binary name is 'server' from CMakeLists
CMD ["/app/build/server"]
