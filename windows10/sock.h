#ifndef SOCK_H
#define SOCK_H
#include <QObject>
#include <iostream>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <winsock2.h>


class sock:public QObject
{
    Q_OBJECT
public:
    sock();
    ~sock();
    SOCKET sock2;
    void connet();
    void  telSendMess(QString str,SOCKET hSocket);
    void reciveMess();
    bool hasconnect;

protected:
        void telnetnvt();
private:
    HANDLE stou1;
signals:
    void connectedReady(bool);
private slots:
        void connected()
        {
            std::cout<<"connect successed!"<<std::endl;
        }
        void error(QAbstractSocket::SocketError)
        {
            std::cout<<"can't connect,faild!"<<std::endl;
        }
        void error()
        {
            std::cout<<"can't connect,faild!"<<std::endl;
        }
        void connectionError(QAbstractSocket::SocketError error)
        {
            std::cout<<"connection Error"<<std::endl;
        }
        void disconnected()
        {
            std::cout<<"has disconnect"<<std::endl;
        }
};
#endif // SOCK_H
