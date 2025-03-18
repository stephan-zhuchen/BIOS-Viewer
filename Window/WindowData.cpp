#include "WindowData.h"
#include "CapsuleView/CapsuleWindow.h"
#include "HexView/HexWindow.h"
#include "BiosView/BiosWindow.h"
#include"BinaryView/BinaryWindow.h"

WindowData::WindowData(QString dir):appDir(std::move(dir)) {}

WindowData::~WindowData() {
    delete BiosViewerUi;
    delete HexViewerUi;
    delete CapsuleViewerUi;
    delete BinaryViewerUi;
    delete InputImage;
}

QStringList vectorToQStringList(const std::vector<std::string>& vec) {
    QStringList qsl;
    qsl.reserve(vec.size()); // 预分配内存提升性能
    for (const auto& str : vec) {
        qsl.append(QString::fromStdString(str));
    }
    return qsl;
}
