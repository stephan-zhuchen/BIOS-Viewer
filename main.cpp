#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.OpenFile("RPL_S_FSPWRAPPER_3265_00_D.rom");
    return a.exec();
}
