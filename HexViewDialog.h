#ifndef HEXVIEWDIALOG_H
#define HEXVIEWDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include "lib/QHexView/qhexview.h"

namespace Ui {
class HexViewDialog;
}

class HexViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HexViewDialog(QWidget *parent = nullptr);
    ~HexViewDialog();

    void keyPressEvent(QKeyEvent *event) override;

    QHexView *m_hexview;

private:
    Ui::HexViewDialog *ui;
    QVBoxLayout *m_layout;
    QSettings setting{"./Setting.ini", QSettings::IniFormat};

    void loadFile();
};

#endif // HEXVIEWDIALOG_H
