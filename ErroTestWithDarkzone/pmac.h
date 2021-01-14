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
#include <opencv2/core.hpp>

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
    cv::Mat temp;

public slots:
    void GetThePosMessage(cv::Mat);

};
#endif // PMAC_H
