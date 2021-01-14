/*设计思路：
 * 1.设计父进程为imgthread，子进程为powerpmac，
 *   a.在接收进程构造函数中，this->moveToThread(this)或(QApplication::instance()->thread())
 * 2.设置两个相同级别的进程，
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

    imgProcess *imgthread = new imgProcess(path);
    pmac *powerpmac = new pmac;
    //powerpmac->moveToThread(QApplication::instance()->thread());

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
    return 0;
}
