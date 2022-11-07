//
// Created by avent on 10/27/22.
//
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int arg, char** argv)
{
    Mat img_1 = Mat::zeros(Size(100, 200), CV_8UC1);
    Mat img_2 = img_1.clone();
    Mat img_3 = img_1.clone();


    int w = img_1.cols, h = img_1.rows;

    uchar* data = img_2.data;
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            uchar* pixel = data + i * img_2.step;
            pixel[j] = 255;
        }
    }

    for (int i = 0; i < h; i++)
    {
        auto pixel = img_3.ptr<uchar>(i);
        for (int j = 0; j < w; j++)
        {

            pixel[j] = 255;
        }
    }

    imshow("01", img_1);
    imshow("02", img_2);
    imshow("03", img_3);

    waitKey(0);
//    cout << w << " " << h << endl;
    return 0;
}