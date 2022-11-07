cmake_minimum_required(VERSION 3.23)
project(FocusStack)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)
include_directories(OpenCV_INCLUDE_DIRS)

add_executable(main main.cpp FocusStack.hpp)
#add_executable(main trying.cpp)
target_link_libraries(main ${OpenCV_LIBS})

#add_executable(FocusStack FocusStack.hpp)
#target_link_libraries(FocusStack ${OpenCV_LIBS})