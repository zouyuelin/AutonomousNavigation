#include <iostream>
#include "sock.h"
#include <time.h>
#include <Windows.h>

using namespace std;

sock::sock():hasconnect(false)
{
    stou1 = GetStdHandle(STD_OUTPUT_HANDLE);
    WSADATA wsaData;
    WORD wVersion;
    wVersion = MAKEWORD(2, 1);   //socket�汾
    WSAStartup(wVersion, &wsaData);//��ʼ���׽��ֿ�
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    //����23�˿�
    //server_addr.sin_addr.S_un.S_addr=INADDR_ANY;
    server_addr.sin_port = htons(IPPORT_TELNET);//IPPORT_TELNET
    server_addr.sin_addr.s_addr = inet_addr("192.168.0.110");//192.168.0.110 192.168.10.129
    memset(server_addr.sin_zero, 0x00, 8);
    if ((sock2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
        cout<<"�׽��ִ���ʧ�ܣ�\n";
    if (::connect(sock2, (struct sockaddr *) &server_addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
        cout<<"pmac����ʧ�ܣ�\n";
    else
    {
        cout<<"PMAC�����ɹ�\n";
    }
    Sleep(3000);
    //�������Э��
    telnetnvt();
}

sock::~sock()
{
    //�ر��׽���
    closesocket(sock2);
    WSACleanup();
}

void sock::connet()
{
    Sleep(1000);
    //reciveMess();
    telSendMess("root", sock2);//root
    reciveMess();
    Sleep(1000);


    telSendMess("deltatau", sock2);//deltatau
    Sleep(1000);
    reciveMess();

    //cout<<"-------------------------------get into the shell--------------------------------\n";
    telSendMess("gpascii", sock2);//gpascii
    Sleep(1000);
    reciveMess();
    //reciveMess();

    //cout<<"---------------------------------���ͱ�������---------------------------------------\n";
}

void  sock::telSendMess(QString str,SOCKET hSocket)
{
    char pBuff[256]={0};
    unsigned long dwLen;
    int nRet = str.length();
    int i = 0;
    QByteArray ba = str.toLatin1();
    strncpy(pBuff, ba.data(), nRet);
    /*while (nRet--)
    {
        int resp = send(hSocket, &pBuff[i], 1, 0);
        cout<<pBuff[i];
        i++;

        //if (nRet == 0) break;
    }
    */
    if(send(hSocket,ba.data(),nRet,0)<0)
    {
        cout<<"����ʧ��:"<<ba.length()<<endl;
        emit connectedReady(false);
    }
    else if(!hasconnect)
    {
        hasconnect = true;
        cout<<"���ͳɹ�\n";
        emit connectedReady(true);
    }
    //cout<<ba.data()<<endl;
    send(hSocket, "\r\n", 1, 0);//��������ַ�����Ҫ---------------------------
}

void sock::reciveMess()
{
    char msg[1024] = {0};
    memset(msg, 0, 1024);
    int res = recv(sock2, msg, 1024, 0);
    if (res == SOCKET_ERROR)
    {
        cout<<"����ʧ��\n";
        return;
    }
    if (res == 0)
    {
        Sleep(10);
        cout<<"can't get the message!\n";
    }
    //printf("%s\n",msg);
    //qDebug()<<msg<<endl;
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
    //cout<<strRecvBuf[1];
    }
}
