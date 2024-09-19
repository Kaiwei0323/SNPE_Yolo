# SNPE YOLOv5 Setup Guide

## Prerequisites

### Hardware Platform:
- **QCS6490**
  - CPU: Octa-Core Kryo 670 
  - GPU: Qualcomm Adreno 643

### Operating System:
- **Ubuntu 20.04**

### SNPE SDK Version:
- **v2.22.6.240515**

### Development Framework:
- **OpenCV 4.5.5**

### Supported Models:
- YOLOv5, YOLOv8

### Dependencies:
- gflags
- json-glib-1.0
- glib-2.0
- spdlog-1.10.0
- jsoncpp-1.7.4
- fmt
- spdlog

---

## SNPE SDK Installation

Download the SNPE SDK from [Qualcomm SNPE SDK](https://www.qualcomm.com/developer/software/neural-processing-sdk-for-ai).

### Directory Structure
```
Documents
└── SNPE_Yolo
└── v2.22.6.240515
```

## Setup Steps
1. Switch to Admin Mode
```
su
Password: oelinux123
```

2. Navigate to Project Directory
```
cd SNPE_Yolo
```

3. Install OpenCV
```
./install_opencv.sh
```

4. Install Dependencies
```
./install_dependencies.sh
```

## Model Setup
1. Prepare Your Model
* Place your custom YOLO model (.dlc) in the model/ folder
* Create a label list (e.g., model/coco80_labels.txt) and put it in the same folder

2. Modify CMake Configuration
Edit CMakeLists.txt to set the paths for OpenCV and SNPE libraries:
```
set(OpenCV_DIR "/usr/local/include/opencv4")
set(SNPE_INCLUDE_DIR /home/aim/Documents/v2.22.6.240515/qairt/2.22.6.240515/include/SNPE)
set(SNPE_LIBRARY_DIR /home/aim/Documents/v2.22.6.240515/qairt/2.22.6.240515/lib/aarch64-ubuntu-gcc9.4)
```

3. Build the Application
```
./build_app.sh
```
## Model Information from Visualization (Example: YOLOv8s)
```
|    237      |   /model.22/Concat_5   | ... | output0  |
...
| Input Name  | Dimensions   | Type    | Encoding Info  |
|-------------|--------------|---------|----------------|
| images      | 1,640,640,3  | uFxp_8  | ...            |

| Output Name | Dimensions   | Type    | Encoding Info  |
|-------------|--------------|---------|----------------|
| output0     | 1,84,8400    | uFxp_8  | ...            |
```
* Input Layer Name: images
* Output Layer Name: /model.22/Concat_5
* Output Tensor Name: output0

## Video Inference
1. Edit Video Configuration
Modify the detect/video_detect/config.json file:
```
{
  "pipeline-config": {
    "camera-url": "file:///home/aim/Documents/SNPE_Yolo/demo_video/parkinglot.mp4",    // "select your video file or rtsp camera stream url"
    "stream-width": 1920,
    "stream-height": 1080,
    ...
  },
  "model-name": "yolov8s",    // "define your own model name"
  "model-path": "../model/yolov8s_quantized.dlc",    // "model location"
  "label-path": "../model/coco80_labels.txt",    // "model labels"
  "threshold-path": "../detect/video_detect/yolo_thresholds.txt",    // "threshold for each class"
  "runtime": "GPU",    // "select runtime (support runtime: CPU, GPU (recommended), GPU_FLOAT16, DSP_FIXED8)"
  "labels": 84,    // "class number + 4 for yolov8, class number + 5 for yolov5"
  "grids": 8400,    // "8400 for yolov8, 25200 for yolov5"
  "input-layers": [
    "images"    // "input layer name"
  ],
  "output-layers": [
    "/model.22/Concat_5"    // "output layer name"
  ],
  "output-tensors": [
    "output0"    // "output tensor name"
  ],
  "global-threshold": 0.2
}
```
2. Run Video Inference
``` 
./video_inference.sh
```

## Webcam Inference
1. Edit Webcam Configuration
Modify the detect/webcam_detect/config.json file:
```
{
  "model-name": "yolov8s",    // "define your own model name"
  "model-path": "../model/yolov8s_quantized.dlc",    // "model location"
  "label-path": "../model/coco80_labels.txt",    // "model labels location"
  "threshold-path": "../detect/video_detect/yolo_thresholds.txt",    // "threshold for each class"
  "runtime": "GPU",    // "select runtime (support runtime: CPU, GPU (recommended), GPU_FLOAT16, DSP_FIXED8)"
  "labels": 84,    // "class number + 4 for yolov8, class number + 5 for yolov5"
  "grids": 8400,    // "8400 for yolov8, 25200 for yolov5"
  "input-layers": [
    "images"    // "input layer name"
  ],
  "output-layers": [
    "/model.22/Concat_5"    // "output layer name"
  ],
  "output-tensors": [
    "output0"    // "output tensor name"
  ],
  "global-threshold": 0.2
}
```
2. Run Webcam Inference
```
./webcam_inference.sh
```

## Code Explanation
* snpetask/SNPETask.cpp: Handles the setup of SNPE runtime and creates user buffers for model input and output data
* yolo/src/YOLOImpl.cpp: Responsible for image preprocessing before inference and postprocessing after inference to extract detection results
* detect/video_detect/VideoPipeline.cpp: Contains the implementation of the GStreamer pipeline for video streaming
