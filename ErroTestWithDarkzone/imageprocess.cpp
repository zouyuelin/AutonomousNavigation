#include "imageprocess.h"
imgProcess::imgProcess(std::string videoPath,std::string backgroundPath,QObject *parent):cameraMatrix(cv::Mat::zeros(3,3,CV_64F)),
    distCoeffs(cv::Mat::zeros(1,5,CV_64F)),StartProcess(true),offset(-100),messageSend(cv::Mat::zeros(1,3,CV_64F))
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
            qDebug()<<"Open the camera/video successful\n";
        }
    else {
            qDebug()<<"Open the camera/video faild\n";
            QApplication* app = nullptr;
            app->exit(0);
        }
    backgroundGray = cv::imread(backgroundPath,cv::IMREAD_GRAYSCALE);
    cv::medianBlur(backgroundGray,backgroundGray,5);
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
                frame = MoveDetect(frame,frameRaw);
                selectCentre(frame,frameRaw);
                emit SendThePosMessage(messageSend);

                meter.stop();
                cv::putText(frameRaw, cv::format("FPS: %.2f ; Time: %.2f ms",
                                          1000.f / meter.getTimeMilli(),
                                          meter.getTimeMilli()),cv::Point(10, 20), 0, 0.5,
                                          cv::Scalar(0, 0, 255),1);
                cv::imshow("video",frameRaw);
                cv::waitKey(33);
            }
    }

}

cv::Mat imgProcess::MoveDetect(cv::Mat frame,cv::Mat &frameRaw)
{
    cv::Mat diff;
    cv::absdiff(backgroundGray, frame, diff);

    int thred = OtsuAlgThreshold(frame);
    threshold(diff,
              diff,
              55,
              255,
              cv::THRESH_BINARY_INV);//cv::THRESH_BINARY_INV cv::THRESH_OTSU 60

    cv::copyMakeBorder(diff,diff,10,10,10,10,cv::BORDER_CONSTANT,cv::Scalar(255,255,255));
    cv::copyMakeBorder(frameRaw,frameRaw,10,10,10,10,cv::BORDER_REPLICATE);


    cv::Mat kernel_dilate = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(5,5));
    cv::Mat kernel_erode = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(5,5));

    /*----先腐蚀后膨胀: 开运算（morph_open）
        消除小物体,在纤细点分离物体，在平滑处不明显改变面积
      ----先膨胀后腐蚀: 闭运算（morph_close）
        消除小黑点
      ----膨胀图与腐蚀图之差: 形态学梯度（gradient）
        保留和突出边缘
      ----闭运算结果与原图作差: 顶帽（morph_tophat）
        分离比临近点亮的斑块（较大斑块）
      ----原图与开运算结果作差: 黑帽（morph_blacktop）
        分离比临近点暗的斑块
    */
    //高级形态学 处理
    //cv::morphologyEx(diff,diff,cv::MORPH_OPEN,kernel);

    //腐蚀
    cv::erode(diff,diff,kernel_erode);
    //膨胀
    cv::dilate(diff,diff,kernel_dilate);

    cv::imshow("diff",diff);
    return diff;
}

void imgProcess::selectCentre(cv::Mat &frame,cv::Mat &raw)
{
    //大津算法,阈值分割
    //int thred = OtsuAlgThreshold(frame);
    //threshold(frame,frame,thred,255,cv::THRESH_BINARY_INV);//cv::THRESH_BINARY_INV cv::THRESH_OTSU

    //清除一遍
    centers.clear();
    Goodcontours.clear();
    contours.clear();

    findContours(frame,contours,cv::noArray(),cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
    //cvtColor(raw,raw,cv::COLOR_GRAY2BGR);

    //画轮廓
    for(int i=0;i<contours.size();i++)
    {
        double area;

        area=contourArea(contours[i]);
        if(area<5000||area>(height*width*0.25))
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
        //显示误差erro
        cv::putText(raw,std::to_string(distance_(center,centerScreen)),cv::Point(center.x+10,center.y+10),0,0.5,
                    cv::Scalar(0,255,255),2);
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
    caclDistanceAndTheta(centers,raw);
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
    cv::medianBlur(frameRepat,frameRepat,5);
    //GaussianBlur(frameRepat,frameRepat,cv::Size(5,5),0,0);

    //灰度处理
    cv::cuda::GpuMat Mat2(frameRepat);
    cv::cuda::GpuMat gpu_frame_2;
    cv::cuda::cvtColor(Mat2,gpu_frame,cv::COLOR_BGR2GRAY);
    gpu_frame.download(frameRepat);

    //cvtColor(frameRepat,frameRepat,cv::COLOR_BGR2GRAY);

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
            distance.push_back(distance_(centerScreen,Centre[i])/424.2);

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
        messageSend.at<double>(0,0) = theta[elemet];
        //发送孔中心距视觉中心距离，已经映射0~530.3像素值到0~1；
        messageSend.at<double>(0,1) = distance[elemet];
    }
}

int imgProcess::OtsuAlgThreshold(cv::Mat image)
{
        int T = 0; //Otsu算法阈值
        double varValue = 0; //类间方差中间值保存
        double w0 = 0; //前景像素点数所占比例
        double w1 = 0; //背景像素点数所占比例
        double u0 = 0; //前景平均灰度
        double u1 = 0; //背景平均灰度
        double Histogram[256] = { 0 }; //灰度直方图，下标是灰度值，保存内容是灰度值对应的像素点总数
        uchar *data = image.data;

        double totalNum = image.rows*image.cols; //像素总数

        for (int i = 0; i < image.rows; i++)
        {
            for (int j = 0; j < image.cols; j++)
            {
                if (image.at<uchar>(i, j) != 0) Histogram[data[i*image.step + j]]++;
            }
        }
        int minpos, maxpos;
        for (int i = 0; i < 255; i++)
        {
            if (Histogram[i] != 0)
            {
                minpos = i;
                break;
            }
        }
        for (int i = 255; i > 0; i--)
        {
            if (Histogram[i] != 0)
            {
                maxpos = i;
                break;
            }
        }

        for (int i = minpos; i <= maxpos; i++)
        {
            //每次遍历之前初始化各变量
            w1 = 0;       u1 = 0;       w0 = 0;       u0 = 0;
            //***********背景各分量值计算**************************
            for (int j = 0; j <= i; j++) //背景部分各值计算
            {
                w1 += Histogram[j];   //背景部分像素点总数
                u1 += j*Histogram[j]; //背景部分像素总灰度和
            }
            if (w1 == 0) //背景部分像素点数为0时退出
            {
                break;
            }
            u1 = u1 / w1; //背景像素平均灰度
            w1 = w1 / totalNum; // 背景部分像素点数所占比例
            //***********背景各分量值计算**************************

            //***********前景各分量值计算**************************
            for (int k = i + 1; k < 255; k++)
            {
                w0 += Histogram[k];  //前景部分像素点总数
                u0 += k*Histogram[k]; //前景部分像素总灰度和
            }
            if (w0 == 0) //前景部分像素点数为0时退出
            {
                break;
            }
            u0 = u0 / w0; //前景像素平均灰度
            w0 = w0 / totalNum; // 前景部分像素点数所占比例
            //***********前景各分量值计算**************************

            //***********类间方差计算******************************
            double varValueI = w0*w1*(u1 - u0)*(u1 - u0); //当前类间方差计算
            if (varValue < varValueI)
            {
                varValue = varValueI;
                T = i;
            }
        }
        T+=offset;
        return T;
}
