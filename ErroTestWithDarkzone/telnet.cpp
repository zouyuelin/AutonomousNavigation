#include <iostream>
#include "telnet.h"

#define WILL  251
#define WONT  252
#define DO    253
#define DONT  254
#define IAC   255

using namespace std;

telnet::telnet():ConnectStatus(false)
{
    stou1 = GetStdHandle(STD_OUTPUT_HANDLE);

    //socket版本
    WORD wVersion;
    wVersion = MAKEWORD(2, 1);

    //初始化套接字库
    WSADATA wsaData;
    WSAStartup(wVersion, &wsaData);

    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family=AF_INET;

    //监听23端口
    //server_addr.sin_addr.S_un.S_addr=INADDR_ANY;
    server_addr.sin_port = htons(IPPORT_TELNET);//IPPORT_TELNET
    server_addr.sin_addr.s_addr = inet_addr("192.168.0.110");
    memset(server_addr.sin_zero, 0x00, 8);

    if ((sock2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
        qDebug()<<QString::fromLocal8Bit("套接字创建失败！")<<endl;
    if (::connect(sock2, (struct sockaddr *) &server_addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
        qDebug()<<QString::fromLocal8Bit("PMAC连接失败！")<<endl;
    else
    {
        qDebug()<<QString::fromLocal8Bit("PMAC创建成功")<<endl;
    }
    Sleep(3000);

    //与服务器协商
    telnetnvt();
}

telnet::~telnet()
{
    //关闭套接字
    closesocket(sock2);
    WSACleanup();
}

void telnet::connect()
{
    Sleep(1000);
    //reciveMess();
    telSendMess("root");//root
    reciveMess();
    Sleep(1000);

    telSendMess("deltatau");//deltatau
    Sleep(1000);
    reciveMess();

    //get into the shell;
    telSendMess("gpascii");//gpascii
    Sleep(1000);
    reciveMess();
    //reciveMess();
}

//发送数据
inline bool telnet::telSendMess(QString str)
{
    char pBuff[256]={0};
    int nRet = str.length();
    QByteArray ba = str.toLatin1();
    strncpy(pBuff, ba.data(), nRet);

    if(send(sock2,ba.data(),nRet,0)<0)
    {
        qDebug()<<QString::fromLocal8Bit("发送失败:")<<ba.length()<<endl;
        emit connectedReady(false);
        return false;
    }
    else if(!ConnectStatus)
    {
        ConnectStatus = true;
        qDebug()<<QString::fromLocal8Bit("发送成功")<<endl;
        emit connectedReady(true);
    }
    //cout<<ba.data()<<endl;
    send(sock2, "\r\n", 1, 0);//这个控制字符必须要---
    return true;
}

//接收数据
inline QString telnet::reciveMess()
{
    char msg[1024] = {0};
    memset(msg, 0, 1024);
    int res = recv(sock2, msg, 1024, 0);
    if (res == SOCKET_ERROR)
    {
        qDebug()<<"Receive Failed\n";
        return nullptr;
    }
    if (res == 0)
    {
        Sleep(10);
        qDebug()<<"Can't get the message!\n";
    }

    return QString(msg);
}

void telnet::telnetnvt()
{
    unsigned char strRecvBuf[3] = {0};
    while(1)
    {
        //选项协商需要 3 个字节
        //例如 IAC DONT ECHO
        //第一个 IAC 一个字节 (0xff)
        if(recv(sock2, (char*)(strRecvBuf), 1, 0)!=1)
            return;

        //如果不是IAC 选项
        if(strRecvBuf[0] != IAC)
            break;

        //第二个字节是 WILL 、 DO 、 WONT 、 DON''T 四者之一
        //最后一个 ID 字节指明激活或禁止选项。
        if(recv(sock2, (char*)(strRecvBuf+1), 2, 0)!=2)//接收到两个字节，WILL和ID
            return;

        //查看第二个字节是什么，以选择相应的代码
        switch(strRecvBuf[1])
        {
            case WILL:	//WILL
                strRecvBuf[1] = DO;	//DO
                break;

            case WONT:	//WONT
                strRecvBuf[1] = DONT;	//DONT
                break;

            case DO:	//DO
                strRecvBuf[1] = WILL;	//WILL
                break;

            case DONT:	//DONT
                strRecvBuf[1] = WONT;	//WONT
                break;

            default:
                return;
        }

        //发送信息反馈
        send(sock2, (char*)strRecvBuf, 3, 0);
        //qDebug()<<strRecvBuf[1];
    }
}
