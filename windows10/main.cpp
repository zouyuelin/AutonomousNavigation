#include "mainwindow.h"
#include <QApplication>
#include <QIcon>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Auto navigation system");                   //设置主窗口的标题
    w.setWindowIcon(QIcon("/home/zyl/qt_creator/AutoControlSystem/12.jpg"));
    w.move(0,0);
    w.show();
    w.setFocus();

    return a.exec();
}
