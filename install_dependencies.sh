#!/bin/bash

set -e

# Update package list and install dependencies
apt-get install -y libjson-glib-dev libgflags-dev libjsoncpp-dev libfmt-dev libspdlog-dev

echo "Installation of libraries and packages completed successfully!"

