#include <iostream>
#include <imageprocess.h>
#include <pmac.h>
#include <QObject>

using namespace std;

int main()
{
    string path = "20210108193519.mp4";

    imgProcess *imgthread = new imgProcess(path);
    pmac *powerpmac = new pmac;

    QObject::connect(imgthread,
                            SIGNAL(SendThePosMessage(double)),
                            powerpmac,
                            SLOT(GetThePosMessage(double)));
    qDebug()<<"---------start---------\n";
    imgthread->start();
    powerpmac->start();

    imgthread->wait();
    powerpmac->wait();
    return 0;
}
