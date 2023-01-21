#ifndef HEXVIEWWIDGET_H
#define HEXVIEWWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include "lib/QHexView/qhexview.h"

namespace Ui {
class HexViewWidget;
}

class HexViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HexViewWidget(QWidget *parent = nullptr);
    ~HexViewWidget();

//    void keyPressEvent(QKeyEvent *event) override;
//    void mousePressEvent(QMouseEvent *event) override;

    QHexView *m_hexview;

private:
    Ui::HexViewWidget *ui;
    QVBoxLayout *m_layout;
};

#endif // HEXVIEWWIDGET_H
