cmake_minimum_required(VERSION 2.8)
project(hc_client)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# Using restclient-cpp (https://github.com/mrtazz/restclient-cpp)
# Using rapidjson (https://github.com/Tencent/rapidjson)
include_directories(/home/tysik/Includes/)
link_directories(/home/tysik/Libs/)

add_executable(hc_client main.cpp include/device.hpp include/session.hpp)
target_link_libraries(hc_client curl restclient-cpp)
