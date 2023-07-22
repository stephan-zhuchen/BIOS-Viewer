#include "Window/StartWindow.h"
#include <QApplication>
#include "vld.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString dir_path = QCoreApplication::applicationDirPath();
    StartWindow w = StartWindow(dir_path);
    w.show();
    if (argc == 2){
        w.DoubleClickOpenFile(argv[1]);
    }
    return a.exec();
}
