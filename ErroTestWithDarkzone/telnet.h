#ifndef TELNET_H
#define TELNET_H
#include <QObject>
#include <iostream>
#include <time.h>
#include <Windows.h>
#include <QString>
#include <QDebug>

//#pragma comment(lib,"ws2_32.lib")

class telnet:public QObject
{
    Q_OBJECT
public:
    telnet();
    ~telnet();
    void connect();
    SOCKET sock2;
    inline bool telSendMess(QString str);
    QString reciveMess();
    bool ConnectStatus;
    int socketStatus;

protected:
    void telnetnvt();
private:
    HANDLE stou1;
signals:
    void connectedReady(bool);
};
#endif // TELNET_H
