#include <iostream>
#include "sock.h"
#include <time.h>
//#include <Windows.h>

using namespace std;

sock::sock():hasconnect(false)
{
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));/*将serv_addr各个字段清零*/
    server_addr.sin_family=AF_INET;
    //监听23端口
    //server_addr.sin_addr.S_un.S_addr=INADDR_ANY;
    server_addr.sin_port = htons(IPPORT_TELNET);//IPPORT_TELNET
    server_addr.sin_addr.s_addr = inet_addr("192.168.0.110");//192.168.0.110 192.168.10.129
    if ((sock2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        qDebug()<<"the socket created failed！\n";
    else{
        qDebug()<<"the socket created successful\n";
    }
    if (::connect(sock2, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
        qDebug()<<"pmac connect failed！\n";
    else
    {
        qDebug()<<"PMAC creat successful!\n";
    }
    usleep(3000);
    //与服务器协商
    telnetnvt();
}

sock::~sock()
{
    //关闭套接字
    close(sock2);
}

void sock::connet()
{
    usleep(500);
    //reciveMess();
    telSendMess("root", sock2);//root
    reciveMess();
    sleep(1);


    telSendMess("deltatau", sock2);//deltatau
    sleep(1);
    reciveMess();

    //qDebug()<<"-------------------------------get into the shell--------------------------------\n";
    telSendMess("gpascii", sock2);//gpascii
    sleep(1);
    reciveMess();
    //reciveMess();

    //qDebug()<<"---------------------------------发送变量数据---------------------------------------\n";
}

void  sock::telSendMess(QString str,int hSocket)
{
    char pBuff[256]={0};
    int nRet = str.length();
    QByteArray ba = str.toLatin1();
    strncpy(pBuff, ba.data(), nRet);
    /*while (nRet--)
    {
        int resp = send(hSocket, &pBuff[i], 1, 0);
        qDebug()<<pBuff[i];
        i++;

        //if (nRet == 0) break;
    }
    */
    if(send(hSocket,ba.data(),nRet,0)<0)
    {
        qDebug()<<"发送失败:"<<ba.length()<<endl;
        emit connectedReady(false);
    }
    else if(!hasconnect)
    {
        hasconnect = true;
        qDebug()<<"发送成功\n";
        emit connectedReady(true);
    }
    //qDebug()<<ba.data()<<endl;
    send(hSocket, "\r\n", 1, 0);//这个控制字符必须要---------------------------
}

QString sock::reciveMess()
{
    char msg[1024] = {0};
    memset(msg, 0, 1024);
    int res = recv(sock2, msg, 1024, 0);
    if (res == -1)
    {
        qDebug()<<"接收失败\n";
        return 0;
    }
    if (res == 0)
    {
        usleep(10);
        qDebug()<<"can't get the message!\n";
    }
    //printf("%s\n",msg);
    //qDebug()<<msg<<endl;
    return QString(msg);
}

void sock::telnetnvt()
{
    unsigned char strRecvBuf[3] = {0};
    while(1)
    {
        if(recv(sock2, (char*)(strRecvBuf), 1, 0)!=1)
            return;
        if(strRecvBuf[0] != 255)
            break;
        if(recv(sock2, (char*)(strRecvBuf+1), 2, 0)!=2)
            return;

    switch(strRecvBuf[1]){
        case 251:	//WILL
            strRecvBuf[1] = 253;	//DO
            break;

        case 252:	//WONT
            strRecvBuf[1] = 254;	//DONT
            break;

        case 253:	//DO
        case 254:	//DONT
            strRecvBuf[1] = 252;	//WONT
            break;
        default:
            return;
    }
    send(sock2, (char*)strRecvBuf, 3, 0);
    //qDebug()<<strRecvBuf[1];
    }
}
