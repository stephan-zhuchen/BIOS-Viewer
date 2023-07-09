#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <QWidget>
#include <QSettings>
#include <QTableWidgetItem>
#include <QListWidgetItem>
#include "UefiLib.h"

using UefiSpace::BiosImageVolume;

namespace Ui {
class InfoWindow;
}

class InfoWindow : public QWidget
{
    Q_OBJECT

public:
    explicit InfoWindow(QString Dir, QWidget *parent = nullptr);
    ~InfoWindow();

    void setBiosImage(BiosImageVolume *Image);
    void setOpenedFileName(QString name);
    void setParentWidget(QWidget *pWidget);
    void showFitTab();
    void showMicrocodeTab();
    void showAcmTab();
    void showBtgTab();
    void showFlashmapTab(const QString &SectionFlashMap);
    void showAcpiTab();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void microcodeListWidgetItemSelectionChanged();
    void acmListWidgetItemSelectionChanged();
    void BtgListWidgetItemSelectionChanged();
    void AcpiListWidgetItemSelectionChanged();

private:
    Ui::InfoWindow   *ui;
    BiosImageVolume  *BiosImage{ nullptr };
    QWidget          *parentWidget{ nullptr };
    QString          appDir;
    QString          OpenedFileName;
    QSettings setting{"Intel", "BiosViewer"};

    enum tableColNum {Address=0, Size, Version, C_V, Checksum, Type};
};

#endif // INFOWINDOW_H
