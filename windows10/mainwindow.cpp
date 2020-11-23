#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMetaType>
#include <QPainter>
#pragma execution_character_set("utf-8")//使中文字符可以显示

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),hasOpenFrame(false),hasOpenPmac(false),
    pmacstart(nullptr),
    ui(new Ui::MainWindow)
{
    qRegisterMetaType<Mat>("Mat");                              //将Mat注册到队列信息中
    //初始化相关参数
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);//设置不可实现最大化
    setFixedSize(1551,728);                                      //设置窗口不可调整
    ui->setupUi(this);
    ui->pushButton->setEnabled(false);
    ui->radioButton_2->setChecked(true);
    //pmacstart = new SSD_Detect(net,capture,false);
    //detect = new SSD_Detect(net,capture,true);
    ui->pushButton_6->setEnabled(false);
    ui->pushButton_7->setEnabled(false);
    ui->pushButton_8->setEnabled(false);
    ui->pushButton_9->setEnabled(false);
    ui->pushButton_10->setEnabled(false);
    ui->pushButton_16->setEnabled(false);
    ui->pushButton->setEnabled(true);

    ui->horizontalSlider->setMaximum(20000);
    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setValue(3000);

    ui->horizontalSlider_2->setMaximum(5000);
    ui->horizontalSlider_2->setMinimum(0);
    ui->horizontalSlider_2->setValue(500);

    ui->horizontalSlider_3->setMaximum(500);
    ui->horizontalSlider_3->setMinimum(0);
    ui->horizontalSlider_3->setValue(80);

    ui->label_13->setText(QString::number(ui->horizontalSlider->value()));
    ui->label_14->setText(QString::number(ui->horizontalSlider_2->value()));
    ui->label_15->setText(QString::number(ui->horizontalSlider_3->value()));

    writer = new VideoWriter("./Video.avi",VideoWriter::fourcc('X', 'V', 'I', 'D'),
                             30,Size(600,600));

    this->setFocus();
}

MainWindow::~MainWindow()
{
    qDebug()<<"quiting\n";
    //delete getTheObjectPosition;
    delete writer;
    if(detect != nullptr)
    delete detect;
    if(pmacstart != nullptr)
    delete pmacstart;
    delete ui;
}

void MainWindow::initNet()
{
    String pb_path="../Auto-win10/faster-rcnn/frozen_inference_graph.pb";
    String pbtxt_path="../Auto-win10/faster-rcnn/frozen_inference_graph.pbtxt";
    net=dnn::readNetFromTensorflow(pb_path,pbtxt_path);

    //开启GPU前向传播运算
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    if(!net.empty()){
        qDebug()<<"get the pb and pbtxt successful!\n";
    }
    else {
        qDebug()<<"get the pb and pbtxt faild!\n";
        return ;
    }
    //---------------导入视频数据------------------
    string val_path = "../Auto-win10/20200928201136.mp4";

    if(ui->radioButton_2->isChecked())
        capture=VideoCapture(val_path);
    else if(ui->radioButton->isChecked())
        capture=VideoCapture(0,CAP_DSHOW);
    if (capture.isOpened())
    {
        qDebug()<<"open the file successful\n";
    }
    else {
        qDebug()<<"open the file faild\n";
        return ;
    }
}

//打开目标检测线程
void MainWindow::on_pushButton_clicked()
{
    initNet();

    if(!hasOpenFrame)
    {
        hasOpenFrame = true;
        detect = new SSD_Detect(net,capture,true);
        //getTheObjectPosition =new SSD_Detect(net,capture,false,true);
        QObject::connect(detect,SIGNAL(setTable(Mat)),this,SLOT(getTheframe(Mat)),Qt::QueuedConnection);//QueuedConnection means asynchronous 异步
        QObject::connect(detect,SIGNAL(setOpenFrameReady()),this,SLOT(setOpenFrame()));
        detect->start();
        //getTheObjectPosition->start();
    }
}

//打开pmac通讯线程
void MainWindow::on_pushButton_2_clicked()
{
        ui->label_6->setText("connecting...");
        hasOpenPmac = true;
        pmacstart = new SSD_Detect(net,capture,false);
        QObject::connect(pmacstart,SIGNAL(setPmac()),this,SLOT(setPmacready()),Qt::DirectConnection);
        QObject::connect(pmacstart,SIGNAL(getConnectReady(bool)),this,SLOT(getConnectStates(bool)));
        //QObject::connect(pmacstart->pmac,SIGNAL(connectedError()),this,SLOT(getConnectError()));
        pmacstart->autoControl=false;
        pmacstart->start();
        Sleep(1000);
        ui->pushButton_5->setEnabled(false);
        ui->pushButton_11->setEnabled(false);
}

void MainWindow::getTheframe(Mat mat)
{
    //利用cuda加速处理cvtColor函数
    //getTheObjectPosition->frame = detect->frame.clone();
    if(pmacstart != nullptr)
    {
        for(int i=0;i<2;i++)
            pmacstart->command[i]=detect->command[i];
    }
    Mat oringe = mat.clone();
    cuda::GpuMat Mat1(mat);
    cuda::GpuMat gpu_frame;
    cuda::cvtColor(Mat1,gpu_frame,COLOR_BGR2RGB);
    gpu_frame.download(mat);
    //将图像数据映射到QImage中
    QImage disImage = QImage((const unsigned char*)(mat.data),mat.cols,
                             mat.rows,QImage::Format_RGB888);
    QPixmap temp_pix = QPixmap::fromImage(disImage);
    //画出矩形框
    /*for(int i=0;i<getTheObjectPosition->qrect.size();i++)
        painter.drawRect(getTheObjectPosition->qrect[i]);*/

    ui->label->setPixmap(temp_pix);
    ui->label->show();
    //getTheObjectPosition->qrect.clear();
    *writer << oringe;
}

void MainWindow::setOpenFrame()
{
    ui->label_5->setText("Detect running");
}
void MainWindow::setPmacready()
{
    ui->label_7->setText("Pmac running");

    //---------------the velocity of the three motors----------------------------------
    pmacstart->pmac->telSendMess(QString("p8302=3000"),pmacstart->pmac->sock2);
    pmacstart->pmac->telSendMess(QString("p8303=500"),pmacstart->pmac->sock2);
    pmacstart->pmac->telSendMess(QString("p8304=80"),pmacstart->pmac->sock2);
}

//急停函数,变为点动程序
void MainWindow::on_pushButton_5_clicked()
{
    ui->pushButton_11->setText(QString("continue"));
    if(pmacstart != nullptr)
    {
        pmacstart->autoControl=false;
        pmacstart->pmac->telSendMess(QString("p8300=1"),pmacstart->pmac->sock2);
    }
}

//开始导航
void MainWindow::on_pushButton_11_clicked()
{
    ui->pushButton_11->setText(QString("guiding..."));
    if(pmacstart != nullptr)
    {
        pmacstart->pmac->telSendMess(QString("p8300=0"),pmacstart->pmac->sock2);
        pmacstart->autoControl=true;
    }

}

//p8294
void MainWindow::on_pushButton_6_pressed()
{
    pmacstart->pmac->telSendMess(QString("p8300=1"),pmacstart->pmac->sock2);
    Sleep(5);
    pmacstart->pmac->telSendMess(QString("p8294=1"),pmacstart->pmac->sock2);
    Sleep(5);
}

void MainWindow::on_pushButton_6_released()
{
    pmacstart->pmac->telSendMess(QString("p8294=0"),pmacstart->pmac->sock2);
    Sleep(5);
}

//p8295
void MainWindow::on_pushButton_10_pressed()
{
    pmacstart->pmac->telSendMess(QString("p8300=1"),pmacstart->pmac->sock2);
    Sleep(5);
    pmacstart->pmac->telSendMess(QString("p8295=1"),pmacstart->pmac->sock2);
    Sleep(5);
}

void MainWindow::on_pushButton_10_released()
{
    pmacstart->pmac->telSendMess(QString("p8295=0"),pmacstart->pmac->sock2);
    Sleep(5);
}

//p8296
void MainWindow::on_pushButton_7_pressed()
{
    pmacstart->pmac->telSendMess(QString("p8300=1"),pmacstart->pmac->sock2);
    Sleep(5);
    pmacstart->pmac->telSendMess(QString("p8296=1"),pmacstart->pmac->sock2);
    Sleep(5);
}

void MainWindow::on_pushButton_7_released()
{
    pmacstart->pmac->telSendMess(QString("p8296=0"),pmacstart->pmac->sock2);
    Sleep(5);
}

//p8297
void MainWindow::on_pushButton_9_pressed()
{
    pmacstart->pmac->telSendMess(QString("p8300=1"),pmacstart->pmac->sock2);
    Sleep(5);
    pmacstart->pmac->telSendMess(QString("p8297=1"),pmacstart->pmac->sock2);
    Sleep(5);
}

void MainWindow::on_pushButton_9_released()
{
    pmacstart->pmac->telSendMess(QString("p8297=0"),pmacstart->pmac->sock2);
    Sleep(5);
}

//p8298
void MainWindow::on_pushButton_8_pressed()
{
    pmacstart->pmac->telSendMess(QString("p8300=1"),pmacstart->pmac->sock2);
    Sleep(5);
    pmacstart->pmac->telSendMess(QString("p8298=1"),pmacstart->pmac->sock2);
    Sleep(5);
}

void MainWindow::on_pushButton_8_released()
{
    pmacstart->pmac->telSendMess(QString("p8298=0"),pmacstart->pmac->sock2);
    Sleep(5);
}
//p8299
void MainWindow::on_pushButton_16_pressed()
{
    pmacstart->pmac->telSendMess(QString("p8300=1"),pmacstart->pmac->sock2);
    Sleep(5);
    pmacstart->pmac->telSendMess(QString("p8299=1"),pmacstart->pmac->sock2);
    Sleep(5);
}

void MainWindow::on_pushButton_16_released()
{
    pmacstart->pmac->telSendMess(QString("p8299=0"),pmacstart->pmac->sock2);
    Sleep(5);
}

void MainWindow::getConnectStates(bool oi)
{
    if(oi)
    {
            ui->label_6->setText("connected successful!");
    }
    else {
            ui->label_6->setText("connected failed!");
    }
    ui->pushButton_5->setEnabled(true);
    ui->pushButton_11->setEnabled(true);
    ui->pushButton_6->setEnabled(true);
    ui->pushButton_7->setEnabled(true);
    ui->pushButton_8->setEnabled(true);
    ui->pushButton_9->setEnabled(true);
    ui->pushButton_10->setEnabled(true);
    ui->pushButton_16->setEnabled(true);
}

//程序退出
void MainWindow::on_pushButton_3_clicked()
{
    if(pmacstart != nullptr)
        pmacstart->pmac->telSendMess(QString("p8301=1"),pmacstart->pmac->sock2);
    if(pmacstart != nullptr)
        pmacstart->quitTheThread = false;
    if(detect != nullptr)
        detect->quitTheThread = false;
    this->close();
}

void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    ui->label_13->setText(QString::number(ui->horizontalSlider->value()));
    if(pmacstart != nullptr)
        pmacstart->pmac->telSendMess(QString("p8302=")+QString::number(ui->horizontalSlider->value()),pmacstart->pmac->sock2);
}

void MainWindow::on_horizontalSlider_2_sliderMoved(int position)
{
    ui->label_14->setText(QString::number(ui->horizontalSlider_2->value()));
    if(pmacstart != nullptr)
        pmacstart->pmac->telSendMess(QString("p8303=")+QString::number(ui->horizontalSlider_2->value()),pmacstart->pmac->sock2);
}

void MainWindow::on_horizontalSlider_3_sliderMoved(int position)
{
    ui->label_15->setText(QString::number(ui->horizontalSlider_3->value()));
    if(pmacstart != nullptr)
        pmacstart->pmac->telSendMess(QString("p8304=")+QString::number(ui->horizontalSlider_3->value()),pmacstart->pmac->sock2);
}


void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->label_13->setText(QString::number(ui->horizontalSlider->value()));
    if(pmacstart != nullptr)
        pmacstart->pmac->telSendMess(QString("p8302=")+QString::number(ui->horizontalSlider->value()),pmacstart->pmac->sock2);
}

void MainWindow::on_horizontalSlider_2_valueChanged(int value)
{
    ui->label_14->setText(QString::number(ui->horizontalSlider_2->value()));
    if(pmacstart != nullptr)
        pmacstart->pmac->telSendMess(QString("p8303=")+QString::number(ui->horizontalSlider_2->value()),pmacstart->pmac->sock2);
}

void MainWindow::on_horizontalSlider_3_valueChanged(int value)
{
    ui->label_15->setText(QString::number(ui->horizontalSlider_3->value()));
    if(pmacstart != nullptr)
        pmacstart->pmac->telSendMess(QString("p8304=")+QString::number(ui->horizontalSlider_3->value()),pmacstart->pmac->sock2);
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    switch (ev->key())
    {
    case Qt::Key_Space:
         on_pushButton_5_clicked();
     default :
        QWidget::keyPressEvent(ev);
    }

}


/*
#define	Quadrant	pshm->P[8287]
#define	rotateMsg	pshm->P[8288]
#define	windMsg	pshm->P[8289]
#define	forwardMsg	pshm->P[8290]
#define	motor1C	pshm->P[8294]
#define	motor1R	pshm->P[8295]
#define	motor2C	pshm->P[8296]
#define	motor2R	pshm->P[8297]
#define	motor3C	pshm->P[8298]
#define	motor3R	pshm->P[8299]
#define	PointControl	pshm->P[8300]
#define	stopall	pshm->P[8301]
  */

