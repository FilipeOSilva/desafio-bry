# syntax=docker/dockerfile:1
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    openssl \
    libssl-dev \
    g++ \
    make \
    libpq-dev \
    libpoco-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY include/ ./include/
COPY src/ ./src/
COPY CMakeLists.txt .

RUN mkdir -p build/docs && \
   cd build && \
   cmake .. && \
   make

CMD ["/app/build/server"]