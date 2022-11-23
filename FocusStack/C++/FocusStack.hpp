//
// Created by avent on 10/25/22.
//

#ifndef FOCUSSTACK_FOCUSSTACK_HPP
#define FOCUSSTACK_FOCUSSTACK_HPP

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d/features2d.hpp> //特征提取&特征匹配库


using namespace std;
//using namespace cv;

class FocusStack
{
public:
    vector<cv::Mat> inputImgs;

    explicit FocusStack(const vector<cv::Mat> &imgList) :
            inputImgs(imgList), len_imgStack(imgList.size())
    {}

    void alignImgs();  //图像对齐

    static cv::Mat doLap(const cv::Mat &img);

    cv::Mat focusStack();

private:
    vector<cv::Mat> alignedImgs;
    uint32_t len_imgStack;

    /** 初始化SIFT特征检测器 **/
    cv::Ptr<cv::SIFT> detector = cv::SIFT::create();
    cv::Ptr<cv::DescriptorMatcher> matcher = cv::BFMatcher::create();
};

void FocusStack::alignImgs()
{
    alignedImgs.push_back(inputImgs[0]);

    int w = inputImgs[0].cols, h = inputImgs[0].rows;

    cv::Mat refeGray;
    cvtColor(inputImgs[0], refeGray, cv::COLOR_BGR2GRAY);

    /** 初始化参照图像的关键点和描述子 **/
    vector<cv::KeyPoint> keypoints_refe;
    cv::Mat descriptors_refe;

    /** 检测关键点位置并计算描述子**/
    detector->detectAndCompute(refeGray, cv::Mat(), keypoints_refe, descriptors_refe);


    for (int i = 1; i < len_imgStack; ++i)
    {
        int w_i = inputImgs[i].cols, h_i = inputImgs[i].rows;
        if (w_i != w || h_i != h)
        {
            cerr << "图片大小不一致！" << endl;
            return;
        }

        cv::Mat imgGray;
        cvtColor(inputImgs[i], imgGray, cv::COLOR_BGR2BGRA);

        vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        detector->detectAndCompute(imgGray, cv::Mat(), keypoints, descriptors);

        vector<vector<cv::DMatch>> rawMatches;
        matcher->knnMatch(descriptors_refe, descriptors, rawMatches, 2);
        vector<cv::DMatch> goodMatches;
        for (auto &rawMatche: rawMatches)
        {
            if (rawMatche[0].distance < 0.69 * rawMatche[1].distance)
            {
                goodMatches.push_back(rawMatche[0]);
            }
        }

        /** 根据distance对goodMatches进行排序 **/
        sort(goodMatches.begin(), goodMatches.end(),
             [](cv::DMatch &a, cv::DMatch &b)
             { return a.distance < b.distance; });

        /** 取goodMatches的一部分 **/
        vector<cv::DMatch> matches(goodMatches.begin(), goodMatches.begin() +
                                                        uint16_t(goodMatches.size() * 2 / 3));

        vector<cv::Point2f> pointsRefe;
        vector<cv::Point2f> points;
        for (const auto &match: matches)
        {
            points.push_back(keypoints[match.trainIdx].pt);
            pointsRefe.push_back(keypoints_refe[match.queryIdx].pt);
        }

        cv::Mat Homo = findHomography(points, pointsRefe, cv::RANSAC, 2.0);
        cv::Mat outcome;
        warpPerspective(inputImgs[i], outcome, Homo, inputImgs[i].size(), cv::INTER_LINEAR);
        alignedImgs.push_back(outcome);
    }
}

cv::Mat FocusStack::doLap(const cv::Mat &img)
{
    int kernel_size = 3, blur_size = 3;
    cv::Mat imgBlur;
    GaussianBlur(img, imgBlur, cv::Size(blur_size, blur_size), 0, 0);
    cv::Mat outcome;
    Laplacian(imgBlur, outcome, CV_32F, kernel_size);
    return outcome;
}

cv::Mat FocusStack::focusStack()
{
    alignImgs();

    int w = alignedImgs[0].cols, h = alignedImgs[0].rows;
    vector<cv::Mat> laps;
    cv::Mat imgGray;
    for (const auto &img: alignedImgs)
    {
        cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);
        laps.push_back(doLap(imgGray));
    }


    /** 对每张lap中的像素取绝对值 **/
    for (auto &lap: laps)
    {
        for (int i = 0; i < h; ++i)
        {
            auto* pixl = lap.ptr<float>(i);
            for (int j = 0; j < w; ++j)
            {
                pixl[j] = fabs(pixl[j]);  // 浮点型绝对值: fabs()
            }
        }
    }

    cv::Mat maxim = cv::Mat::zeros(cv::Size(laps[0].cols, laps[0].rows), laps[0].type());  //创建一个空白图像
    for (int i = 0; i < laps[0].rows; ++i)
    {
        for (int j = 0; j < laps[0].cols; ++j)
        {
            auto pixl = laps[0].at<float>(i, j);
            for (const auto &img: laps)
            {
                if (pixl < img.at<float>(i, j))
                {
                    pixl = img.at<float>(i, j);
                }
            }
            maxim.at<float>(i, j) = pixl;
        }
    }

    /** 根据maxim制作掩膜 **/
    vector<cv::Mat> maskList;
    for (auto &lap: laps)
    {
        cv::Mat mask = cv::Mat::zeros(cv::Size(w, h), CV_8UC1);
        for (int i = 0; i < h; ++i)
        {
            const auto pixl_lap = lap.ptr<float>(i);
            const auto pixl_maxim = maxim.ptr<float>(i);
            auto pixl_mask = mask.ptr<uchar>(i);

            for (int j = 0; j < w; ++j)
            {
                if (pixl_lap[j] == pixl_maxim[j])
                {
                    pixl_mask[j] = 1;
                }
                else
                {
                    pixl_mask[j] = 0;
                }
            }
        }
        maskList.push_back(mask);
    }

    /** 输出图片 **/
    cv::Mat output;
    for (int i = 0; i < len_imgStack; ++i)
    {
        bitwise_not(alignedImgs[i], output, maskList[i]);
    }

    bitwise_not(output, output);
    return output;
}

#endif //FOCUSSTACK_FOCUSSTACK_HPP
