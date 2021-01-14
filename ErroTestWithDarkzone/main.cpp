/*���˼·��
 * 1.��Ƹ�����Ϊimgthread���ӽ���Ϊpowerpmac��
 *   a.�ڽ��ս��̹��캯���У�this->moveToThread(this)��(QApplication::instance()->thread())
 * 2.����������ͬ����Ľ��̣�
 *   b.���ӵķ�ʽΪQt::DirectConnection�������ۺ�������ڴ����źŵ��߳�������
 * 3.���������߳�
 *   c.����exec()�����������¼������������ѭ��run��������Ӧ�ÿ����������ַ�ʽ
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
