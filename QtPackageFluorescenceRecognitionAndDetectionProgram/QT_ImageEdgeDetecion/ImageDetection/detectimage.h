#ifndef DETECTIMAGE_H
#define DETECTIMAGE_H

#include "vector"
#include "string"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

Mat imfill(Mat cop,int n);

void RemoveSmallRegion(Mat &Src, Mat &Dst, int AreaLimit, int CheckMode, int NeihborMode);

void DetectImage(Mat img, string imgPath);

#endif // DETECTIMAGE_H
