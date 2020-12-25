#ifndef SOCK_H
#define SOCK_H
#include <QObject>
#include <iostream>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <stdlib.h>
#include <QString>
//#include <winsock2.h>

class sock:public QObject
{
    Q_OBJECT
public:
    sock();
    ~sock();
    int sock2;
    void connet();
    void  telSendMess(QString str,int hSocket);
    QString reciveMess();
    bool hasconnect;

protected:
        void telnetnvt();
signals:
    void connectedReady(bool);
private slots:
        void connected()
        {
            qDebug()<<"connect successed!"<<endl;
        }
        void error(QAbstractSocket::SocketError)
        {
            qDebug()<<"can't connect,faild!\n";
        }
        void error()
        {
            qDebug()<<"can't connect,faild!\n";
        }
        void connectionError(QAbstractSocket::SocketError error)
        {
            qDebug()<<"connection Error\n";
        }
        void disconnected()
        {
            qDebug()<<"has disconnect\n";
        }
};
#endif // SOCK_H
