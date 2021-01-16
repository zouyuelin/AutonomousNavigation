/*设计思路：
 * 1.设计父进程为imgthread，子进程为powerpmac，
 *   a.1在接收进程构造函数中，this->moveToThread(singal)或发射进程中 singal->moveToThread(this)
 *   a.2或者在父进程的构造函数中实现connect(this,&A::signal,other,&B::slot)和other->start
 *   a.3传递自定义参数时，可以用qRegisterMetaType<Mat>("Mat")的形式注册
 *   a.4利用Object实现的多线程，需要使用moveToThread函数将它放到线程中，例如：
 *
    father_Thread = new QThread();
    child = new child_Thread();
    child->moveToThread(father_Thread);
    father_Thread->start();

 * 2.设置两个相同级别的进程，子线程和子线程之间
 *   b.连接的方式为Qt::DirectConnection，这样槽函数会放在传递信号的线程中运行
 * 3.创建两个线程
 *   c.启动exec()处理函数进行事件处理，但如果是循环run函数，则应该考虑上面两种方式
*/

#include <iostream>
#include <imageprocess.h>
#include <pmac.h>
#include <QObject>

using namespace std;

int main()
{
    qRegisterMetaType<cv::Mat>("cv::Mat");
    string path = "20210108193519.mp4";
    string camera = "";

    imgProcess *imgthread = new imgProcess(path);
    pmac *powerpmac = new pmac;
    //powerpmac->moveToThread(imgthread);

    QObject::connect(imgthread,
                            &imgProcess::SendThePosMessage,
                            powerpmac,
                            &pmac::GetThePosMessage,
                            Qt::DirectConnection);
    //Qt::QueuedConnection Qt::AutoConnection Qt::DirectConnection

    qDebug()<<"---------start---------\n";
    imgthread->start();
    powerpmac->start();

    imgthread->wait();
    powerpmac->wait();
    
    delete imgthread;
    delete powerpmac;
    return 0;
}
