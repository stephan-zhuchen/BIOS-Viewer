#include "Start/StartWindow.h"
#include <QApplication>
#ifdef Q_OS_WIN
#include "vld.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString dir_path = QCoreApplication::applicationDirPath();
    StartWindow w = StartWindow(dir_path);
    w.show();
    if (argc == 2){
        w.OpenFile(argv[1]);
    }
    return QApplication::exec();
}
