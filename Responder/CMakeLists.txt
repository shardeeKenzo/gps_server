cmake_minimum_required(VERSION 2.8)

project(GpsResponder)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_BUILD_TYPE "Release")

include_directories(common)
include_directories(Utils)
include_directories(Modules)
include_directories(SQL)
include_directories(Server)
include_directories(Parser)
include_directories(Parser/HTTP)

add_executable(
    GpsResponder

    main.cpp

    common/DataTypes.h
    common/ErrorCodes.h
    common/Globals.hpp

    Utils/utils.hpp

    Server/Server.h
    Server/Server.cpp
    Server/IOServicePool.h
    Server/IOServicePool.cpp
    Server/Connection.h
    Server/Connection.cpp

    Parser/Parser.hpp

    Parser/HTTP/HttpParser.h
    Parser/HTTP/HttpParser.cpp
    Parser/HTTP/Reply.h
    Parser/HTTP/Reply.cpp
    Parser/HTTP/UriParser.h
    Parser/HTTP/UriParser.cpp

    Modules/Authorization.h
    Modules/Authorization.cpp
    Modules/DataStorage.h
    Modules/DataStorage.cpp

    SQL/PSQLHandler.h
    SQL/PSQLHandler.cpp
)

target_link_libraries(
    GpsResponder
    pthread
    boost_system
    boost_thread
    boost_program_options
    boost_serialization
    pqxx
)

INSTALL(TARGETS GpsResponder
        RUNTIME DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_EXECUTE 
        OWNER_WRITE GROUP_READ GROUP_EXECUTE 
        WORLD_READ WORLD_EXECUTE
)