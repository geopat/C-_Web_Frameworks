FROM ubuntu:22.04

# 1. Install dependencies (as in Drogon docs)
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    cmake \
    libjsoncpp-dev \
    uuid-dev \
    zlib1g-dev \
    openssl \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# 2. Build & install Drogon
WORKDIR /tmp

RUN git clone https://github.com/drogonframework/drogon && \
    cd drogon && \
    git submodule update --init && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j"$(nproc)" && \
    make install

# 3. Build your Drogon app
WORKDIR /app

# This will copy main.cpp, CMakeLists.txt, docker-compose.yml, README, etc.
COPY . /app

RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j"$(nproc)"

EXPOSE 8080
CMD ["/app/build/server"]
