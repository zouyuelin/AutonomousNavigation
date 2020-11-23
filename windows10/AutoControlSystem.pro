#-------------------------------------------------
#
# Project created by QtCreator 2020-07-08T08:36:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AutoControlSystem
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

SOURCES += main.cpp mainwindow.cpp sock.cpp ssd_thread.cpp

HEADERS += mainwindow.h sock.h ssd_thread.h

FORMS += mainwindow.ui
LIBS += -lws2_32
QT += network
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#将opencv库添加的两种方式
#INCLUDEPATH += "/usr/local/include/opencv4"
#LIBS += -L /usr/local/lib/\
#            -lopencv_imgproc\
#            -lopencv_dnn\
#            -lopencv_cudawarping\
#            -lopencv_cudaimgproc\
#            -lopencv_highgui\
#            -lopencv_core\
#            -lopencv_imgcodecs\
#            -lopencv_videoio
LIBS +=E:/OpenCV/opencv/build-4.3/x64/vc14/lib/opencv_*.lib
INCLUDEPATH +=E:/OpenCV/opencv/build-4.3/include

#include("E:/OpenCV/opencv-gpu4.3.0.pri")

RESOURCES +=  image.qrc
