#!/bin/bash

set -e

# Check if OpenCV is already installed
if [ -d "/usr/local/include/opencv4/opencv2" ]; then
    echo "OpenCV is already installed in /usr/local/include/opencv4. Skipping installation."
    exit 0
fi

# If not installed, continue with the installation
echo "OpenCV is not installed. Proceeding with installation."

apt install git

# Update package list and install dependencies
apt install build-essential cmake git pkg-config libgtk-3-dev \
                 libavcodec-dev libavformat-dev libswscale-dev libv4l-dev \
                 libxvidcore-dev libx264-dev libjpeg-dev libpng-dev \
                 libtiff-dev gfortran openexr libatlas-base-dev \
                 python3-dev python3-numpy libtbb2 libtbb-dev \
                 libdc1394-22-dev libeigen3-dev libopenblas-dev \
                 liblapack-dev libavresample-dev libgstreamer1.0-dev \
                 libgstreamer-plugins-base1.0-dev libgoogle-glog-dev \
                 libprotobuf-dev protobuf-compiler libprotoc-dev \
                 libgoogle-glog-dev libgflags-dev libgphoto2-dev \
                 libhdf5-dev

# Create or navigate to build directory
if [ -d "~/opencv_build" ]; then
    echo "~/opencv_build directory already exists. Navigating to it."
    cd ~/opencv_build
else
    echo "~/opencv_build directory does not exist. Creating and navigating to it."
    mkdir ~/opencv_build
    cd ~/opencv_build
fi

# Clone OpenCV and OpenCV contrib repositories
git clone --branch 4.5.5 https://github.com/opencv/opencv.git
git clone --branch 4.5.5 https://github.com/opencv/opencv_contrib.git

# Run CMake to configure the build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
      -D CMAKE_INSTALL_PREFIX=/usr/local \
      -D OPENCV_EXTRA_MODULES_PATH=~/opencv_build/opencv_contrib/modules \
      -D ENABLE_NEON=ON \
      -D WITH_OPENMP=ON \
      -D WITH_OPENGL=ON \
      -D WITH_TBB=ON \
      -D BUILD_TBB=ON \
      -D BUILD_TESTS=OFF \
      -D INSTALL_PYTHON_EXAMPLES=OFF \
      -D OPENCV_ENABLE_NONFREE=ON \
      -D BUILD_EXAMPLES=OFF \
      ~/opencv_build/opencv

# Build OpenCV
make -j$(nproc)

# Install OpenCV
make install

echo "OpenCV 4.5.5 installation completed successfully!"

