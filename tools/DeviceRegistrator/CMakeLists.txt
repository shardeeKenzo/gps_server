cmake_minimum_required(VERSION 2.8)

project(DeviceRegistrator)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_BUILD_TYPE "Debug")

add_executable(
    DeviceRegistrator
    main.cpp
)

target_link_libraries(
    DeviceRegistrator
    boost_system
    boost_thread
    boost_program_options
    pqxx
)

#
#
#
