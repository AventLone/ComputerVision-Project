//
// Created by avent on 11/14/22.
//

#ifndef YOLO_YOLO_HPP
#define YOLO_YOLO_HPP

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <utility>
#include <fstream>

class YoloDetector
{
public:
    std::vector<std::string> classes;

    explicit YoloDetector(const std::string &classFiles, const std::string &configuration, const std::string &weights);

    void runModel(cv::Mat &img);

private:
    float confThreshold = 0.5; // Confidence threshold
    float nmsThreshold = 0.4;  // Non-maximum suppression threshold
    int inpWidth = 416;  // Width of network's input image
    int inpHeight = 416; // Height of network's input image
    cv::dnn::Net net;

    // Remove the bounding boxes with low confidence using non-maxima suppression
    void postprocess(cv::Mat &frame, const std::vector<cv::Mat> &outs);

    std::vector<std::string> getOutputsNames();

    // Draw the predicted bounding box
    void drawPred(int classId, float confidence, cv::Rect box, cv::Mat &frame);
};

YoloDetector::YoloDetector(const std::string &classFiles, const std::string &configuration, const std::string &weights)
{
    std::ifstream ifs(classFiles.c_str());
    std::string line;
    while (getline(ifs, line))
    {
        classes.push_back(line);
    }
    net = cv::dnn::readNetFromDarknet(configuration, weights);
    //使用CPU
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    //下面两行在使用CUDA检测时使用
    //  net.setPreferableBackend(dnn::DNN_BACKEND_CUDA);
    // net.setPreferableTarget(dnn::DNN_TARGET_CUDA);
}

void YoloDetector::drawPred(int classId, float confidence, cv::Rect box, cv::Mat &frame)
{
    int top = box.y, bottom = box.y + box.height, left = box.x, right = box.x + box.width;
    //Draw startTask1 rectangle displaying the bounding box
    rectangle(frame, cv::Point(left, top), cv::Point(right, bottom),
              cv::Scalar(0, 255, 0), 1);

    //Get the label for the class name and its confidence
    std::string conf_label = cv::format("%.2f", confidence);
    std::string label;
    if (!classes.empty())
    {
        label = classes[classId] + ":" + conf_label;
    }

    //Display the label at the top of the bounding box
    int baseLine;
    cv::Size labelSize = getTextSize(label, cv::FONT_HERSHEY_SIMPLEX,
                                     0.5, 1, &baseLine);
    top = std::max(top, labelSize.height);
    rectangle(frame, cv::Point(left, top - labelSize.height),
              cv::Point(left + labelSize.width, top + baseLine),
              cv::Scalar(255, 255, 255), cv::FILLED);
    putText(frame, label, cv::Point(left, top),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0),
            1, cv::LINE_AA);
}

void YoloDetector::postprocess(cv::Mat &frame, const std::vector<cv::Mat> &outs)
{
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (const auto &out: outs)
    {
        // Scan through all the bounding boxes output from the network and keep only the
        // ones with high confidence scores. Assign the box's class label as the class
        // with the highest score for the box.
        auto* data = (float*) out.data;
        for (int j = 0; j < out.rows; ++j, data += out.cols)
        {
            cv::Mat scores = out.row(j).colRange(5, out.cols);
            cv::Point classIdPoint;
            double confidence;
            // Get the value and location of the maximum score
            minMaxLoc(scores, nullptr, &confidence, nullptr, &classIdPoint);
            if (confidence > confThreshold)
            {
                int centerX = (int) (data[0] * frame.cols);
                int centerY = (int) (data[1] * frame.rows);
                int width = (int) (data[2] * frame.cols);
                int height = (int) (data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float) confidence);
                boxes.emplace_back(left, top, width, height);
            }
        }
    }

    // Perform non maximum suppression to eliminate redundant overlapping boxes with
    // lower confidences
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for (int idx: indices)
    {
        cv::Rect box = boxes[idx];
        drawPred(classIds[idx], confidences[idx], box, frame);
    }
}


std::vector<std::string> YoloDetector::getOutputsNames()
{
    std::vector<std::string> names;

    //Get the indices of the output layers, i.e. the layers with unconnected outputs
    std::vector<int> outLayers = net.getUnconnectedOutLayers();

    //get the names of all the layers in the network
    std::vector<std::string> layersNames = net.getLayerNames();

    // Get the names of the output layers in names
    names.resize(outLayers.size());
    for (uint32_t i = 0; i < outLayers.size(); ++i)
    {
        names[i] = layersNames[outLayers[i] - 1];
    }

    return names;
}

void YoloDetector::runModel(cv::Mat &img)
{
    cv::Mat blob;
    cv::dnn::blobFromImage(img, blob, 1 / 255.0, cv::Size(inpWidth, inpHeight),
                           cv::Scalar(0, 0, 0), true, false);

    net.setInput(blob);
    std::vector<cv::Mat> outs;
    net.forward(outs, getOutputsNames());

    // Remove the bounding boxes with low confidence
    postprocess(img, outs);
}

#endif //YOLO_YOLO_HPP
