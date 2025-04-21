cmake_minimum_required(VERSION 2.8)

project(AccountRegistrator)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_BUILD_TYPE "Debug")

add_executable(
    AccountRegistrator
    main.cpp
)

target_link_libraries(
    AccountRegistrator
    boost_system
    boost_thread
    boost_program_options
    pqxx
)

#
#
#
