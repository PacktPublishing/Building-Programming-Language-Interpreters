FROM debian:latest

# Depenedencies to fetch, build llvm and clang
RUN apt-get -y update && apt-get -y dist-upgrade && apt-get install -y \
        cmake \
        build-essential \
        g++ \
        libgtest-dev \
        libuv1-dev \
        git     \
        libcli11-dev \
        && apt-get clean

COPY . src/
WORKDIR build/

# Build tool, run tests, and do a test install
RUN cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ../src
RUN cmake --build . --verbose
RUN ctest --output-on-failure
