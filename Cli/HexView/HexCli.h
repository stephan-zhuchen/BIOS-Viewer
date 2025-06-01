//
// Created by stephan on 25-3-16.
//

#ifndef HEXCLI_H
#define HEXCLI_H

#include <fstream>
#include "BaseLib.h"
#include "DataModel.h"

#ifdef _WIN32
    #include "curses.h"
#else
    #include <ncurses.h>
#endif

class HexCliView {
  public:
    enum class Mode { NORMAL, EDIT, COMMAND_LINE };

  private:
    BinaryData *currentBinaryData{nullptr}; // 指向外部管理的二进制数据

    bool dataModified = false; // 数据是否被修改

    WINDOW *hexWin{nullptr};    // 主内容窗口
    WINDOW *statusWin{nullptr}; // 状态栏/命令行窗口

    Mode currentMode = Mode::NORMAL; // 当前模式
    std::string commandString;       // 用于命令行模式的输入

    // 布局相关
    int addressWidth = 6; // 地址列宽度
    int dataWidth = 48;   // 十六进制数据面板宽度 (16*3)
    int totalWidth = 0;   // 终端总宽度

    // 滚动和光标
    int scrollOffset = 0;            // 滚动偏移（行）
    int VISIBLE_LINES = 20;          // 可视区域的数据行数
    const int BYTES_PER_LINE = 0x10; // 每行字节数

    // 编辑光标位置 (相对于整个数据)
    int editCursorLine = 0;       // 当前编辑的绝对行号
    int editCursorByteInLine = 0; // 当前编辑的行内字节偏移 (0 to BYTES_PER_LINE - 1)
    int editCursorNibble = 0;     // 当前编辑的半字节 (0 高位, 1 低位)

    void calculateLayout();                           // 计算布局参数
    bool saveFile(const std::string &filePathToSave); // 保存文件

    void drawHexContent(); // 绘制十六进制和ASCII内容
    void drawStatusBar();  // 绘制状态栏

    void processCommand();               // 处理命令行输入的命令
    void handleNormalModeInput(int ch);  // 处理普通模式下的输入
    void handleEditModeInput(int ch);    // 处理编辑模式下的输入
    void handleCommandLineInput(int ch); // 处理命令行模式下的输入

    void moveEditCursor(int dLine, int dByte, bool respectNibble = false); // 移动编辑光标
    void ensureCursorVisible();            // 确保光标在可视区域内，调整滚动
    INT64 getCursorAbsoluteOffset() const; // 获取光标的绝对文件偏移
  public:
    HexCliView(BinaryData *binData); // 构造函数接收 BinaryData 指针
    ~HexCliView();

    void show(); // 显示界面并进入主循环
};

#endif // HEXCLI_H
