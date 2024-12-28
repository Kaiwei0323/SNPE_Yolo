#!/bin/bash

set -e

# Check if 'build' directory exists; if not, create it
if [ ! -d "build" ]; then
    mkdir build
fi

# Navigate into the 'build' directory
cd build

# Clean up previous builds if necessary
if [ -f CMakeCache.txt ]; then
    echo "Cleaning previous build..."
    make clean
fi

# Run cmake to configure the build
echo "Configuring the build with cmake..."
cmake ..

# Build the application
echo "Building the application..."
make -j$(nproc)

# Install the application
echo "Installing the application..."
make install

echo "Build and installation completed successfully!"
