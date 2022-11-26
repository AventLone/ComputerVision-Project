//
// Created by avent on 11/25/22.
//

#ifndef YOLO_TASKS_HPP
#define YOLO_TASKS_HPP

#include "yolo.hpp"
#include <thread>
#include <atomic>

class Tasks
{
public:
    explicit Tasks();

    ~Tasks();

    void taskInit();

    void startTasks();

private:
    YoloDetector* detector;
    cv::VideoCapture cap;
    cv::Mat frame;

    std::atomic<bool> isOpen;   //通过这个布尔值控制任务中的循环
    std::thread thread_tasks[2];   //创建两个线程

    void task1_getImgsFromCamera();

    void task2_doYoloAndView();

};

Tasks::Tasks() : isOpen(true)
{
    std::string config = "../model/yolov7.cfg";
    std::string weights = "../model/yolov7.weights";
    std::string labels = "../model/coco.names";
    detector = new YoloDetector(labels, config, weights);
}

Tasks::~Tasks()
{
    thread_tasks[0].join();
    thread_tasks[1].join();
    delete detector;
}

void Tasks::taskInit()
{
    /** 设置相机捕获画面尺寸大小 **/
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 800);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 600);

    /** 曝光   min:-8  max:0 **/
    //  cap.set(cv::CAP_PROP_EXPOSURE, (-4));

    /** 帧数   30 60 **/
    cap.set(cv::CAP_PROP_FPS, 30);

    cap.open(0);
    if (!cap.isOpened())
    {
        std::cerr << "摄像头打开失败！" << std::endl;
        abort();
    }
}

void Tasks::task1_getImgsFromCamera()
{
    while (isOpen)
    {
        cap >> frame;
        cap >> frame;
        if (frame.empty())
            return;
    }
    cap.release();
}

void Tasks::task2_doYoloAndView()
{
    cv::namedWindow("Yolo Viewer", cv::WINDOW_NORMAL);
    cv::resizeWindow("Yolo Viewer", 800, 600);
    while (isOpen)
    {
        if (!frame.empty())
        {
            while (isOpen)
            {
                detector->runModel(frame);
                cv::imshow("Yolo Viewer", frame);
                cv::waitKey(1);
                if (cv::waitKey(30) >= 0)
                {
                    isOpen = false;
                    break;
                }
            }
        }
    }
}

void Tasks::startTasks()
{
    thread_tasks[0] = std::thread(&Tasks::task1_getImgsFromCamera, this);
    thread_tasks[1] = std::thread(&Tasks::task2_doYoloAndView, this);
}


#endif //YOLO_TASKS_HPP
