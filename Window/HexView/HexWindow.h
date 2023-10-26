#pragma once

#include <QWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QSettings>
#include "SymbolDefinition.h"

class QHexView;
class StartWindow;
class GeneralData;

class HexViewWindow : public QWidget
{
    Q_OBJECT

public:
    explicit HexViewWindow(StartWindow *parent);
    ~HexViewWindow() override;
    void setupUi(QMainWindow *MainWindow, GeneralData *wData);
    void refresh() const;

    void closeEvent(QCloseEvent *event) override;
    void setEditedState(bool edited);
    void loadBuffer(UINT8 *image, INT64 imageLength);
    void saveImage();
    void setNewHexBuffer(QByteArray &buffer);
    void ActionSearchHexTriggered() const;
    void ActionGotoTriggered() const;

    StartWindow *mWindow;
    GeneralData *WindowData{ nullptr };
    QHexView    *m_hexview{ nullptr };
    QWidget     *centralwidget{ nullptr };
    QVBoxLayout *CentralwidgetVerticalLayout{};
    QByteArray  hexBuffer;
    QByteArray  NewHexBuffer;
    bool        BinaryEdited{false};
    QSettings   setting;
};
