cmake_minimum_required(VERSION 3.5.1)
set(project_name AutoControlSystem)
project(${project_name})

# 设置自动生成moc文件,AUTOMOC打开可以省去QT5_WRAP_CPP命令
set(CMAKE_AUTOMOC ON)

# 设置自动生成ui.h文件,AUTOUIC打开可以省去QT5_WRAP_UI命令
set(CMAKE_AUTOUIC ON)

# 设置自动生成ui.h文件,AUTOUIC打开可以省去QT5_ADD_RESOURCES命令
set(CMAKE_AUTORCC ON)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(QT_USE_QTOPENGL TRUE)
# -----------------
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -Wall")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#find the opencv and the qt5
find_package(OpenCV 4.3.0  REQUIRED)
find_package(Qt5 COMPONENTS Widgets Core Gui Network REQUIRED)
find_package(Boost REQUIRED)

set(auto_SOURCES 
	main.cpp
	mainwindow.cpp
	sock.cpp
	ssd_thread.cpp)

set(auto_HEADERS
	mainwindow.h
	sock.h
	ssd_thread.h)

set(auto_RESOURCES
	image.qrc)

set(auto_FORMS
	mainwindow.ui)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${BOOST_INCLUDE_DIRS})

#生成可执行文件 
ADD_EXECUTABLE(${project_name} 
	${auto_SOURCES} 
	${auto_HEADERS} 
	${auto_RESOURCES} 
	${auto_FORMS})

target_link_libraries(${project_name} ${Boost_LIBRARIES} ${OpenCV_LIBS} Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Network)

message(STATUS "opencv version: ${OpenCV_VERSION}")
message(STATUS "opencv lib: ${OpenCV_LIBS}")
message(STATUS "opencv include dir: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "opencv lib dir: ${OpenCV_INSTALL_PATH}")
message(STATUS "opencv config path : ${OpenCV_CONFIG_PATH}")
message(STATUS "the boost is ${Boost_INCLUDE_DIRS}")
