cmake_minimum_required(VERSION 3.7)
project(socket_stream)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp processMsg.cpp processMsg.h)
add_executable(socket_stream ${SOURCE_FILES})