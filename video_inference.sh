#!/bin/bash

set -e

# Navigate into the 'build' directory
cd build

# Run the executable with the specified config
echo "Running the application..."
./detect/video_detect/video-detect --config_path ../detect/video_detect/config.json

echo "Execution completed successfully!"
