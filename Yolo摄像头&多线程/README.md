# 摄像头 Yolo 检测 & 多线程编程

使用**多线程**时，CmakeList 需要进行如下配置：

```cmake
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(main ${OpenCV_LIBS} Threads::Threads)
```



需要将以下三个文件下载好后，放进 model 文件夹

- coco.names
- yolov7.cfg
- yolov7.weights



