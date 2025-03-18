#ifndef WINDOWDATA_H
#define WINDOWDATA_H

#include <QString>
#include "SymbolDefinition.h"

class QHexView;
class StartWindow;
class BiosViewerWindow;
class HexViewWindow;
class CapsuleWindow;
class BinaryWindow;

enum class WindowMode { None, Hex, BIOS, CAPSULE, BINARY };

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
    BinaryWindow      *BinaryViewerUi{nullptr};

    explicit WindowData(QString dir);
    ~WindowData();
};

QStringList vectorToQStringList(const std::vector<std::string>& vec);

#endif // WINDOWDATA_H
