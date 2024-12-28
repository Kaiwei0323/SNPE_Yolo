#include <math.h>
#include <algorithm>

#include <opencv2/opencv.hpp>

#include "YOLOImpl.h"

namespace yolo {

ObjectDetectionImpl::ObjectDetectionImpl() : m_task(nullptr) {

}

ObjectDetectionImpl::~ObjectDetectionImpl() {
    DeInitialize();
}

bool ObjectDetectionImpl::Initialize(const ObjectDetectionConfig& config)
{
    m_task = std::move(std::unique_ptr<snpetask::SNPETask>(new snpetask::SNPETask()));

    m_inputLayers = config.inputLayers;
    m_outputLayers = config.outputLayers;
    m_outputTensors = config.outputTensors;
    m_labels = config.labels;
    m_grids = config.grids;

    m_task->setOutputLayers(m_outputLayers);

    if (!m_task->init(config.model_path, config.runtime)) {
        LOG_ERROR("Can't init snpetask instance.");
        return false;
    }

    m_output = new float[m_grids * m_labels];

    m_isInit = true;
    return true;
}

bool ObjectDetectionImpl::DeInitialize()
{
    if (m_task) {
        m_task->deInit();
        m_task.reset(nullptr);
    }

    if (m_output) {
        delete[] m_output;
        m_output = nullptr;
    }

    m_isInit = false;
    return true;
}

bool ObjectDetectionImpl::PreProcess(const cv::Mat& image)
{
    auto inputShape = m_task->getInputShape(m_inputLayers[0]);

    size_t batch = inputShape[0];
    size_t inputHeight = inputShape[1];
    size_t inputWidth = inputShape[2];
    size_t channel = inputShape[3];

    if (m_task->getInputTensor(m_inputLayers[0]) == nullptr) {
        LOG_ERROR("Empty input tensor");
        return false;
    }

    cv::Mat input(inputHeight, inputWidth, CV_32FC3, m_task->getInputTensor(m_inputLayers[0]));

    if (image.empty()) {
        LOG_ERROR("Invalid image!");
        return false;
    }

    int imgWidth = image.cols;
    int imgHeight = image.rows;

    m_scale = std::min(inputHeight /(float)imgHeight, inputWidth / (float)imgWidth);
    int scaledWidth = imgWidth * m_scale;
    int scaledHeight = imgHeight * m_scale;
    m_xOffset = (inputWidth - scaledWidth) / 2;
    m_yOffset = (inputHeight - scaledHeight) / 2;

    cv::Mat inputMat(inputHeight, inputWidth, CV_8UC3, cv::Scalar(128, 128, 128));
    cv::Mat roiMat(inputMat, cv::Rect(m_xOffset, m_yOffset, scaledWidth, scaledHeight));
    cv::resize(image, roiMat, cv::Size(scaledWidth, scaledHeight), cv::INTER_LINEAR);

    inputMat.convertTo(input, CV_32FC3);
    input /= 255.0f;
    return true;
}

bool ObjectDetectionImpl::Detect(const cv::Mat& image,
    std::vector<ObjectData>& results)
{

    if (m_roi.empty()) {
        if (m_isRegisteredPreProcess) m_preProcess(image); 
        else PreProcess(image);
    } else {
        auto roi_image = image(m_roi);
        if (m_isRegisteredPreProcess) m_preProcess(roi_image); 
        else PreProcess(roi_image);
    }

    int64_t start = GetTimeStamp_ms();
    if (!m_task->execute()) {
        LOG_ERROR("SNPETask execute failed.");
        return false;
    }

    if (m_isRegisteredPostProcess) m_postProcess(results);
    else PostProcess(results, GetTimeStamp_ms() - start);

    return true;
}

bool ObjectDetectionImpl::PostProcess(std::vector<ObjectData> &results, int64_t time)
{  
    float* tmpOutput = m_output;
    auto outputShape = m_task->getOutputShape(m_outputTensors[0]);
    const float *predOutput = m_task->getOutputTensor(m_outputTensors[0]);
    
    std::vector<int> boxIndexs;
    std::vector<float> boxConfidences;
    std::vector<ObjectData> winList;
    
    bool yolov8 = false;
    
    if(m_grids == 8400) {
        yolov8 = true;
    }
    
    if(yolov8) {
        for(int i = 0; i < m_grids; i++) {
            for(int j = 0; j < m_labels; j++) {
                m_output[i * m_labels + j] = predOutput[j * m_grids + i];
    	    }
        }
    } else {
        for(int i = 0; i < m_grids; i++) {
            for(int j = 0; j < m_labels; j++) {
                m_output[i * m_labels + j] = predOutput[i * m_labels + j];
            }
        }
    }
       

    float curMaxScore = 0.0f;
    float curMinScore = 0.0f;
    
    for (int i = 0; i < m_grids; i++) {
        int curIdx = i;
        if(yolov8) {
            int max_idx = 4;
            for (int j = 5; j < m_labels; j++) {    // [prob0, prob1,..., prob84]
                if (m_output[curIdx * m_labels + j] > m_output[curIdx * m_labels + max_idx]) max_idx = j;
            }

            float score = m_output[curIdx * m_labels + max_idx];
            if (score > m_confThresh) {
                ObjectData rect;
                rect.bbox.width = m_output[curIdx * m_labels + 2];
                rect.bbox.height = m_output[curIdx * m_labels + 3];
                rect.bbox.x = std::max(0, static_cast<int>(m_output[curIdx * m_labels] - rect.bbox.width / 2)) - m_xOffset;
                rect.bbox.y = std::max(0, static_cast<int>(m_output[curIdx * m_labels + 1] - rect.bbox.height / 2)) - m_yOffset;

                rect.bbox.width /= m_scale;
                rect.bbox.height /= m_scale;
                rect.bbox.x /= m_scale;
                rect.bbox.y /= m_scale;
                rect.confidence = score;
                rect.label = max_idx - 4;
                rect.time_cost = time;
                winList.push_back(rect);
            }
        } else {
            int max_idx = 5;
            float curBoxConfidence = m_output[curIdx * m_labels + 4]; 
            for (int j = 6; j < m_labels; j++) {    // [prob0, prob1,..., prob84]
                if (m_output[curIdx * m_labels + j] > m_output[curIdx * m_labels + max_idx]) max_idx = j;
            }

            float score = curBoxConfidence * m_output[curIdx * m_labels + max_idx];
            if (score > m_confThresh) {
                ObjectData rect;
                rect.bbox.width = m_output[curIdx * m_labels + 2];
                rect.bbox.height = m_output[curIdx * m_labels + 3];
                rect.bbox.x = std::max(0, static_cast<int>(m_output[curIdx * m_labels] - rect.bbox.width / 2)) - m_xOffset;
                rect.bbox.y = std::max(0, static_cast<int>(m_output[curIdx * m_labels + 1] - rect.bbox.height / 2)) - m_yOffset;

                rect.bbox.width /= m_scale;
                rect.bbox.height /= m_scale;
                rect.bbox.x /= m_scale;
                rect.bbox.y /= m_scale;
                rect.confidence = score;
                rect.label = max_idx - 5;
                rect.time_cost = time;
                winList.push_back(rect);
            }
        }
    }
    

    winList = nms(winList, m_nmsThresh);

    for (size_t i = 0; i < winList.size(); i++) {
        if (winList[i].bbox.width >= m_minBoxBorder || winList[i].bbox.height >= m_minBoxBorder) {
            if (!m_roi.empty()) {
                winList[i].bbox.x += m_roi.x;
                winList[i].bbox.y += m_roi.y;
            }
            results.push_back(winList[i]);
        }
    }

    return true;
}

} // namespace yolo
