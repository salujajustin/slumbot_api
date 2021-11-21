cmake_minimum_required(VERSION 3.5)
project(slumbot_api_cpp)

set(CMAKE_CXX_STANDARD 17)

include_directories(lib)

find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS serialization filesystem property_tree)

add_executable(sample_api sample_api.cpp)
target_link_libraries(sample_api OpenSSL::SSL)
target_link_libraries(sample_api OpenSSL::Crypto)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(sample_api ${Boost_LIBRARIES})
endif()

