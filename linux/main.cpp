#include "mainwindow.h"
#include <QApplication>
#include <QIcon>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Auto navigation system");                   //���������ڵı���
    w.setWindowIcon(QIcon("./12.jpg"));
    w.move(200,160);
    w.show();
    w.setFocus();

    return a.exec();
}
