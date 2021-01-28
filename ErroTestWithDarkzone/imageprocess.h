#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudawarping.hpp"
#include "opencv2/dnn.hpp"
#include<QThread>
#include<QString>
#include<string>
#include<QDebug>
#include<QApplication>
#include<vector>
#include<QtMath>
#include<QApplication>
#include<iostream>

class imgProcess:  public QThread
{
    Q_OBJECT
public:
    explicit imgProcess(bool dnn,std::string videoPath = NULL,std::string backgroundPath = NULL,QObject *parent = nullptr);
    ~imgProcess() override;
    void AllStop();
    void run() override;
    //std::shared_ptr<telnet> pmacConnect;

protected:
    void selectCentre(cv::Mat &frame,cv::Mat &raw);
    cv::Mat MoveDetect(cv::Mat frame,cv::Mat &frameRaw);
    cv::Mat PreProcess(cv::Mat frame,cv::Mat &raw);
    double inline distance_(cv::Point2d point1,cv::Point2d point2);
    //¥ÛΩÚÀ„∑®
    int OtsuAlgThreshold(cv::Mat image);
    void inline objectPosition(std::vector<cv::Point2d> &center_p,cv::Mat &frame);
    void inline caclDistanceAndTheta(std::vector<cv::Point2d> Centre,cv::Mat &frame,int elemet=0);
    void inline PreProcessForDnn(cv::Mat &frame);

private:
    QMutex m_lock;
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;
    cv::VideoCapture capture;
    std::string videopath;
    bool StartProcess;
    std::vector<std::vector<cv::Point> > contours;
    std::vector<std::vector<cv::Point> > Goodcontours;
    std::vector<cv::Point2d> centers;
    cv::Mat messageSend;
    int offset;
    cv::Mat backgroundGray;
    cv::dnn::Net net;

    cv::Point2d centerScreen;
    const int width = 600;
    const int height = 600;
    const bool useDnn;
signals:
    void SendThePosMessage(cv::Mat );

};

#endif // IMAGEPROCESS_H
