cmake_minimum_required(VERSION 3.31)
project(POV_Converter)

set(CMAKE_CXX_STANDARD 20)

set(OpenCV_DIR "C:/tools/opencv/vs-x64-build/install")

set(ENV{PATH} "$ENV{PATH};C:/tools/opencv/vs-x64-build/install/x64/vc17/bin")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(POV_Converter main.cpp
        POV_Converter.cpp
        POV_Converter.h)
target_link_libraries(POV_Converter ${OpenCV_LIBS})
