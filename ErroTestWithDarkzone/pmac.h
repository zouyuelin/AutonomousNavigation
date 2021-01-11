#ifndef PMAC_H
#define PMAC_H

#include <QString>
#include <QMutex>
#include <QThread>
#include <QTime>
#include <QDebug>
#include "telnet.h"
#include <vector>
#include <memory>

class pmac:  public QThread
{
    Q_OBJECT
public:
    explicit pmac(QObject *parent = nullptr);
    ~pmac() override;
    void AllStop();
    void run() override;
    std::shared_ptr<telnet> pmacConnect;

protected:
    inline bool Send(QString message);
    inline QString receive();
private:
    QMutex m_lock;
    bool runstatus;
    bool autoControl;
    double temp[3];

    //��ֵ�����Ҫһ��ָ������
    //p1000��ʾ��ת��p1500��ʾ������p2000��ʾ������p2500��ʾ��ת������
    QString temp_cmd[3]={"p8288=","p8289=","p8287="};
private slots:
    void GetThePosMessage(double message[3]);

};
#endif // PMAC_H
