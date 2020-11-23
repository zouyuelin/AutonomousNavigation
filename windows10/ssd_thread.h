#ifndef SSD_THREAD_H
#define SSD_THREAD_H

#include <iostream>
#include "opencv2/dnn.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudawarping.hpp"
#include <QString>
#include <QtMath>
#include <QDebug>
#include <QThread>
#include <QTime>
#include <QObject>
#include "sock.h"
#include <QRect>
#include <vector>

using namespace std;
using namespace cv;


//-----------------------------------定义pmac线程----------------------
//---------------------------------定义目标检测线程-------------------------
class SSD_Detect:  public QThread
{
    Q_OBJECT
public:
    SSD_Detect(dnn::Net getnet,VideoCapture getcapture,bool isdetect=true,bool isGetRect=false);
    ~SSD_Detect() override;

    void stop(){}
    void run() override;
    //Mat mattopixmap;
    bool autoControl;
    sock *pmac;//基于套接字的pmac通讯
    bool quitTheThread;
    double command[2]={0};//send command
    Mat frame;
    bool _isGetRect;
    vector<QRect> qrect;
	int theHolesNumber;
    vector<Point2f> theTempCentre;
protected:
    void Detect();
    void pmacsendmassage();
    void objectPosition(vector<Point2f> &center_p);
    void  setBottle(vector<Point2f> &center_p,Mat &frame);
    double  distance_(Point2f point1,Point2f point2);
    void  getConfidenceCentre(vector< vector<Point2f> > tens,Mat &frame);
    void caclDistanceandTheta(vector<Point2f> theLastCentre, Mat &frame,int elemet=0);
    void threadSift();

signals:
    void setTable(Mat);
    void setOpenFrameReady();
    void setPmac();
    void getConnectReady(bool);
private slots:
    void sendConnectReady(bool);
private:
        dnn::Net net;
        VideoCapture capture;
        int width;
        int height;
        bool startdetect;
        vector< vector<Point2f> > tens;

        //-------------------------------全局变量的定义-------------------
        const int inwidth = 600;
        const int inheight = 600;
        const char* classNames[2] = { "holes","stone" };
        const bool get_val_mp4 = true;
};
#endif // SSD_THREAD_H
