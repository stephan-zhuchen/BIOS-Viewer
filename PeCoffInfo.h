#ifndef PECOFFINFO_H
#define PECOFFINFO_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class PeCoffInfo;
}

class PeCoffInfo : public QDialog
{
    Q_OBJECT

public:
    explicit PeCoffInfo(QWidget *parent = nullptr);
    ~PeCoffInfo();

    void setAsmText(QString txt);
    void setHeaderText(QString txt);
    void setRelocationText(QString txt);

private:
    Ui::PeCoffInfo *ui;
    QSettings setting{"Intel", "BiosViewer"};
};

#endif // PECOFFINFO_H
