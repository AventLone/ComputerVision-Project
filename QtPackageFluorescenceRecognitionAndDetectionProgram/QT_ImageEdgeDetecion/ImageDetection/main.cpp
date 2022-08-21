#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
//    w.setWindowTitle(QStringLiteral("冯牛的荧光物质检测程序"));
//    w.setWindowTitle(QStringLiteral("qq"));
    w.show();

    return a.exec();
}
