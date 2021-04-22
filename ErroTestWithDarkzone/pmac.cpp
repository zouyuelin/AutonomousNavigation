#include "pmac.h"
pmac::pmac(QObject *parent):runstatus(true),autoControl(true),temp(cv::Mat::zeros(1,3,CV_64F))
{
    pmacConnect = std::shared_ptr<telnet>(new telnet);
    pmacConnect->connect();

    qDebug()<<"Pre-process completed\n";
}

pmac::~pmac()
{
    this->requestInterruption();
    this->quit();
    this->wait();
}

inline bool pmac::Send(QString message)
{
    return pmacConnect->telSendMess(message);
}

inline QString pmac::receive()
{
    return pmacConnect->reciveMess();
}

void pmac::AllStop()
{
    QMutexLocker locker(&m_lock);
    autoControl = false;
    runstatus = false;
    this->terminate();
    this->wait();
}

void pmac::GetThePosMessage(cv::Mat message)
{
    QMutexLocker locker(&m_lock);
    for(int i=0;i<3;i++)
        temp.at<double>(0,i)=message.at<double>(0,i);//windmsg rotatemsg radiomsg
}

void pmac::run()
{
    qDebug()<<"Pmac send start\n";
    //开始发送消息
    Send(QString("p8306=0"));
        while(runstatus)
        {
                while(temp.at<double>(0,0)!=0 && autoControl)
                {
                    //QMutexLocker locker(&m_lock);
                    //赋值语句需要一条指令周期
                    //p1000表示旋转，p1500表示弯曲，p2000表示进给，p2500表示旋转角象限
                    QString temp_cmd[4]={"p8294=","p8295=","p8297=","p8293="};
                    //m_lock.lock();
                    //下位机根据象限发送信息
                    if(temp.at<double>(0,0)==0 && temp.at<double>(0,1)==0)
                    {
                        temp_cmd[3] += QString::number(0);
                    }
                    else if (0<=temp.at<double>(0,0) && temp.at<double>(0,0)<0.25) {
                        temp_cmd[3] += QString::number(1);
                    }
                    else if (0.25<=temp.at<double>(0,0) && temp.at<double>(0,0)<0.5) {
                        temp_cmd[3] += QString::number(2);
                    }
                    else if (0.5<=temp.at<double>(0,0) && temp.at<double>(0,0)<0.75) {
                        temp_cmd[3] += QString::number(3);
                    }
                    else if (0.75<=temp.at<double>(0,0) && temp.at<double>(0,0)<1) {
                        temp_cmd[3] += QString::number(4);
                    }

                    //QString::number((float)temp.at<double>(0,i))
                    for(int i=0;i<3;i++)
                        temp_cmd[i] += QString::number(temp.at<double>(0,i),'f',3);//在极短的指令周期内赋值给temp_cmd，保留3位小数
                    //m_lock.unlock();

                    for(auto index=0;index<4;index++)
                    {

                        Send(temp_cmd[index]);
                        receive();  //从缓冲栈读取数据
                        usleep(3);
                    }

                }
        }
}
