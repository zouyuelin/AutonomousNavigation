#cmake -G"Visual Studio 15 2017 Win64"  ..
#cmake ..

cmake_minimum_required(VERSION 2.8)

project(ErroTestWithDarkzone)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_C_COMPILER "clang")
    link_libraries(ws2_32 wsock32)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CMAKE_C_COMPILER "clang") #gcc
    set(CMAKE_CXX_COMPILER "clang++") #g++
    set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -Wall")
ENDIF()

# -----------------
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#find the opencv and the qt5
find_package(OpenCV 4.3.0  REQUIRED)
find_package(Qt5 COMPONENTS Widgets Core Gui Network REQUIRED)

set(auto_SOURCES
        main.cpp
        telnet.cpp
        pmac.cpp
        imageprocess.cpp)

set(auto_HEADERS
    telnet.h
    pmac.h
    imageprocess.h
                )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

#生成可执行文件
#add_library(${PROJECT_NAME} xxx.cpp)
ADD_EXECUTABLE(${PROJECT_NAME}
        ${auto_SOURCES}
        ${auto_HEADERS} )

target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Network)

message(STATUS "opencv version: ${OpenCV_VERSION}")
message(STATUS "opencv lib: ${OpenCV_LIBS}")
message(STATUS "opencv include dir: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "opencv lib dir: ${OpenCV_INSTALL_PATH}")
message(STATUS "opencv config path : ${OpenCV_CONFIG_PATH}")
