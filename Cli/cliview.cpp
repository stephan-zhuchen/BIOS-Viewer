#include "cliview.h"
#include <fstream>
#include <iostream>

CliView::~CliView() {
    delete ui;
    delete hexUi;
    delete binary;
}

int CliView::openFile(const std::string &path, ViewType type) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file: " << path << std::endl;
        return -1;
    }

    binary = new BinaryData();

    // 获取文件大小
    file.seekg(0, std::ios::end);
    binary->InputImageSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (binary->InputImageSize <= 0) {
        std::cerr << "Error: File is empty or invalid: " << path << std::endl;
        return -1;
    }

    // 读取文件内容到 InputImage
    binary->InputImage = new UINT8[binary->InputImageSize]; // 分配内存
    file.read(reinterpret_cast<char *>(binary->InputImage), binary->InputImageSize);

    if (!file) {
        std::cerr << "Error: Failed to read file: " << path << std::endl;
        return -1;
    }

    // 保存文件名
    binary->OpenedFileName = path;

    if (type == ViewType::BiosView) {
        if (BiosCliData::isValidBIOS(binary->InputImage, binary->InputImageSize)) {
            ui = new BiosCliView(binary);
            ui->loadBios();
        } else {
            std::cerr << "Error: Invalid BIOS file: " << path << std::endl;
            return -1;
        }
    } else if (type == ViewType::HexView) {
        hexUi = new HexCliView(binary);
        hexUi->show();
    } else {
        std::cerr << "Error: Unsupported view type." << std::endl;
        return -1;
    }

    return 0; // 返回成功
}
