#include "detectimage.h"

Mat imfill(Mat cop,int n)
{
    Mat data = ~cop;
    Mat labels, stats, centroids;
    connectedComponentsWithStats(data, labels, stats, centroids, 4, CV_16U);
    int regions_count = stats.rows - 1;
    int regions_size, regions_x1, regions_y1, regions_x2, regions_y2;

    for (int i = 1; i <= regions_count; i++)
    {
        regions_size = stats.ptr<int>(i)[4];
        if (regions_size < n)
        {
            regions_x1 = stats.ptr<int>(i)[0];
            regions_y1 = stats.ptr<int>(i)[1];
            regions_x2 = regions_x1 + stats.ptr<int>(i)[2];
            regions_y2 = regions_y1 + stats.ptr<int>(i)[3];

            for (int j = regions_y1; j < regions_y2; j++)
            {
                for (int k = regions_x1; k < regions_x2; k++)
                {
                    if (labels.ptr<ushort>(j)[k] == i)
                        data.ptr<uchar>(j)[k] = 0;
                }
            }
        }
    }
    data = ~data;
    return data;
}

void RemoveSmallRegion(Mat &Src, Mat &Dst, int AreaLimit, int CheckMode, int NeihborMode)
{
    int RemoveCount = 0;
    //新建一幅标签图像初始化为0像素点，为了记录每个像素点检验状态的标签，0代表未检查，1代表正在检查,2代表检查不合格（需要反转颜色），3代表检查合格或不需检查
    //初始化的图像全部为0，未检查
    Mat PointLabel = Mat::zeros(Src.size(), CV_8UC1);
    if (CheckMode == 1)//去除小连通区域的白色点
    {
        //cout << "去除小连通域.";
        for (int i = 0; i < Src.rows; i++)
        {
            for (int j = 0; j < Src.cols; j++)
            {
                if (Src.at<uchar>(i, j) < 10)
                {
                    PointLabel.at<uchar>(i, j) = 3;//将背景黑色点标记为合格，像素为3
                }
            }
        }
    }
    else//去除孔洞，黑色点像素
    {
        //cout << "去除孔洞";
        for (int i = 0; i < Src.rows; i++)
        {
            for (int j = 0; j < Src.cols; j++)
            {
                if (Src.at<uchar>(i, j) > 10)
                {
                    PointLabel.at<uchar>(i, j) = 3;//如果原图是白色区域，标记为合格，像素为3
                }
            }
        }
    }


    vector<Point2i>NeihborPos;//将邻域压进容器
    NeihborPos.push_back(Point2i(-1, 0));
    NeihborPos.push_back(Point2i(1, 0));
    NeihborPos.push_back(Point2i(0, -1));
    NeihborPos.push_back(Point2i(0, 1));
    if (NeihborMode == 1)
    {
        //cout << "Neighbor mode: 8邻域." << endl;
        NeihborPos.push_back(Point2i(-1, -1));
        NeihborPos.push_back(Point2i(-1, 1));
        NeihborPos.push_back(Point2i(1, -1));
        NeihborPos.push_back(Point2i(1, 1));
    }
    else int a = 0;//cout << "Neighbor mode: 4邻域." << endl;
    int NeihborCount = 4 + 4 * NeihborMode;
    int CurrX = 0, CurrY = 0;
    //开始检测
    for (int i = 0; i < Src.rows; i++)
    {
        for (int j = 0; j < Src.cols; j++)
        {
            if (PointLabel.at<uchar>(i, j) == 0)//标签图像像素点为0，表示还未检查的不合格点
            {   //开始检查
                vector<Point2i>GrowBuffer;//记录检查像素点的个数
                GrowBuffer.push_back(Point2i(j, i));
                PointLabel.at<uchar>(i, j) = 1;//标记为正在检查
                int CheckResult = 0;

                for (int z = 0; z < GrowBuffer.size(); z++)
                {
                    for (int q = 0; q < NeihborCount; q++)
                    {
                        CurrX = GrowBuffer.at(z).x + NeihborPos.at(q).x;
                        CurrY = GrowBuffer.at(z).y + NeihborPos.at(q).y;
                        if (CurrX >= 0 && CurrX<Src.cols&&CurrY >= 0 && CurrY<Src.rows)  //防止越界
                        {
                            if (PointLabel.at<uchar>(CurrY, CurrX) == 0)
                            {
                                GrowBuffer.push_back(Point2i(CurrX, CurrY));  //邻域点加入buffer
                                PointLabel.at<uchar>(CurrY, CurrX) = 1;           //更新邻域点的检查标签，避免重复检查
                            }
                        }
                    }
                }
                if (GrowBuffer.size()>AreaLimit) //判断结果（是否超出限定的大小），1为未超出，2为超出
                    CheckResult = 2;
                else
                {
                    CheckResult = 1;
                    RemoveCount++;//记录有多少区域被去除
                }

                for (int z = 0; z < GrowBuffer.size(); z++)
                {
                    CurrX = GrowBuffer.at(z).x;
                    CurrY = GrowBuffer.at(z).y;
                    PointLabel.at<uchar>(CurrY, CurrX) += CheckResult;//标记不合格的像素点，像素值为2
                }
                //********结束该点处的检查**********
            }
        }
    }
    CheckMode = 255 * (1 - CheckMode);
    //开始反转面积过小的区域
    for (int i = 0; i < Src.rows; ++i)
    {
        for (int j = 0; j < Src.cols; ++j)
        {
            if (PointLabel.at<uchar>(i, j) == 2)
            {
                Dst.at<uchar>(i, j) = CheckMode;
            }
            else if (PointLabel.at<uchar>(i, j) == 3)
            {
                Dst.at<uchar>(i, j) = Src.at<uchar>(i, j);

            }
        }
    }
}

void DetectImage(Mat img, string imgPath)
{
    // Convert to graycsale
    Mat img_gray;
    cvtColor(img, img_gray, COLOR_BGR2GRAY);

    Mat img_copy = img_gray.clone();

    // Blur the image for better edge detection
    Mat img_blur;
    blur(img_copy, img_blur, Size(24,24));

    for(int i = 0;i < img.rows; ++i)
    {
        for(int j = 0; j < img.cols; ++j)
        {
            if(img_blur.at<uchar>(i,j) > 64)
            {
                img_blur.at<uchar>(i,j) = 255;
            }
        }
    }

    //    imshow("11",img_blur);
    //    waitKey(0);
    // Canny edge detection
    Mat edges;  //轮廓
    Mat imgRemove;

    imgRemove=imfill(img_blur,1000);


    // Remove small spot
    threshold(imgRemove, imgRemove, 128, 255, THRESH_BINARY_INV);
    RemoveSmallRegion(imgRemove, imgRemove, 1000, 0, 1);

    Canny(imgRemove,edges,75,200,3,false);

//    imshow("Image",img_blur);
//    waitKey(0);


    Mat srcImg = edges.clone();
    Mat dstImg = img.clone();

    vector<vector<Point>> contours; //储存图片中每一个轮廓
    vector<Vec4i> hierarcy;

    findContours(srcImg, contours, hierarcy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    Point2f center;  //定义圆中心坐标
    float radius;  //定义圆半径
    for(int i = 0; i < contours.size(); ++i)  //依次遍历每个轮廓
    {
        minEnclosingCircle(Mat(contours[i]), center, radius);
        drawContours(dstImg, contours, i, Scalar(0, 0, 255), 2, 8);
        circle(dstImg, center, radius, Scalar(0, 255, 0), 2, 8);  //绘制第i个轮廓的最小外接圆
    }

    imgPath.erase(imgPath.size()-4, 4); //去掉原文件名的后缀
    imwrite(imgPath + "_detected.jpg", dstImg);

    imshow("dst", dstImg);
    waitKey(0);
}
