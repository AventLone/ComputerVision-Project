cmake_minimum_required(VERSION 3.23)
project(yolo)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)
include_directories(OpenCV_INCLUDE_DIRS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


add_executable(main main.cpp yolo.hpp tasks.hpp)

target_link_libraries(main
        ${OpenCV_LIBS}
        Threads::Threads)