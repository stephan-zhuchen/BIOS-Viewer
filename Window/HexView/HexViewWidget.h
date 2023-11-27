#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QSettings>
#include "Volume.h"

namespace Ui {
class HexViewWidget;
}

class QHexView;

class HexViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HexViewWidget(bool darkMode, QWidget *parent = nullptr);
    ~HexViewWidget();

    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void setEditedState(bool edited);

    void loadBuffer(QByteArray buffer, Volume *image, INT64 imageOffset, INT64 imageSize, const QString &bufferName, const QString &imageName, bool Compressed);
    void saveImage();
    void setNewHexBuffer(QByteArray &buffer);

    QHexView    *m_hexview;
    Ui::HexViewWidget *ui;
    QAction     *AlignNoneAction;
    QAction     *AlignImageAction;
    QAction     *Align4GAction;
    QString     title;
    QString     OpenedFileName;
    Volume      *OpenedImage{nullptr};
    INT64       OpenedImageOffset{};
    INT64       OpenedImageSize{};
    QByteArray  hexBuffer;
    QByteArray  NewHexBuffer;
    bool        BinaryEdited{false};
    bool        isCompressed{false};
    QVBoxLayout *m_layout;
    QSettings   setting;

public slots:
    void HexDialogGoto();
    void HexDialogSearch();
    void HexDialogAlignNone();
    void HexDialogAlignImage();
    void HexDialogAlign4G();
};
