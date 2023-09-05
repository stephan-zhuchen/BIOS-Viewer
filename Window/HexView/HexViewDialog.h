#ifndef HEXVIEWDIALOG_H
#define HEXVIEWDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QSettings>
#include "Volume.h"

namespace Ui {
class HexViewDialog;
}

class QHexView;

class HexViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HexViewDialog(bool darkMode, QWidget *parent = nullptr);
    ~HexViewDialog();

    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void setEditedState(bool edited);

    void loadBuffer(QByteArray buffer, Volume *image, INT64 imageOffset, INT64 imageSize, const QString &bufferName, const QString &imageName, bool Compressed);
    void saveImage();
    void setNewHexBuffer(QByteArray &buffer);

    QHexView    *m_hexview;
    Ui::HexViewDialog *ui;
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

#endif // HEXVIEWDIALOG_H
