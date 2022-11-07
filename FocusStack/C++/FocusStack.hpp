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
using namespace cv;

class FocusStack
{
public:
    vector<Mat> inputImgs;

    explicit FocusStack(const vector<Mat> &imgList) :
            inputImgs(imgList), len_imgStack(imgList.size())
    {}

    void alignImgs();  //图像对齐

    static Mat doLap(const Mat &img);

    Mat focusStack();

private:
    vector<Mat> alignedImgs;
    uint32_t len_imgStack;
};

void FocusStack::alignImgs()
{
    alignedImgs.push_back(inputImgs[0]);

    int w = inputImgs[0].cols, h = inputImgs[0].rows;

    Mat refeGray;
    cvtColor(inputImgs[0], refeGray, COLOR_BGR2GRAY);

    /** 初始化参照图像的关键点和描述子 **/
    vector<KeyPoint> keypoints_refe;
    Mat descriptors_refe;

    /** 初始化SIFT特征检测器 **/
    Ptr<FeatureDetector> detector = SIFT::create();
    Ptr<DescriptorExtractor> descriptor = SIFT::create();
    Ptr<DescriptorMatcher> matcher = BFMatcher::create();

    /** 检测关键点位置 **/
    detector->detect(refeGray, keypoints_refe);

    /** 根据关键点计算描述子 **/
    descriptor->compute(refeGray, keypoints_refe, descriptors_refe);

    for (int i = 1; i < len_imgStack; ++i)
    {
        int w_i = inputImgs[i].cols, h_i = inputImgs[i].rows;
        if (w_i != w || h_i != h)
        {
            cerr << "图片大小不一致！" << endl;
            return;
        }

        Mat imgGray;
        cvtColor(inputImgs[i], imgGray, COLOR_BGR2BGRA);

        vector<KeyPoint> keypoints;
        Mat descriptors;

        /** 检测关键点位置 **/
        detector->detect(imgGray, keypoints);
        /** 根据关键点计算描述子 **/
        descriptor->compute(imgGray, keypoints, descriptors);

        vector<vector<DMatch>> rawMatches;
        matcher->knnMatch(descriptors_refe, descriptors, rawMatches, 2);
        vector<DMatch> goodMatches;
        for (auto &rawMatche: rawMatches)
        {
            if (rawMatche[0].distance < 0.69 * rawMatche[1].distance)
            {
                goodMatches.push_back(rawMatche[0]);
            }
        }

        /** 根据distance对goodMatches进行排序 **/
        sort(goodMatches.begin(), goodMatches.end(),
             [](DMatch &a, DMatch &b)
             { return a.distance < b.distance; });

        /** 取goodMatches的一部分 **/
        vector<DMatch> matches(goodMatches.begin(), goodMatches.begin() +
                                                    uint16_t(goodMatches.size() * 2 / 3));
        
        vector<Point2f> pointsRefe;
        vector<Point2f> points;
        for (const auto &match: matches)
        {
            points.push_back(keypoints[match.trainIdx].pt);
            pointsRefe.push_back(keypoints_refe[match.queryIdx].pt);
        }

        Mat Homo = findHomography(points, pointsRefe, RANSAC, 2.0);
        Mat outcome;
        warpPerspective(inputImgs[i], outcome, Homo, inputImgs[i].size(), INTER_LINEAR);
        alignedImgs.push_back(outcome);
    }
}

Mat FocusStack::doLap(const Mat &img)
{
    int kernel_size = 3, blur_size = 3;
    Mat imgBlur;
    GaussianBlur(img, imgBlur, Size(blur_size, blur_size), 0, 0);
    Mat outcome;
    Laplacian(imgBlur, outcome, CV_32F, kernel_size);
    return outcome;
}

Mat FocusStack::focusStack()
{
    alignImgs();

    int w = alignedImgs[0].cols, h = alignedImgs[0].rows;
    vector<Mat> laps;
    Mat imgGray;
    for (const auto &img: alignedImgs)
    {
        cvtColor(img, imgGray, COLOR_BGR2GRAY);
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

    Mat maxim = Mat::zeros(Size(laps[0].cols, laps[0].rows), laps[0].type());  //创建一个空白图像
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
    vector<Mat> maskList;
    for (auto &lap: laps)
    {
        Mat mask = Mat::zeros(Size(w, h), CV_8UC1);
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
    Mat output;
    for (int i = 0; i < len_imgStack; ++i)
    {
        bitwise_not(alignedImgs[i], output, maskList[i]);
    }

    bitwise_not(output, output);
    return output;
}

#endif //FOCUSSTACK_FOCUSSTACK_HPP
