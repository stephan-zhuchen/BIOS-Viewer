#pragma once

#include <QMainWindow>
#include <QFile>
#include <iostream>
#include <fstream>
#include "lib/BaseLib.h"
#include "lib/UefiLib.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void OpenFile(std::string path);
    void setFvData();

private:
    Ui::MainWindow *ui;
    BaseLibrarySpace::Buffer *buffer{nullptr};

    QByteArray *FirmwareVolumeData;
};

