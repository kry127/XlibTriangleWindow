cmake_minimum_required(VERSION 3.6)
project(xlib_example)

set(CMAKE_CXX_STANDARD 11)  # for c++11

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

set(SOURCE_FILES main.cpp)
add_executable(xlib_example ${SOURCE_FILES})
