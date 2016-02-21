cmake_minimum_required(VERSION 2.8)
project(multithread_server)

# include_directories(
#  dir1/src
#  dir2/includes:wq
#)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -Wall")

set(SRC_LIST main.cpp)

add_executable(final ${SRC_LIST})


