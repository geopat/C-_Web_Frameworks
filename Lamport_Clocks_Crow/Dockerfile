FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    libasio-dev \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Download Crow headers
RUN git clone https://github.com/CrowCpp/Crow.git /tmp/crow && \
    cp -r /tmp/crow/include/* /usr/local/include/ && \
    rm -rf /tmp/crow

# Copy source code
COPY main.cpp .

# Compile the application
RUN g++ -std=c++17 -o server main.cpp -lpthread -lboost_system

# Expose port
EXPOSE 8080

# Run the application
CMD ["./server"]