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

    //赋值语句需要一条指令周期
    //p1000表示旋转，p1500表示弯曲，p2000表示进给，p2500表示旋转角象限
    QString temp_cmd[3]={"p8288=","p8289=","p8287="};
private slots:
    void GetThePosMessage(double message[3]);

};
#endif // PMAC_H
