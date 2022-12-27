#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OpenFile(std::string path)
{
    buffer = new BaseLibrarySpace::Buffer(new std::ifstream(path, std::ios::in | std::ios::binary));

//    qDebug("size = 0x%x", array.size());
    setFvData();
}
