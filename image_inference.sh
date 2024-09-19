#!/bin/bash

set -e

# Navigate into the 'build' directory
cd build

# Run the executable with the specified config
echo "Running the application..."
./detect/image_detect/image-detect --input ../detect/image_detect/people.jpg --labels ../model/coco80_labels.txt --config_path ../detect/image_detect/yolov8_config.json



echo "Execution completed successfully!"
