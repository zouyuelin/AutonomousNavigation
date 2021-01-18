#include "imageprocess.h"
imgProcess::imgProcess(std::string videoPath,std::string backgroundPath,QObject *parent):cameraMatrix(cv::Mat::zeros(3,3,CV_64F)),
    distCoeffs(cv::Mat::zeros(1,5,CV_64F)),StartProcess(true),offset(-100),messageSend(cv::Mat::zeros(1,3,CV_64F))
{
    centerScreen = cv::Point2d(height/2,width/2);

    //����ڲ�
    cameraMatrix.at<double>(0,0) = 399.23;
    cameraMatrix.at<double>(1,1) = 396.99;
    cameraMatrix.at<double>(0,2) = 301.175;
    cameraMatrix.at<double>(1,2) = 306.12;
    cameraMatrix.at<double>(2,2) = 1.00;

    distCoeffs.at<double>(0,0) = -0.0876;
    distCoeffs.at<double>(0,1) = -0.1972;
    distCoeffs.at<double>(0,4) = 0.1358;

    //���·��
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

    //�Ƿ�򿪳ɹ�
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
    //--------------����ʶ��ͼ������-----------------
    std::cout<<"img process start\n";
    while(true)
    {
            while(capture.isOpened() && StartProcess)
            {
                cv::Mat frame;
                cv::Mat frameRaw;
                //��ʱ
                cv::TickMeter meter;
                meter.start();
                capture>>frame;

                //�˳�����
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

    /*----�ȸ�ʴ������: �����㣨morph_open��
        ����С����,����ϸ��������壬��ƽ���������Ըı����
      ----�����ͺ�ʴ: �����㣨morph_close��
        ����С�ڵ�
      ----����ͼ�븯ʴͼ֮��: ��̬ѧ�ݶȣ�gradient��
        ������ͻ����Ե
      ----����������ԭͼ����: ��ñ��morph_tophat��
        ������ٽ������İ߿飨�ϴ�߿飩
      ----ԭͼ�뿪����������: ��ñ��morph_blacktop��
        ������ٽ��㰵�İ߿�
    */
    //�߼���̬ѧ ����
    //cv::morphologyEx(diff,diff,cv::MORPH_OPEN,kernel);

    //��ʴ
    cv::erode(diff,diff,kernel_erode);
    //����
    cv::dilate(diff,diff,kernel_dilate);

    cv::imshow("diff",diff);
    return diff;
}

void imgProcess::selectCentre(cv::Mat &frame,cv::Mat &raw)
{
    //����㷨,��ֵ�ָ�
    //int thred = OtsuAlgThreshold(frame);
    //threshold(frame,frame,thred,255,cv::THRESH_BINARY_INV);//cv::THRESH_BINARY_INV cv::THRESH_OTSU

    //���һ��
    centers.clear();
    Goodcontours.clear();
    contours.clear();

    findContours(frame,contours,cv::noArray(),cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
    //cvtColor(raw,raw,cv::COLOR_GRAY2BGR);

    //������
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
        //��������
        cv::Moments mom;
        cv::Point2d center;
        mom = moments(Goodcontours[i]);
        center.x = (int)(mom.m10/mom.m00);
        center.y = (int)(mom.m01/mom.m00);
        centers.push_back(center);
        circle(raw,center,2,cv::Scalar(255,0,0),2);
        //��ʾ���erro
        cv::putText(raw,std::to_string(distance_(center,centerScreen)),cv::Point(center.x+10,center.y+10),0,0.5,
                    cv::Scalar(0,255,255),2);
        drawContours(raw,
                 Goodcontours,
                 i,
                 cv::Scalar(0,0,255),
                 3);
    }
    cv::circle(raw,centerScreen,2,cv::Scalar(0,255,0),2);
    //������,����ʹ��for auto������
    //for(auto a:b)��bΪһ��������Ч��������a���������b�����е�ÿһ��ֵ������a�޷�Ӱ�쵽b�����е�Ԫ��
    //for(auto &a:b)�м������÷��ţ����Զ������е����ݽ��и�ֵ������ͨ����a��ֵ����������b���������
    caclDistanceAndTheta(centers,raw);
}

cv::Mat imgProcess::PreProcess(cv::Mat frame,cv::Mat &raw)
{
    //�ü�
    cv::Rect rect(frame.cols/2-355,frame.rows/2-355,750,750);
    frame=frame(rect);

    //cuda resize
    cv::cuda::GpuMat Mat1(frame);
    cv::cuda::GpuMat gpu_frame;
    cv::cuda::resize(Mat1,gpu_frame,cv::Size(width,height));
    gpu_frame.download(frame);

    //cv::resize(frame,frame,cv::Size(width,height));

    //repat the image-��������-
    cv::Mat frameRepat;
    undistort(frame ,frameRepat ,cameraMatrix,distCoeffs);
    raw = frameRepat.clone();

    //�˲�
    cv::medianBlur(frameRepat,frameRepat,5);
    //GaussianBlur(frameRepat,frameRepat,cv::Size(5,5),0,0);

    //�Ҷȴ���
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

        //����ƫת�Ƕȣ��Ѿ�ӳ��0~360�㵽0~1��
        messageSend.at<double>(0,0) = theta[elemet];
        //���Ϳ����ľ��Ӿ����ľ��룬�Ѿ�ӳ��0~530.3����ֵ��0~1��
        messageSend.at<double>(0,1) = distance[elemet];
    }
}

int imgProcess::OtsuAlgThreshold(cv::Mat image)
{
        int T = 0; //Otsu�㷨��ֵ
        double varValue = 0; //��䷽���м�ֵ����
        double w0 = 0; //ǰ�����ص�����ռ����
        double w1 = 0; //�������ص�����ռ����
        double u0 = 0; //ǰ��ƽ���Ҷ�
        double u1 = 0; //����ƽ���Ҷ�
        double Histogram[256] = { 0 }; //�Ҷ�ֱ��ͼ���±��ǻҶ�ֵ�����������ǻҶ�ֵ��Ӧ�����ص�����
        uchar *data = image.data;

        double totalNum = image.rows*image.cols; //��������

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
            //ÿ�α���֮ǰ��ʼ��������
            w1 = 0;       u1 = 0;       w0 = 0;       u0 = 0;
            //***********����������ֵ����**************************
            for (int j = 0; j <= i; j++) //�������ָ�ֵ����
            {
                w1 += Histogram[j];   //�����������ص�����
                u1 += j*Histogram[j]; //�������������ܻҶȺ�
            }
            if (w1 == 0) //�����������ص���Ϊ0ʱ�˳�
            {
                break;
            }
            u1 = u1 / w1; //��������ƽ���Ҷ�
            w1 = w1 / totalNum; // �����������ص�����ռ����
            //***********����������ֵ����**************************

            //***********ǰ��������ֵ����**************************
            for (int k = i + 1; k < 255; k++)
            {
                w0 += Histogram[k];  //ǰ���������ص�����
                u0 += k*Histogram[k]; //ǰ�����������ܻҶȺ�
            }
            if (w0 == 0) //ǰ���������ص���Ϊ0ʱ�˳�
            {
                break;
            }
            u0 = u0 / w0; //ǰ������ƽ���Ҷ�
            w0 = w0 / totalNum; // ǰ���������ص�����ռ����
            //***********ǰ��������ֵ����**************************

            //***********��䷽�����******************************
            double varValueI = w0*w1*(u1 - u0)*(u1 - u0); //��ǰ��䷽�����
            if (varValue < varValueI)
            {
                varValue = varValueI;
                T = i;
            }
        }
        T+=offset;
        return T;
}
