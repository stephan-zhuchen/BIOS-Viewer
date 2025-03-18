#ifndef BINARYWINDOW_H
#define BINARYWINDOW_H

#include <QMainWindow>
#include <QWidget>

namespace Ui {
class BinaryWindow;
}

class BinaryWindow : public QWidget
{
    Q_OBJECT
public:
    Ui::BinaryWindow *ui;
    explicit BinaryWindow(QWidget *parent = nullptr);

signals:
};

#endif // BINARYWINDOW_H
