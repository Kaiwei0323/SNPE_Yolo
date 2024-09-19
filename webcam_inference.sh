#!/bin/bash

set -e

# Navigate into the 'build' directory
cd build

# Run the executable with the specified config
echo "Running the application..."
./detect/webcam_detect/webcam-detect --config_path ../detect/webcam_detect/config.json

echo "Execution completed successfully!"
