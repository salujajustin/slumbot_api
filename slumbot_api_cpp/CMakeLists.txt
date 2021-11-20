cmake_minimum_required(VERSION 3.5)
project(slumbot_api_cpp)

set(CMAKE_CXX_STANDARD 17)

include_directories(lib)

add_executable(sample_api sample_api.cpp)

