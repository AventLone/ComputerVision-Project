#include "FocusStack.hpp"

using namespace cv;

int main()
{
    const vector<Mat> imgList = {imread("../Images/step0.jpg"), imread("../Images/step1.jpg"),
                                 imread("../Images/step2.jpg"), imread("../Images/step3.jpg"),
                                 imread("../Images/step4.jpg"), imread("../Images/step5.jpg")};

//    const vector<Mat> imgList = {imread("../Images/001.png"), imread("../Images/002.png")};
    FocusStack Imgs(imgList);
//    Imgs.alignImgs();

    Mat outcome = Imgs.focusStack();

    imshow("Outcome", outcome);
    waitKey(0);
    return 0;
}
