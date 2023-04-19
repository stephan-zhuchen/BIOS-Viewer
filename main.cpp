#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w = MainWindow();
    w.show();
    if (argc == 2){
        w.DoubleClickOpenFile(argv[1]);
    }
    return a.exec();
}
