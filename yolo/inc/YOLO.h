#ifndef __YOLO_H__
#define __YOLO_H__

#include <vector>
#include <string>
#include <functional>
#include <memory>

#include <opencv2/opencv.hpp>

#include "utils.h"

namespace yolo
{


struct ObjectData {
    // Bounding box information: top-left coordinate and width, height
    cv::Rect bbox;
    // Confidence of this bounding box
    float confidence = -1.0f;
    // The label of this Bounding box
    int label = -1;
    // Time cost of detecting this frame
    int64_t time_cost = 0;
};


struct ObjectDetectionConfig {
    std::string model_path;
    runtime_t runtime;
    int labels;
    int grids;
    std::vector<std::string> inputLayers;
    std::vector<std::string> outputLayers;
    std::vector<std::string> outputTensors;
};


typedef std::function<bool(const cv::Mat& frame)> pre_process_t;
typedef std::function<bool(std::vector<ObjectData>& results)> post_process_t;


class EXPORT_API ObjectDetection {
public:

    ObjectDetection();

    ~ObjectDetection();

    
    bool Init(const ObjectDetectionConfig& config);

    bool Deinit();

    bool SetScoreThreshold(const float& conf_thresh, const float& nms_thresh);
   
    bool SetROI(const cv::Rect& roi);

    bool RegisterPreProcess(pre_process_t func);

    bool RegisterPreProcess(post_process_t func);

    bool Detect(const cv::Mat& image, std::vector<ObjectData>& results);

    bool IsInitialized();

private:
    void* impl = nullptr;
};

} // namespace yolo


#endif // __YOLO_H__
