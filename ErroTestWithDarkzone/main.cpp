/*���˼·��
 * 1.��Ƹ�����Ϊimgthread���ӽ���Ϊpowerpmac��
 *   a.1�ڽ��ս��̹��캯���У�this->moveToThread(this)��(QApplication::instance()->thread())
 *   a.2�����ڸ����̵Ĺ��캯����ʵ��connect(this,&A::signal,other,&B::slot)��other->start
 *   a.3�����Զ������ʱ��������qRegisterMetaType<Mat>("Mat")����ʽע��
 *   a.4����Objectʵ�ֵĶ��̣߳���Ҫʹ��moveToThread���������ŵ��߳��У����磺
 *
    father_Thread = new QThread();
    child = new child_Thread();
    child->moveToThread(father_Thread);
    father_Thread->start();

 * 2.����������ͬ����Ľ��̣����̺߳����߳�֮��
 *   b.���ӵķ�ʽΪQt::DirectConnection�������ۺ�������ڴ����źŵ��߳�������
 * 3.���������߳�
 *   c.����exec()�����������¼������������ѭ��run��������Ӧ�ÿ����������ַ�ʽ
*/

#include <iostream>
#include <imageprocess.h>
#include <pmac.h>
#include <QObject>

using namespace std;

int main(int argc,char**argv)
{
    qRegisterMetaType<cv::Mat>("cv::Mat");
    string path = "20210108193519.mp4";
    string back = "background.jpg";
    string camera = "";
    imgProcess *imgthread = new imgProcess(path,back);
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
    return 0;
}
