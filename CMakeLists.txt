cmake_minimum_required(VERSION 2.8)
project(hc_client)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED system)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(hc_client hc_client.cpp include/device.hpp include/session.hpp)
target_link_libraries(hc_client curl restclient-cpp pthread ${Boost_LIBRARIES})

add_executable(hc_client_test test/test.cpp)
target_link_libraries(hc_client_test gtest_main gmock_main pthread)
