#include "ssd_thread.h"

SSD_Detect::SSD_Detect(dnn::Net getnet,VideoCapture getcapture,bool isdetect,bool isGetRect )
    :net(getnet),capture(getcapture),autoControl(false),quitTheThread(true),theHolesNumber(0)       //子对象初始化
{
    startdetect = isdetect;
    if(isdetect)
    {
        qDebug()<<"The thread of object-detect has been constructed!"<<endl;
    //--------------设置相关视频参数-----------------
        double rate=capture.get(CAP_PROP_FPS);
        int position=0;
        long totalFrameNumber = capture.get(CAP_PROP_FRAME_COUNT);
        capture.set(CAP_PROP_POS_FRAMES,position);
        capture.set(CAP_PROP_FRAME_WIDTH,1920);
        capture.set(CAP_PROP_FRAME_HEIGHT,1080);
        width=capture.get(CAP_PROP_FRAME_WIDTH);
        height=capture.get(CAP_PROP_FRAME_HEIGHT);
    }
    _isGetRect = isGetRect;
    if(_isGetRect)
    {
        frame = Mat::zeros(600,600,CV_8UC3);
    }
}

SSD_Detect::~SSD_Detect()
{
    //关闭线程函数
    this->requestInterruption();
    this->quit();
    this->wait();
    //删除pmac指针，释放内存
    delete pmac;
}
void SSD_Detect::run()
{
    //得到目标位置
    while(_isGetRect && quitTheThread)
    {
        //objectPosition();
    }
    //目标检测
    if(startdetect)
    {
        qDebug()<<"detect runing"<<endl;
        Detect();
    }
    //pmac
    else  if(!startdetect && _isGetRect==false){
        pmac = new sock();
        QObject::connect(pmac,SIGNAL(connectedReady(bool)),this,SLOT(sendConnectReady(bool)));
        pmac->connet();

        //启动程序代号3
        pmac->telSendMess(QString("&1B3R"),pmac->sock2);

        emit setPmac();
        //qDebug()<<"PMAC thread runing"<<endl;
        pmacsendmassage();
    }
}

void SSD_Detect::sendConnectReady(bool io)
{
    emit getConnectReady(io);
}

void SSD_Detect::pmacsendmassage()
{

    //开始发送消息
    while(quitTheThread)
    {
            while(command[0]!=0 && autoControl)
            {
                double temp[2];
                for(int i=0;i<2;i++)
                temp[i]=command[i];
                QString temp_cmd[3]={"p8288=","p8289=","p8287="};  //赋值语句需要一条指令周期
                //p1000表示旋转，p1500表示弯曲，p2000表示进给，p2500表示旋转角象限

                //下位机根据象限发送信息
                if (0<=temp[0] && temp[0]<0.25) {
                    temp_cmd[2] += QString::number(1);
                }
                else if (0.25<=temp[0] && temp[0]<0.5) {
                    temp_cmd[2] += QString::number(2);
                }
                else if (0.5<=temp[0] && temp[0]<0.75) {
                    temp_cmd[2] += QString::number(3);
                }
                else if (0.75<=temp[0] && temp[0]<1) {
                    temp_cmd[2] += QString::number(4);
                }

                for(int i=0;i<2;i++)
                    temp_cmd[i] += QString::number(temp[i]);//在极短的指令周期内赋值给temp_cmd

                pmac->telSendMess(temp_cmd[0],pmac->sock2);
                usleep(5);
                pmac->reciveMess();
                pmac->telSendMess(temp_cmd[1],pmac->sock2);
                usleep(5);
                pmac->reciveMess();
                pmac->telSendMess(temp_cmd[2],pmac->sock2);
                usleep(5);
                pmac->reciveMess();
            }

    }
}

void SSD_Detect::Detect()
{
        emit setOpenFrameReady();
        //--------------处理识别图像数据-----------------
        while(capture.isOpened() && quitTheThread)
        {
            cv::TickMeter meter;
            meter.start();
            capture>>frame;
            if (frame.empty())
            {
                capture.release();
                break;
            }

            Rect rect(width/2-355,height/2-355,750,750);
            frame=frame(rect);
            //----------------resize GPU加速处理-------------------
                    cuda::GpuMat Mat1(frame);
                    cuda::GpuMat gpu_frame;
                    cuda::resize(Mat1,gpu_frame,Size(frame.rows*0.8,frame.cols*0.8));
                    gpu_frame.download(frame);

            //----------------resize GPU加速完成-------------------
            vector<Point2f> center;
            objectPosition(center);
            setBottle(center,frame);//set big vector
            //imshow("image", frame);
            //waitKey(20);
            center.clear();
            meter.stop();
            putText(frame, format("FPS: %.2f ; time: %.2f ms; number: %d ",
                                  1000.f / meter.getTimeMilli(),
                                  meter.getTimeMilli(),theHolesNumber),
                    Point(10, 20), 0, 0.5, Scalar(0, 0, 255),1);

            emit setTable(frame);

            usleep(10);
        }
}

void inline SSD_Detect::objectPosition(vector<Point2f> &center_p)
{
    Mat inputblob = dnn::blobFromImage(frame,
                                   1.,
                                   Size(inwidth, inheight),false,true);
    net.setInput(inputblob);
    Mat output = net.forward();

    Mat detectionMat(output.size[2], output.size[3], CV_32F, output.ptr<float>());
    float confidenceThreshold = 0.3;

//-------------------显示帧数fps和延迟ms----------------------
//    vector<double> layersTimings;
//    double tick_freq = getTickFrequency();
//    double time_ms = net.getPerfProfile(layersTimings) / tick_freq * 1000;
//    putText(frame, format("FPS: %.2f ; time: %.2f ms; ", 1000.f / time_ms, time_ms),
//            Point(10, 20), 0, 0.5, Scalar(0, 0, 255),1);


//------------------循环遍历显示检测框--------------------------
    for (int i = 0; i < detectionMat.rows; i++)
        {
            float confidence = detectionMat.at<float>(i, 2);
            size_t objectClass = (size_t)(detectionMat.at<float>(i, 1));

            if (confidence > confidenceThreshold && objectClass == 0)
            {
                int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
                int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
                int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
                int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

//                ostringstream ss;
//                ss << confidence;
//                String conf(ss.str());

                if(xRightTop - xLeftBottom>400||yRightTop - yLeftBottom>400)
                    continue;

                //cacl the theta and length
                 Point center_C;
                 center_C.x = (xLeftBottom+xRightTop)/2.F;
                 center_C.y = (yLeftBottom+yRightTop)/2.F;

                 //draw the object crentre
                 //circle(frame,Point((xLeftBottom+xRightTop)/2.F,(yLeftBottom+yRightTop)/2.F),5,Scalar(0,0,255));
                 //circle(frame,Point(frame.cols/2,frame.rows/2),3,Scalar(0,255,255),2);
                 center_p.push_back(center_C);

                //显示检测框
                /*Rect object((int)xLeftBottom, (int)yLeftBottom,
                    (int)(xRightTop - xLeftBottom),
                    (int)(yRightTop - yLeftBottom));

                rectangle(frame, object, getColor[objectClass-1], 2);
                String label = String(classNames[objectClass-1]) + ": " + conf;
                int baseLine = 0;
                Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.3, 1, &baseLine);
                rectangle(frame, Rect(Point(xLeftBottom, yLeftBottom - labelSize.height),
                    Size(labelSize.width, labelSize.height + baseLine)),
                    Scalar(255, 255, 0), FILLED);
                putText(frame, label, Point(xLeftBottom, yLeftBottom),
                    FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));
                    */
            }
        }
            //show the centre of the camera
            circle(frame,Point(frame.cols/2,frame.rows/2),2,Scalar(0,50,255),2);
}

//the big vector
void inline SSD_Detect::setBottle(vector<Point2f> &center_p,Mat &frame)
{
    if(tens.size()<12)
    {
        tens.push_back(center_p);
        return;
    }
    tens.erase(tens.begin());
    tens.push_back(center_p);

    getConfidenceCentre(tens,frame);
}

//get confident centre
void inline SSD_Detect::getConfidenceCentre(vector< vector<Point2f> > tens,Mat &frame)
{
    vector<Point2f> theLastCentre;

    for(vector< vector<Point2f> >::iterator i=tens.begin();i !=tens.end()-5;i++)
    {
        Point2f theLastTemp;
        int count = 0;
        for(auto k = (*i).begin();k != (*i).end() ;k++)
        {
                for(auto j = i+1 ; j !=tens.end();j++)
                {
                    //compare the distance
                    for(auto h = (*j).begin();  (h != (*j).end()&&(*j).size() !=0) ;)
                    {
                        if( distance_(*k,*h)<20)
                        {
                            count++;
                            theLastTemp=*h;
                            h= (*j).erase(h);
                            if(h != (*j).begin())
                                h--;
                            else {
                                continue;
                            }
                        }
                        h++;
                    }
                    if(count>4)
                    {
                        count = 0;
                        theLastCentre.push_back(theLastTemp);
                        //break;
                    }
                }
        }
    }

    //sift the single point in the vector again, which making the result looks better
    //set the threshold as 40, but you can debug the threshold if the envirnoment has chaged.
    for(auto i = theLastCentre.begin(); (i != theLastCentre.end() && theLastCentre.size() != 0);i++)
    {
        for(auto j = i+1;(j != theLastCentre.end() && theLastCentre.size() != 0);j++)
        {
            if(distance_(*j,*i)<40)
            {
                theLastCentre.erase(j);
                j--;
            }
        }
    }

    //cal theta and distance
    if(theLastCentre.size()>0)
        theTempCentre = theLastCentre;

    //sift the points whose thread doesn't meet the requirement
    threadSift();

    for(int i = 0; i<theTempCentre.size();i++)
    {
        circle(frame,theTempCentre[i],3,Scalar(0,255,255),2);
        circle(frame,theTempCentre[i],20,Scalar(0,255,0),1);
    }
    caclDistanceandTheta(theTempCentre,frame,0);
    theHolesNumber = theTempCentre.size();
}

//the distance of the points
double inline SSD_Detect::distance_(Point2f point1,Point2f point2)
{
    return sqrt(pow(point1.x-point2.x,2)+pow(point1.y-point2.y,2));
}

//sift the points whose thread doesn't meet the requirement
void inline SSD_Detect::threadSift()
{
    int averThread=0;
    int size = 7;
    for(int i = 0;i<theTempCentre.size();i++)
    {
            for(int k = -1*(size/2);k<size-size/2;k++)
                for(int h = -1*(size/2);h<size-size/2;h++)
                    averThread += frame.at<Vec3b>(theTempCentre[i].x+k,theTempCentre[i].y+h)[0];
            averThread /= pow(size,2);
            if(averThread>185)
            {
                theTempCentre.erase(i+theTempCentre.begin());
                i--;
            }
     }
}

//select the holes to get in

//cal theta and distance
void inline  SSD_Detect::caclDistanceandTheta(vector<Point2f> theLastCentre,Mat &frame,int elemet)
{
    vector<double> theta;
    vector<double> distance;
    for(int i = 0;i<theLastCentre.size();i++)
    {
            double K =(frame.rows/2.-theLastCentre[i].y)/(frame.cols/2.-theLastCentre[i].x);
            double theta_val = 0.0;
            if(frame.rows/2.>theLastCentre[i].y&&frame.cols/2.<theLastCentre[i].x)
            theta_val = qAtan(-K);
            else if (frame.rows/2.>theLastCentre[i].y&&frame.cols/2.>theLastCentre[i].x) {
            theta_val =qAtan(K)+1.57;
            }
            else if (frame.rows/2.<theLastCentre[i].y&&frame.cols/2.>theLastCentre[i].x) {
            theta_val =qAtan(-K)+3.14;
            }
            else if (frame.rows/2.<theLastCentre[i].y&&frame.cols/2.<theLastCentre[i].x) {
            theta_val =6.28-qAtan(K);
            }
            //save the theta(angle)
            theta.push_back(theta_val/6.28);
            distance.push_back(sqrt(pow((frame.cols/2-theLastCentre[i].x),2)
                  +pow((frame.rows/2-theLastCentre[i].y),2))/530.3);

            //cast the data to the powerpmac
            theta[i] = theta[i];
            distance[i] = distance[i]/1.F;
    }
    //telnet send data
    if(distance.size()!=0)
    {
        //auto smallest = std::min_element(std::begin(distance), std::end(distance));
        //int elemet = std::distance(std::begin(distance),smallest);

        //draw the camera centre to the object centre with a green line
        if(theLastCentre.size()>0)
            line(frame,theLastCentre[elemet],Point(frame.cols/2,frame.rows/2),Scalar(255,200,0));

        //发送偏转角度，已经映射0~360°到0~1；
        command[0] = theta[elemet];
        //发送孔中心距视觉中心距离，已经映射0~530.3像素值到0~1；
        command[1] = distance[elemet];
    }
}
