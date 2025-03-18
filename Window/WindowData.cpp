#include "WindowData.h"
#include "CapsuleView/CapsuleWindow.h"
#include "HexView/HexWindow.h"
#include "BiosView/BiosWindow.h"

WindowData::WindowData(QString dir):appDir(std::move(dir)) {}

WindowData::~WindowData() {
    delete BiosViewerUi;
    delete HexViewerUi;
    delete CapsuleViewerUi;
    delete InputImage;
}
