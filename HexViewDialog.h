#ifndef HEXVIEWDIALOG_H
#define HEXVIEWDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QSettings>
#include "lib/UefiLib.h"

namespace Ui {
class HexViewDialog;
}

class QHexView;
using UefiSpace::Volume;

class HexViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HexViewDialog(QString &applicationDir, QWidget *parent = nullptr);
    ~HexViewDialog();

    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void setEditedState(bool edited);
    void loadFile();
    void loadBuffer(QByteArray buffer, Volume *image, INT64 imageOffset, const QString &bufferName, const QString &imageName, bool Compressed);
    void saveImage();
    void setNewHexBuffer(QByteArray &buffer);

    QHexView *m_hexview;

private:
    Ui::HexViewDialog *ui;
    QString     title;
    QString     OpenedFileName;
    Volume      *OpenedImage{nullptr};
    INT64       OpenedImageOffset;
    QByteArray  hexBuffer;
    QByteArray  NewHexBuffer;
    bool        BinaryEdited{false};
    bool        isCompressed{false};
    QVBoxLayout *m_layout;
    QSettings   setting;
};

#endif // HEXVIEWDIALOG_H
