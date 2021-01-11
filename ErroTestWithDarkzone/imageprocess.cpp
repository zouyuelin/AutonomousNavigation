#include "imageprocess.h"
imgProcess::imgProcess(std::string videoPath,QObject *parent):cameraMatrix(cv::Mat::zeros(3,3,CV_64F)),
    distCoeffs(cv::Mat::zeros(1,5,CV_64F)),StartProcess(true)
{
    centerScreen = cv::Point2d(height/2,width/2);

    //相机内参
    cameraMatrix.at<double>(0,0) = 399.23;
    cameraMatrix.at<double>(1,1) = 396.99;
    cameraMatrix.at<double>(0,2) = 301.175;
    cameraMatrix.at<double>(1,2) = 306.12;
    cameraMatrix.at<double>(2,2) = 1.00;

    distCoeffs.at<double>(0,0) = -0.0876;
    distCoeffs.at<double>(0,1) = -0.1972;
    distCoeffs.at<double>(0,4) = 0.1358;

    //相机路径
    videopath = videoPath;
    if(videopath.empty())
    {
        capture = cv::VideoCapture(0);
        qDebug()<<"use camera\n";
    }
    else
        capture = cv::VideoCapture(videopath);

    std::cout<<"The width is: "<<capture.get(cv::CAP_PROP_FRAME_WIDTH)<<std::endl;//1920*1080
    std::cout<<"The height is: "<<capture.get(cv::CAP_PROP_FRAME_HEIGHT)<<std::endl;
    capture.set(cv::CAP_PROP_POS_FRAMES,0.65);

    //是否打开成功
    if (capture.isOpened())
        {
            qDebug()<<"open the file successful\n";
        }
    else {
            qDebug()<<"open the file faild\n";
            QApplication* app = nullptr;
            app->exit(0);
        }
}

imgProcess::~imgProcess()
{
    this->requestInterruption();
    this->quit();
    this->wait();
}

void imgProcess::AllStop()
{
    QMutexLocker locker(&m_lock);
    StartProcess = false;
    this->terminate();
    this->wait();
}

void imgProcess::run()
{
    //--------------处理识别图像数据-----------------
    std::cout<<"img process start\n";
    while(true)
    {
            while(capture.isOpened() && StartProcess)
            {
                cv::Mat frame;
                cv::Mat frameRaw;
                //计时
                cv::TickMeter meter;
                meter.start();
                capture>>frame;

                //退出条件
                if (frame.empty())
                {
                    capture.release();
                    break;
                }

                frame = PreProcess(frame,frameRaw);
                selectCentre(frame,frameRaw);
                emit SendThePosMessage(messageSend);
                meter.stop();
                cv::imshow("video",frameRaw);
                cv::waitKey(33);
            }
    }
}

void imgProcess::selectCentre(cv::Mat &frame,cv::Mat &raw)
{
    //清除一遍
    centers.clear();
    Goodcontours.clear();
    contours.clear();

    findContours(frame,contours,cv::noArray(),cv::RETR_CCOMP,cv::CHAIN_APPROX_SIMPLE);
    //cvtColor(raw,raw,cv::COLOR_GRAY2BGR);

    //画轮廓
    for(int i=0;i<contours.size();i++)
    {
        double area;

        area=contourArea(contours[i]);
        if(area<8000||area>(height*width*0.3))
        {
            continue;
        }
        Goodcontours.push_back(contours[i]);

    }

    for(int i=0;i<Goodcontours.size();i++)
    {
        //计算质心
        cv::Moments mom;
        cv::Point2d center;
        mom = moments(Goodcontours[i]);
        center.x = (int)(mom.m10/mom.m00);
        center.y = (int)(mom.m01/mom.m00);
        centers.push_back(center);
        circle(raw,center,2,cv::Scalar(255,0,0),2);

        drawContours(raw,
                 Goodcontours,
                 i,
                 cv::Scalar(0,0,255),
                 3);
    }
    cv::circle(raw,centerScreen,2,cv::Scalar(0,255,0),2);
    //画轮廓,可以使用for auto迭代器
    //for(auto a:b)中b为一个容器，效果是利用a遍历并获得b容器中的每一个值，但是a无法影响到b容器中的元素
    //for(auto &a:b)中加了引用符号，可以对容器中的内容进行赋值，即可通过对a赋值来做到容器b的内容填充
    caclDistanceAndTheta(centers,frame);
}

cv::Mat imgProcess::PreProcess(cv::Mat frame,cv::Mat &raw)
{
    //裁剪
    cv::Rect rect(frame.cols/2-355,frame.rows/2-355,750,750);
    frame=frame(rect);

    //cuda resize
    cv::cuda::GpuMat Mat1(frame);
    cv::cuda::GpuMat gpu_frame;
    cv::cuda::resize(Mat1,gpu_frame,cv::Size(width,height));
    gpu_frame.download(frame);

    //cv::resize(frame,frame,cv::Size(width,height));

    //repat the image-消除畸变-
    cv::Mat frameRepat;
    undistort(frame ,frameRepat ,cameraMatrix,distCoeffs);
    raw = frameRepat.clone();

    //滤波
    GaussianBlur(frameRepat,frameRepat,cv::Size(5,5),0,0);

    //灰度处理
    cv::cuda::GpuMat Mat2(frameRepat);
    cv::cuda::GpuMat gpu_frame_2;
    cv::cuda::cvtColor(Mat2,gpu_frame,cv::COLOR_BGR2GRAY);
    gpu_frame.download(frameRepat);

    //cvtColor(frameRepat,frameRepat,cv::COLOR_BGR2GRAY);

    //大津算法
    threshold(frameRepat,frameRepat,110,255,cv::THRESH_OTSU);//cv::THRESH_BINARY_INV

    return frameRepat;
}

inline double imgProcess::distance_(cv::Point2d point1,cv::Point2d point2)
{
     return sqrt(pow(point1.x-point2.x,2)+pow(point1.y-point2.y,2));
}

inline void imgProcess::caclDistanceAndTheta(std::vector<cv::Point2d> Centre,cv::Mat &frame,int elemet)
{
    std::vector<double> theta;
    std::vector<double> distance;
    for(int i = 0;i<Centre.size();i++)
    {
            double K =(centerScreen.y-Centre[i].y)/(centerScreen.x-Centre[i].x);
            double theta_val = 0.0;
            if(centerScreen.y>Centre[i].y&&centerScreen.x<Centre[i].x)
            theta_val = qAtan(-K);
            else if (centerScreen.y>Centre[i].y&&centerScreen.x>Centre[i].x) {
            theta_val =qAtan(K)+1.57;
            }
            else if (centerScreen.y<Centre[i].y&&centerScreen.x>Centre[i].x) {
            theta_val =qAtan(-K)+3.14;
            }
            else if (centerScreen.y<Centre[i].y&&centerScreen.x<Centre[i].x) {
            theta_val =6.28-qAtan(K);
            }
            //save the theta(angle)
            theta.push_back(theta_val/6.28);
            distance.push_back(distance_(centerScreen,Centre[i])/530.3);

            //cast the data to the powerpmac
            theta[i] = theta[i];
            distance[i] = distance[i]/1.F;
    }
    //telnet send data
    if(distance.size()!=0)
    {
        //draw the camera centre to the object centre with a green line
        for(auto iter:Centre)
        {
            cv::line(frame,iter,centerScreen,cv::Scalar(255,255,0),2);
        }

        //发送偏转角度，已经映射0~360°到0~1；
        messageSend[0] = theta[elemet];
        //发送孔中心距视觉中心距离，已经映射0~530.3像素值到0~1；
        messageSend[1] = distance[elemet];
    }
}
