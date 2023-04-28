#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString dir_path = QCoreApplication::applicationDirPath();
    MainWindow w = MainWindow(dir_path);
    w.show();
    if (argc == 2){
        w.DoubleClickOpenFile(argv[1]);
    }
    return a.exec();
}
