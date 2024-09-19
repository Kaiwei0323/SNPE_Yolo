#!/bin/bash

set -e

# Update package list and install dependencies
apt-get install -y libjson-glib-dev libgflags-dev libjsoncpp-dev

# Check if fmt library is already installed
if [ -d "/usr/local/include/fmt" ]; then
    echo "fmt library is already installed, skipping installation."
else
    # Clone and install fmt library
    git clone https://github.com/fmtlib/fmt.git
    cd fmt
    mkdir build
    cd build
    cmake ..
    make -j$(nproc)
    make install
    cd ../..
fi

# Check if spdlog library is already installed
if [ -d "/usr/local/include/spdlog" ]; then
    echo "spdlog library is already installed, skipping installation."
else
    # Clone and install spdlog library
    git clone https://github.com/gabime/spdlog.git
    cd spdlog
    mkdir build
    cd build
    cmake ..
    make -j$(nproc)
    make install
    cd ../..
fi

echo "Installation of libraries and packages completed successfully!"

