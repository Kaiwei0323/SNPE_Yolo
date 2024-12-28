#include <iostream>
#include <fstream>
#if defined(WIN32) || defined(_WIN32)
#include <time.h>
#include <gettimeofday.h>
#else
#include <sys/time.h>
#endif

#include "VideoAnalyzer.h"

static runtime_t device2runtime(std::string&& device)
{
    std::transform(device.begin(), device.end(), device.begin(),
        [](unsigned char ch){ return tolower(ch); });

    if (0 == device.compare("cpu")) {
        return CPU;
    } else if (0 == device.compare("gpu")) {
        return GPU;
    } else if (0 == device.compare("gpu_float16")) {
        return GPU_FLOAT16;
    } else if (0 == device.compare("dsp")) {
        return DSP;
    } else if (0 == device.compare("aip")) {
        return AIP;
    } else { 
        return CPU;
    }
}

void VideoAnalyzer::InferenceFrame()
{
    std::shared_ptr<cv::Mat> image;
    cv::Mat output_image;
    while (isRunning) {
        Json::Value root;
        for (auto& [k, v] : detectors) {
            std::vector<yolo::ObjectData> results;
            consumeQueue->consumption(image);
            v->Detect(*image.get(), results);
            
            output_image = *image.get();
            
            LOG_INFO("Model Name: {}, Detect {} Items", k, results.size());
            for (auto& result : results) {
                if (result.confidence >= thresholds[k][result.label]) {
                    LOG_INFO("[x: {}, y: {}, width: {}, height: {}, confidence: {}, label: {}, time cost: {}]",
                        result.bbox.x,
                        result.bbox.y,
                        result.bbox.width,
                        result.bbox.height,
                        result.confidence,
                        result.label,
                        result.time_cost);
                    cv::rectangle(output_image, cv::Rect(result.bbox.x, result.bbox.y, result.bbox.width, result.bbox.height), cv::Scalar(0, 255, 0), 3);
                    cv::Point position = cv::Point(result.bbox.x, result.bbox.y - 10);
                    cv::putText(output_image, labels[k][result.label], position, cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0, 255, 0), 2, 0.3);
                    

                    
                    Json::Value object;
                    object["bbox"]["x"] = result.bbox.x;
                    object["bbox"]["y"] = result.bbox.y;
                    object["bbox"]["width"] = result.bbox.width;
                    object["bbox"]["height"] = result.bbox.height;
                    object["confidence"] = result.confidence;
                    object["label"] = labels[k][result.label];
                    object["model"] = k;
                    root["results"].append(object);
                }
            }
            output_productQueue->product(std::make_shared<cv::Mat>(output_image));   
        }
    }
}

void VideoAnalyzer::OutputFrame()
{
    int i = 0;
    std::shared_ptr<cv::Mat> frame;
    
    // Create a window to display the video
    cv::namedWindow("Live Video", cv::WINDOW_AUTOSIZE);
    
    while (isRunning) {
        output_productQueue->consumption(frame);
        if (frame) {
            cv::Mat output_frame = *frame.get();
        
            // Display the frame in the window
            cv::imshow("Live Video", output_frame);
            /*
            std::string output_path = "./test_img/object_detection_result_" + std::to_string(i) + ".jpg";
            cv::imwrite(output_path, output_frame);
            i++;
            */
        }
        // Wait for a short time to process window events
        int key = cv::waitKey(30); // 30ms delay
        if (key == 27) { // ESC key to exit
            break;
        }
    }
}

void VideoAnalyzer::ParseConfig(Json::Value& root, yolo::ObjectDetectionConfig& config)
{
    config.model_path = root["model-path"].asString();
    config.runtime = device2runtime(root["runtime"].asString());
    config.labels = root["labels"].asInt();
    config.grids = root["grids"].asInt();
    if (root["input-layers"].isArray()) {
        int sz = root["input-layers"].size();
        for (int i = 0; i < sz; ++i)
            config.inputLayers.push_back(root["input-layers"][i].asString());
    }
    if (root["output-layers"].isArray()) {
        int sz = root["output-layers"].size();
        for (int i = 0; i < sz; ++i)
            config.outputLayers.push_back(root["output-layers"][i].asString());
    }
    if (root["output-tensors"].isArray()) {
        int sz = root["output-tensors"].size();
        for (int i = 0; i < sz; ++i)
            config.outputTensors.push_back(root["output-tensors"][i].asString());
    }
}

VideoAnalyzer::VideoAnalyzer()
{
    isRunning = true;
}

VideoAnalyzer::~VideoAnalyzer()
{
    DeInit();
}

bool VideoAnalyzer::Init(Json::Value& model)
{
    if (model.isArray()) {
        int sz = model.size();
        for (int i = 0; i < sz; ++i) {
            std::string modelName = model[i]["model-name"].asString();
            std::ifstream in(model[i]["label-path"].asString());
            std::string line;
            std::vector<std::string> label;
            while (getline(in, line)){
                label.push_back(line);
            }
            this->labels[modelName] = label;
            in.close();

            in.open(model[i]["threshold-path"].asString());
            std::vector<float> threshold;
            while (getline(in, line)){
                threshold.push_back(std::stof(line));
            }
            this->thresholds[modelName] = threshold;
            in.close();

            std::shared_ptr<yolo::ObjectDetection> detector = std::shared_ptr<yolo::ObjectDetection>(new yolo::ObjectDetection());
            yolo::ObjectDetectionConfig config;
            ParseConfig(model[i], config);
            detector->Init(config);
            detector->SetScoreThreshold(model[i]["global-threshold"].asFloat(), 0.5);
            this->detectors[modelName] = detector;
        }
    }
    return true;
}

bool VideoAnalyzer::DeInit()
{    
    isRunning = false;

    if (inferThread) {
        inferThread->join();
        inferThread = nullptr;
    }
    return true;
}

bool VideoAnalyzer::Start()
{

    isRunning = true;
    if (!(inferThread = std::make_shared<std::thread>(std::bind(&VideoAnalyzer::InferenceFrame, this)))) {
        LOG_ERROR("Failed to new a std::thread object");
        isRunning = false;
        return false;
    }
    return true;
}

void VideoAnalyzer::SetUserData(std::shared_ptr<SafetyQueue<cv::Mat>> user_data)
{
    consumeQueue = user_data;
    output_productQueue = std::make_shared<SafetyQueue<cv::Mat>>();
}
