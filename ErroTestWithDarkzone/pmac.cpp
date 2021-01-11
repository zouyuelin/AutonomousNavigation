#include "pmac.h"
pmac::pmac(QObject *parent):runstatus(true),autoControl(true)
{
    pmacConnect = std::shared_ptr<telnet>(new telnet);
    pmacConnect->connect();

    for(int i=0;i<3;i++)
        temp[i]=0;

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

void pmac::GetThePosMessage(double message[3])
{
    QMutexLocker locker(&m_lock);
    for(int i=0;i<2;i++)
        temp[i]=message[i];
}

void pmac::run()
{
    qDebug()<<"Pmac send start\n";
    //开始发送消息
        while(runstatus)
        {
                while(temp[0]!=0 && autoControl)
                {
                    m_lock.lock();
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
                    m_lock.unlock();

                    for(int i=0;i<2;i++)
                        temp_cmd[i] += QString::number(temp[i]);//在极短的指令周期内赋值给temp_cmd

                    Send(temp_cmd[0]);
                    usleep(5);
                    receive();
                    Send(temp_cmd[1]);
                    usleep(5);
                    receive();
                    Send(temp_cmd[2]);
                    usleep(5);
                    receive();
                }
        }
}
