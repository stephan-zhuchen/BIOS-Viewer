#ifndef WINDOWDATA_H
#define WINDOWDATA_H

#include <QString>
#include "SymbolDefinition.h"

class QHexView;
class StartWindow;
class BiosViewerWindow;
class HexViewWindow;
class CapsuleWindow;

enum class WindowMode { None, Hex, BIOS, CAPSULE };

class WindowData {
public:
    QString           appDir;
    QString           OpenedFileName;
    QString           WindowTitle;
    bool              DarkmodeFlag{false};
    UINT8             *InputImage{nullptr};
    INT64             InputImageSize{};
    INT32             CurrentTabIndex{};
    WindowMode        CurrentWindow {WindowMode::None};
    StartWindow       *parentWindow{nullptr};
    BiosViewerWindow  *BiosViewerUi{nullptr};
    HexViewWindow     *HexViewerUi{nullptr};
    CapsuleWindow     *CapsuleViewerUi{nullptr};

    explicit WindowData(QString dir);
    ~WindowData();
};

#endif // WINDOWDATA_H
