#pragma once

#include <thread>
#include <memory>
#include <vector>
#include <functional>
#include <string>
#include <map>

#include <opencv2/opencv.hpp>
#include <jsoncpp/json/json.h>

#include "YOLO.h"
#include "YOLOImpl.h"
#include "utils.h"
#include "SafeQueue.h"

class VideoAnalyzer {
public:
    VideoAnalyzer();
    ~VideoAnalyzer();
    bool Init(Json::Value& model);
    bool DeInit();
    bool Start();
    void SetUserData(std::shared_ptr<SafetyQueue<cv::Mat>> user_data);
    void OutputFrame();

private:
    void ParseConfig(Json::Value& root, yolo::ObjectDetectionConfig& config);

private:
    bool isRunning;
    void InferenceFrame();
    
    std::shared_ptr<std::thread> inferThread;
    std::shared_ptr<std::thread> outputThread;

    std::unordered_map<std::string, std::shared_ptr<yolo::ObjectDetection>> detectors;
    std::unordered_map<std::string, std::vector<std::string>> labels;
    std::unordered_map<std::string, std::vector<float>> thresholds;
    std::shared_ptr<SafetyQueue<cv::Mat>> consumeQueue;
    std::shared_ptr<SafetyQueue<cv::Mat>> output_productQueue;
};
