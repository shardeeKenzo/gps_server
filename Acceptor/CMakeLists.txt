cmake_minimum_required(VERSION 2.8)

project(GpsAcceptor)

set(CMAKE_CXX_FLAGS "-std=c++17 -g" )
set(CMAKE_BUILD_TYPE "Debuge")

include_directories(Data_modal)
include_directories(Utils)
include_directories(Modules)
include_directories(SQL)
include_directories(Server)
include_directories(Parser)
include_directories(Parser/WialonIPS)
include_directories(Parser/WialonIPS/PackageType)

add_executable(test_teltonica Parser/Teltonica/Teltonica.cpp Parser/Teltonica/testTeltonica.cpp Parser/Teltonica/Teltonica.h)

add_executable(
    GpsAcceptor
    
    main.cpp
    
    Server/Server.cpp
    Server/IOServicePool.cpp
    Server/Connection.cpp
    
    Parser/WialonIPS/WialonIPS.cpp
    
    Parser/WialonIPS/PackageType/PingParser.cpp
    Parser/WialonIPS/PackageType/LoginParser.cpp
    Parser/WialonIPS/PackageType/DataParser.cpp
    Parser/WialonIPS/PackageType/BlackBoxParser.cpp
    
    Modules/Authorization.cpp
    Modules/DataStorage.cpp
    SQL/PSQLHandler.cpp
        Parser/Teltonica/Teltonica.h Parser/Teltonica/Teltonica.cpp)
 
target_link_libraries(
    GpsAcceptor pthread boost_system boost_thread boost_log boost_log_setup boost_program_options pqxx
)

INSTALL(TARGETS GpsAcceptor
        RUNTIME DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_EXECUTE 
        OWNER_WRITE GROUP_READ GROUP_EXECUTE 
        WORLD_READ WORLD_EXECUTE
)