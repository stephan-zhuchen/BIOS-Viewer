#include "cliview.h"
#include <fstream>
#include <iostream>

// CliView::CliView() {}

CliView::~CliView() {
    delete ui;
    delete hexUi;
    delete binary;
}

int CliView::openFile(const std::string &path) {
    // 打开文件
    std::cout << "Try open file: " << path << std::endl;

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file: " << path << std::endl;
        return -1; // 返回错误码
    }

    binary = new BinaryData();


    // 获取文件大小
    file.seekg(0, std::ios::end);
    binary->InputImageSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (binary->InputImageSize <= 0) {
        std::cerr << "Error: File is empty or invalid: " << path << std::endl;
        return -1; // 返回错误码
    }

    // 读取文件内容到 InputImage
    binary->InputImage = new UINT8[binary->InputImageSize]; // 分配内存
    file.read(reinterpret_cast<char*>(binary->InputImage), binary->InputImageSize);


    if (!file) {
        std::cerr << "Error: Failed to read file: " << path << std::endl;
        return -1; // 返回错误码
    }

    // 保存文件名
    binary->OpenedFileName = path;
    std::cout << "Successfully opened file: " << path << std::endl;

    if (BiosCliData::isValidBIOS(binary->InputImage, binary->InputImageSize)) {
        std::cout << "open Bios view" << std::endl;
        ui = new BiosCliView(binary);
        ui->loadBios();
    } else {
        std::cout << "open Hex view" << std::endl;
        hexUi = new HexCliView(binary->InputImage, binary->InputImageSize);
        hexUi->show();
    }


    std::cout << "Successfully loaded Bios" << std::endl;

    return 0; // 返回成功
}
