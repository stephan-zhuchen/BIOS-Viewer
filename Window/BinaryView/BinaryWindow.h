#ifndef BINARYWINDOW_H
#define BINARYWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QSettings>
#include "DataModel.h"
#include "WindowData.h"

class ELF;
class PE32;

namespace Ui {
class BinaryWindow;
}

enum class Binary {ELF, PE32};

class BinaryWindow : public QWidget
{
    Q_OBJECT
public:
    Ui::BinaryWindow *ui;

    // Data
    StartWindow     *mWindow{nullptr};
    QSettings       setting;
    WindowData      *winData{nullptr};
    Binary          binaryKind;
    DataModel       *imageModel{nullptr};
    ELF             *elf{nullptr};
    PE32            *pe{nullptr};

    explicit BinaryWindow(StartWindow *parent);
    ~BinaryWindow();
    void setupUi(QMainWindow *MainWindow, WindowData *wData);
    void setTreeData();
    void loadBinary();

    static bool TryOpenBinary(UINT8 *image, INT64 imageLength);

private slots:
    void TreeWidgetItemSelectionChanged() const;

signals:
};

#endif // BINARYWINDOW_H
