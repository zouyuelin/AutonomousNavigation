#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ssd_thread.h"
#include <QPixmap>
#include <QList>
#include <QKeyEvent>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <boost/shared_ptr.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    dnn::Net net;
    VideoCapture capture;
    SSD_Detect *pmacstart;
    SSD_Detect *detect;
    boost::shared_ptr<cv::VideoWriter> writer;
    //SSD_Detect *getTheObjectPosition;
protected:
    void initNet();//³õÊ¼»¯netÍøÂç
    bool hasOpenFrame;
    bool hasOpenPmac;
    virtual void keyPressEvent(QKeyEvent *ev);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void getTheframe(Mat mat);
    void setOpenFrame();
    void setPmacready();
    void on_pushButton_5_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton_6_pressed();
    void on_pushButton_6_released();
    void on_pushButton_10_pressed();
    void on_pushButton_10_released();
    void on_pushButton_7_pressed();
    void on_pushButton_7_released();
    void on_pushButton_9_pressed();
    void on_pushButton_9_released();
    void on_pushButton_8_pressed();
    void on_pushButton_8_released();
    void on_pushButton_16_pressed();
    void on_pushButton_16_released();
    void getConnectStates(bool oi);
    void on_pushButton_3_clicked();
    void on_horizontalSlider_sliderMoved(int position);
    void on_horizontalSlider_2_sliderMoved(int position);
    void on_horizontalSlider_3_sliderMoved(int position);
    void on_horizontalSlider_valueChanged(int value);
    void on_horizontalSlider_2_valueChanged(int value);
    void on_horizontalSlider_3_valueChanged(int value);
    void on_pushButton_Next_clicked();
    void on_pushButton_Last_clicked();
};

#endif // MAINWINDOW_H
