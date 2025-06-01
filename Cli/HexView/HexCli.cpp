//
// Created by stephan on 25-3-16.
//

#include "HexCli.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef _WIN32
    #define VLINE_CHAR '|'
    #define HLINE_CHAR '-'
#else
    #define VLINE_CHAR ACS_VLINE
    #define HLINE_CHAR ACS_HLINE
#endif

// 辅助函数：将字符转换为十六进制值
int hexCharToVal(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return '.'; // 对于任何其他非法字符，返回ASCII码的点
}

void HexCliView::calculateLayout() {
    INT64 maxOffset = currentBinaryData->InputImageSize > 0 ? currentBinaryData->InputImageSize - 1 : 0;
    int hexDigits = 1; // 默认至少需要1位十六进制数显示偏移

    if (maxOffset > 0) {
        // 根据最大偏移量计算需要的十六进制位数
        hexDigits = static_cast<int>(log2(static_cast<double>(maxOffset)) / 4) + 1;
        hexDigits = std::clamp(hexDigits, 1, 8); // 将位数限制在1到8位之间
    }

    // 地址列宽度 = "0x" (2个字符) + 十六进制数字位数 + " " (1个空格)
    addressWidth = 2 + hexDigits + 1;
    addressWidth = std::max(6, addressWidth); // 保证地址列至少能显示 "0x0000 "

    // 十六进制数据面板宽度 = 每行字节数 * 每个字节的显示宽度 ("XX ")
    dataWidth = BYTES_PER_LINE * 3;

    // ASCII字符面板宽度 = 每行字节数 (每个ASCII字符占1列)
    int asciiPanelWidth = BYTES_PER_LINE;

    // 计算各主要部分的起始X坐标
    int hex_data_start_x = addressWidth + 3;              // 地址区后，跳过" | "分隔符
    int ascii_separator_x = hex_data_start_x + dataWidth; // 十六进制数据区之后
    int ascii_data_start_x = ascii_separator_x + 2;       // ASCII分隔符之后，跳过" | "

    // 总宽度 = ASCII数据起始位置 + ASCII面板宽度 + 右边距(1)
    totalWidth = ascii_data_start_x + asciiPanelWidth + 1;
}

// 构造函数
HexCliView::HexCliView(BinaryData *binData) : currentBinaryData(binData) {
    if (!currentBinaryData) {
        // 严重错误：传入了空指针。通常应由调用者确保binData有效，
        // 或者在这里抛出异常/设置错误状态，并在show()之前检查。
    }
    calculateLayout(); // 初始化布局参数
    // 初始化滚动和光标位置
    scrollOffset = 0;
    editCursorLine = 0;
    editCursorByteInLine = 0;
    editCursorNibble = 0;
    currentMode = Mode::NORMAL; // 默认进入普通模式
    dataModified = false;       // 初始数据未被修改
    commandString.clear();      // 命令字符串初始为空
}

// 析构函数，释放curses窗口资源
HexCliView::~HexCliView() {
    if (hexWin) {
        delwin(hexWin);
        hexWin = nullptr;
    }
    if (statusWin) {
        delwin(statusWin);
        statusWin = nullptr;
    }
}

// 保存文件内容到指定路径
bool HexCliView::saveFile(const std::string &filePathToSave) {
    if (filePathToSave.empty()) {
        commandString = "Error: No file name specified for saving.";
        return false;
    }
    std::ofstream file(filePathToSave, std::ios::binary | std::ios::trunc);
    if (!file) {
        commandString = "Error: Could not open file for writing: " + filePathToSave;
        return false;
    }
    file.write(reinterpret_cast<const char *>(currentBinaryData->InputImage), currentBinaryData->InputImageSize);
    if (!file) {
        commandString = "Error: Could not write data to file: " + filePathToSave;
        return false;
    }
    file.close();
    dataModified = false;                               // 保存成功后，清除修改标记
    currentBinaryData->OpenedFileName = filePathToSave; // 更新当前文件名（如果保存到新路径）
    commandString = "File saved to " + filePathToSave;
    return true;
}

// 绘制十六进制和ASCII内容到hexWin窗口
void HexCliView::drawHexContent() {
    // 计算可见数据行数
    // LINES是终端总行数。hexWin占用了LINES-1行。
    // hexWin内部：1行标题，1行顶部横线，1行底部横线。所以数据区可用行数为 (LINES-1)-3 = LINES-4。
    VISIBLE_LINES = std::max(0, LINES - 4);
    if (LINES < 4)
        VISIBLE_LINES = 0; // 终端过小时，没有数据行

    werase(hexWin); // 清除hexWin内容

    // 计算各部分的X坐标
    int addr_text_print_x = 1;
    int sep1_print_x = addressWidth + 1;
    int hex_data_print_x = addressWidth + 1 + 1;
    int hex_col_header_start_x = hex_data_print_x - 1;
    int ascii_sep_print_x = hex_data_print_x + dataWidth;
    int ascii_data_print_x = ascii_sep_print_x + 1 + 1;

    // 绘制列标题 (在hexWin的第0行)
    wattron(hexWin, A_BOLD); // 加粗
    for (int i = 0; i < BYTES_PER_LINE; i += 1) {
        mvwprintw(hexWin, 0, hex_col_header_start_x + i * 3, " %01X ", i); // 十六进制列号
    }
    mvwprintw(hexWin, 0, ascii_data_print_x, "ASCii"); // ASCII区标题
    wattroff(hexWin, A_BOLD);                          // 取消加粗

    // 绘制顶部水平分隔线 (在hexWin的第1行)
    int hline_start_x_user = sep1_print_x + 1; // 从第一个垂直分隔符之后开始
    int hline_len_user = (ascii_data_print_x + BYTES_PER_LINE - 1) - hline_start_x_user;
    if (hline_len_user < 0)
        hline_len_user = 0; // 防止长度为负
    mvwhline(hexWin, 1, hline_start_x_user, HLINE_CHAR, hline_len_user);

    // 获取总数据行数，并根据模式调整滚动使光标可见
    const int totalDataLines = (currentBinaryData->InputImageSize > 0)
        ? (static_cast<int>((currentBinaryData->InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE))
        : 0;
    if (currentMode == Mode::EDIT) {
        ensureCursorVisible(); // 编辑模式下确保光标可见
    } else {
        scrollOffset =
            std::clamp(scrollOffset, 0, std::max(0, totalDataLines - VISIBLE_LINES)); // 普通模式下限制滚动范围
    }

    // 绘制数据行 (从hexWin的第2行，即DATA_START_ROW_IN_WINDOW开始)
    constexpr int DATA_START_ROW_IN_WINDOW = 2;
    int drawn_data_row_count = 0;     // 记录实际绘制的数据行数
    for (int line_idx = scrollOffset; // line_idx是当前处理的数据在文件中的绝对行号
         line_idx < scrollOffset + VISIBLE_LINES && line_idx < totalDataLines;
         ++line_idx, ++drawn_data_row_count) {
        const INT64 currentFileOffset = static_cast<INT64>(line_idx) * BYTES_PER_LINE; // 当前行的起始字节偏移

        // 绘制地址
        std::stringstream addrStrStream;
        addrStrStream << "0x" << std::hex << std::setw(addressWidth - 3) << std::setfill('0') << currentFileOffset;
        mvwprintw(hexWin,
                  DATA_START_ROW_IN_WINDOW + drawn_data_row_count,
                  addr_text_print_x,
                  "%s",
                  addrStrStream.str().c_str());

        // 遍历当前行的每个字节位置
        for (int byte_col_idx = 0; byte_col_idx < BYTES_PER_LINE; ++byte_col_idx) {
            if (currentFileOffset + byte_col_idx < currentBinaryData->InputImageSize) { // 检查是否在文件数据范围内
                UINT8 byteValue = currentBinaryData->InputImage[currentFileOffset + byte_col_idx]; // 获取字节值

                // 检查当前字节是否是编辑光标所在位置
                bool isCursorOnThisByte =
                    (currentMode == Mode::EDIT && line_idx == editCursorLine && byte_col_idx == editCursorByteInLine);

                // 绘制十六进制表示 (两位，高低半字节)
                for (int nibble_idx = 0; nibble_idx < 2; ++nibble_idx) {
                    bool isCursorOnThisNibble = isCursorOnThisByte && (nibble_idx == editCursorNibble);
                    if (isCursorOnThisNibble)
                        wattron(hexWin, A_REVERSE); // 高亮光标下的半字节

                    char nibble_char = (nibble_idx == 0) ? "0123456789ABCDEF"[(byteValue >> 4) & 0xF] : // 高半字节
                        "0123456789ABCDEF"[byteValue & 0xF];                                            // 低半字节

                    mvwaddch(hexWin,
                             DATA_START_ROW_IN_WINDOW + drawn_data_row_count,
                             hex_data_print_x + byte_col_idx * 3 + nibble_idx,
                             nibble_char);

                    if (isCursorOnThisNibble)
                        wattroff(hexWin, A_REVERSE); // 取消高亮
                }
                // 在每两个十六进制数字后打印一个空格（除非是行尾）
                if (byte_col_idx < BYTES_PER_LINE - 1) {
                    mvwaddch(hexWin,
                             DATA_START_ROW_IN_WINDOW + drawn_data_row_count,
                             hex_data_print_x + byte_col_idx * 3 + 2,
                             ' ');
                }

                // 绘制ASCII字符表示 (可打印字符直接显示，否则显示'.')
                char displayChar = (isprint(byteValue) ? static_cast<char>(byteValue) : '.');
                mvwaddch(hexWin,
                         DATA_START_ROW_IN_WINDOW + drawn_data_row_count,
                         ascii_data_print_x + byte_col_idx,
                         displayChar);
            } else { // 如果当前位置超出文件数据范围 (行末尾的填充部分)
                mvwprintw(hexWin,
                          DATA_START_ROW_IN_WINDOW + drawn_data_row_count,
                          hex_data_print_x + byte_col_idx * 3,
                          "   "); // 打印3个空格
                mvwaddch(hexWin,
                         DATA_START_ROW_IN_WINDOW + drawn_data_row_count,
                         ascii_data_print_x + byte_col_idx,
                         ' '); // 打印1个空格
            }
        }
    }

    // 绘制状态栏上方的底部水平分隔线
    // Y坐标 = 数据起始行 + 可见数据行数
    int bottom_hline_y = DATA_START_ROW_IN_WINDOW + VISIBLE_LINES;
    // 确保此线绘制在hexWin的有效行内 (hexWin高度为LINES-1, 最大行索引为LINES-2)
    // 并且终端至少有4行高才能容纳所有界面元素
    if (bottom_hline_y < (LINES - 1) && LINES >= 4) {
        mvwhline(hexWin, bottom_hline_y, hline_start_x_user, HLINE_CHAR, hline_len_user);
    }

    // 绘制垂直分隔线 (仅当有数据且有空间绘制时)
    if (totalDataLines > 0 && VISIBLE_LINES > 0) {
        int actual_drawn_data_lines_for_vline = 0;
        if (scrollOffset < totalDataLines) { // 只有当滚动视图内有真实数据时才计算
            actual_drawn_data_lines_for_vline = std::min(VISIBLE_LINES, totalDataLines - scrollOffset);
        }

        if (actual_drawn_data_lines_for_vline > 0) { // 确保有数据行被绘制
            mvwvline(hexWin, DATA_START_ROW_IN_WINDOW, sep1_print_x, VLINE_CHAR, actual_drawn_data_lines_for_vline);
            mvwvline(
                hexWin, DATA_START_ROW_IN_WINDOW, ascii_sep_print_x, VLINE_CHAR, actual_drawn_data_lines_for_vline);
        }
    }
}

// 绘制状态栏内容到statusWin窗口
void HexCliView::drawStatusBar() {
    werase(statusWin); // 清除状态栏

    std::string modeStr; // 根据当前模式设置模式字符串
    switch (currentMode) {
    case Mode::NORMAL:
        modeStr = "-- NORMAL --";
        break;
    case Mode::EDIT:
        modeStr = "-- INSERT --";
        break; // 编辑模式在Vim中通常称为INSERT
    case Mode::COMMAND_LINE:
        modeStr = ":";
        break;
    }

    if (currentMode == Mode::COMMAND_LINE) {                      // 命令行模式下
        mvwprintw(statusWin, 0, 0, ":%s", commandString.c_str()); // 显示冒号和用户输入的命令
        wmove(statusWin, 0, 1 + commandString.length());          // 将光标移动到命令末尾
        curs_set(1);                                              // 显示终端光标
    } else {                                                      // 普通或编辑模式下
        curs_set(0);                                              // 隐藏终端光标 (我们自己绘制编辑光标)
        std::string modifiedIndicator = dataModified ? "*" : "";  // 如果数据已修改，显示星号
        std::string statusLeft = modeStr + " " + currentBinaryData->OpenedFileName + modifiedIndicator; // 左侧状态信息

        // 计算右侧状态信息（偏移、行号、大小）
        INT64 currentByteOffsetDisplay = 0;
        int currentLineDisplay = 0;
        int totalLinesDisplay = (currentBinaryData->InputImageSize > 0)
            ? (static_cast<int>((currentBinaryData->InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE))
            : 0;

        if (currentBinaryData->InputImageSize > 0) {
            if (currentMode == Mode::EDIT) { // 编辑模式下显示光标位置
                currentByteOffsetDisplay = getCursorAbsoluteOffset();
                currentLineDisplay = editCursorLine + 1; // 行号从1开始显示
            } else {                                     // 普通模式下显示当前视图顶部的偏移和行号
                currentByteOffsetDisplay = static_cast<INT64>(scrollOffset) * BYTES_PER_LINE;
                currentLineDisplay = scrollOffset + 1;
                if (totalLinesDisplay == 0)
                    currentLineDisplay = 0; // 空文件时行号为0
            }
        }

        std::stringstream rightStatus;
        rightStatus << "Offset:0x" << std::hex << std::setw(addressWidth - 3) << std::setfill('0')
                    << currentByteOffsetDisplay << "  Line:" << std::dec << currentLineDisplay << "/"
                    << totalLinesDisplay << "  Size:" << currentBinaryData->InputImageSize;

        std::string rs_str = rightStatus.str();
        mvwprintw(statusWin, 0, 0, "%s", statusLeft.c_str()); // 绘制左侧状态
        mvwprintw(
            statusWin, 0, std::max(0, totalWidth - 1 - (int) rs_str.length()), "%s", rs_str.c_str()); // 绘制右侧状态

        // 如果有临时命令字符串（如保存成功消息），且不是在命令行模式，则在中间显示
        if (!commandString.empty() && currentMode != Mode::COMMAND_LINE) {
            int mid_pos = (totalWidth - commandString.length()) / 2;    // 计算中间位置
            mid_pos = std::max((int) statusLeft.length() + 2, mid_pos); // 确保不与左侧状态重叠
            // 确保不与右侧状态重叠
            if (mid_pos + (int) commandString.length() < totalWidth - 1 - (int) rs_str.length() - 2) {
                mvwprintw(statusWin, 0, mid_pos, "%s", commandString.c_str());
            }
        }
    }
}

// 获取编辑光标相对于文件起始的绝对字节偏移
INT64 HexCliView::getCursorAbsoluteOffset() const {
    if (currentBinaryData->InputImageSize == 0)
        return 0; // 空文件偏移为0
    INT64 offset = static_cast<INT64>(editCursorLine) * BYTES_PER_LINE + editCursorByteInLine;
    // 确保偏移量在有效范围内 [0, InputImageSize - 1]
    return std::min(offset, std::max((INT64) 0, currentBinaryData->InputImageSize - 1));
}

// 调整滚动偏移，确保编辑光标在可视区域内
void HexCliView::ensureCursorVisible() {
    if (currentMode != Mode::EDIT)
        return; // 此逻辑仅用于编辑模式

    // 如果光标在当前视图上方，向上滚动
    if (editCursorLine < scrollOffset) {
        scrollOffset = editCursorLine;
    }
    // 如果光标在当前视图下方，向下滚动
    else if (editCursorLine >= scrollOffset + VISIBLE_LINES) {
        scrollOffset = editCursorLine - VISIBLE_LINES + 1;
    }

    const int totalDataLines = (currentBinaryData->InputImageSize > 0)
        ? (static_cast<int>((currentBinaryData->InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE))
        : 0;
    // 限制滚动偏移在有效范围内
    scrollOffset = std::clamp(scrollOffset, 0, std::max(0, totalDataLines - VISIBLE_LINES));
}

// 移动编辑光标的逻辑
void HexCliView::moveEditCursor(int dLine, int dByteOrNibble, bool isNibbleMove) {
    // 如果文件为空且在编辑模式，光标不应移动（除非是未来的插入功能）
    if (currentBinaryData->InputImageSize == 0 && currentMode == Mode::EDIT) {
        editCursorLine = 0;
        editCursorByteInLine = 0;
        editCursorNibble = 0;
        return;
    }

    // 根据移动类型（字节或半字节）更新光标位置
    if (!isNibbleMove) { // 按字节移动
        editCursorByteInLine += dByteOrNibble;
        editCursorLine += dLine;
        editCursorNibble = 0; // 移动到新字节时，默认定位到高半字节
    } else {                  // 按半字节移动
        editCursorNibble += dByteOrNibble;
        if (editCursorNibble > 1) { // 从低半字节前进到下一字节的高半字节
            editCursorNibble = 0;
            editCursorByteInLine++;
        } else if (editCursorNibble < 0) { // 从高半字节后退到前一字节的低半字节
            editCursorNibble = 1;
            editCursorByteInLine--;
        }
        editCursorLine += dLine; // 行变化独立处理
    }

    // 处理行内字节的换行（前进或后退）
    while (editCursorByteInLine >= BYTES_PER_LINE) {
        editCursorByteInLine -= BYTES_PER_LINE;
        editCursorLine++;
    }
    while (editCursorByteInLine < 0) {
        editCursorByteInLine += BYTES_PER_LINE;
        editCursorLine--;
    }

    // 获取总数据行数
    const int totalDataLines = (currentBinaryData->InputImageSize > 0)
        ? (static_cast<int>((currentBinaryData->InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE))
        : 0; // 空文件没有数据行用于光标定位

    if (currentBinaryData->InputImageSize == 0) { // 如果文件为空，光标固定在(0,0,0)
        editCursorLine = 0;
        editCursorByteInLine = 0;
        editCursorNibble = 0;
    } else {
        // 限制行号在有效的数据行范围内 [0, totalDataLines - 1]
        editCursorLine = std::clamp(editCursorLine, 0, std::max(0, totalDataLines - 1));

        // 限制字节在行内的偏移
        int bytesInCurrentLine = BYTES_PER_LINE;
        if (editCursorLine == totalDataLines - 1) { // 如果光标在最后一行数据上
            bytesInCurrentLine = currentBinaryData->InputImageSize % BYTES_PER_LINE;
            if (bytesInCurrentLine == 0)
                bytesInCurrentLine = BYTES_PER_LINE; // 如果文件大小刚好是BYTES_PER_LINE的整数倍
        }
        editCursorByteInLine = std::clamp(editCursorByteInLine, 0, std::max(0, bytesInCurrentLine - 1));
        // editCursorNibble 已经是0或1，由之前的逻辑保证，不需要额外clamp
    }
}

// 处理普通模式下的键盘输入
void HexCliView::handleNormalModeInput(int ch) {
    int totalDataLines = (currentBinaryData->InputImageSize > 0)
        ? (static_cast<int>((currentBinaryData->InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE))
        : 0;
    int max_scroll = std::max(0, totalDataLines - VISIBLE_LINES); // 最大滚动偏移

    switch (ch) {
    // 滚动控制
    case KEY_UP:
        scrollOffset = std::max(0, scrollOffset - 1);
        break;
    case KEY_DOWN:
        scrollOffset = std::min(max_scroll, scrollOffset + 1);
        break;
    case KEY_PPAGE:
        scrollOffset = std::max(0, scrollOffset - VISIBLE_LINES);
        break;
    case KEY_NPAGE:
        scrollOffset = std::min(max_scroll, scrollOffset + VISIBLE_LINES);
        break;
    case KEY_HOME:
        scrollOffset = 0;
        break;
    case KEY_END:
        scrollOffset = max_scroll;
        break;
    // 模式切换
    case 'i': // 进入编辑模式
        currentMode = Mode::EDIT;
        editCursorLine = scrollOffset;                // 光标初始位置为当前视图顶部
        if (currentBinaryData->InputImageSize == 0) { // 空文件特殊处理
            editCursorLine = 0;
        }
        editCursorByteInLine = 0;
        editCursorNibble = 0;
        ensureCursorVisible(); // 确保光标在屏幕上可见
        break;
    case ':': // 进入命令行模式
        currentMode = Mode::COMMAND_LINE;
        commandString.clear(); // 清空命令字符串准备接收新命令
        break;
    }
}

// 处理编辑模式下的键盘输入
void HexCliView::handleEditModeInput(int ch) {
    // 如果文件为空且不是按ESC键，则提示并阻止编辑
    if (currentBinaryData->InputImageSize == 0 && ch != 27 /* ESC */) {
        if (ch == 27) { // 如果是ESC，则退出编辑模式
            currentMode = Mode::NORMAL;
            curs_set(0);
            commandString.clear(); // 清除可能存在的提示消息
        } else {
            commandString = "File is empty. Press ESC to exit edit mode.";
        }
        return;
    }

    switch (ch) {
    case 27: // ESC键，退出编辑模式返回普通模式
        currentMode = Mode::NORMAL;
        curs_set(0);           // 隐藏终端光标
        commandString.clear(); // 清除可能存在的提示消息
        break;
    // 光标移动键
    case KEY_UP:
        moveEditCursor(-1, 0, false);
        ensureCursorVisible();
        break;
    case KEY_DOWN:
        moveEditCursor(1, 0, false);
        ensureCursorVisible();
        break;
    case KEY_LEFT:
        moveEditCursor(0, -1, false);
        ensureCursorVisible();
        break;
    case KEY_RIGHT:
        moveEditCursor(0, 1, false);
        ensureCursorVisible();
        break;
    default: {                                         // 处理十六进制字符输入
        int val = hexCharToVal(static_cast<char>(ch)); // 获取输入字符的十六进制值
        if (val != '.') {                              // 检查是否是有效的十六进制输入 (即不是'.')
            INT64 offset = getCursorAbsoluteOffset();  // 获取当前光标的绝对文件偏移
            if (offset < currentBinaryData->InputImageSize) {              // 确保在文件数据范围内
                UINT8 currentByte = currentBinaryData->InputImage[offset]; // 获取当前字节
                // 根据光标在高/低半字节位置修改对应部分
                if (editCursorNibble == 0) { // 修改高半字节
                    currentByte = (currentByte & 0x0F) | (static_cast<UINT8>(val) << 4);
                } else { // 修改低半字节
                    currentByte = (currentByte & 0xF0) | static_cast<UINT8>(val);
                }
                currentBinaryData->InputImage[offset] = currentByte; // 更新数据
                dataModified = true;                                 // 标记数据已修改
                moveEditCursor(0, 1, true);                          // 将光标移动到下一个半字节
                ensureCursorVisible();                               // 确保光标可见
            } else {
                // 此情况理论上不应发生，因为moveEditCursor会限制光标在有效数据内
                // 但作为防御性编程，可以保留一个提示
                // commandString = "End of File or empty file.";
            }
        }
        break;
    }
    }
}

// 处理命令行模式下的键盘输入
void HexCliView::handleCommandLineInput(int ch) {
    switch (ch) {
    case 27: // ESC键，退出命令行模式返回普通模式
        currentMode = Mode::NORMAL;
        commandString.clear();
        curs_set(0); // 隐藏终端光标
        break;
    case KEY_ENTER: // 回车键
    case '\n':      // 换行符 (某些终端可能发送这个)
    case '\r':      // 回车符 (某些终端可能发送这个)
        if (!commandString.empty()) {
            processCommand(); // 处理输入的命令
        } else {              // 如果只输入了冒号后直接回车，则返回普通模式
            currentMode = Mode::NORMAL;
            commandString.clear();
        }
        // processCommand可能会改变模式（例如:q成功后）或设置错误信息
        // 如果命令执行后没有错误阻止，并且模式仍是命令行，则切换回普通模式
        if (currentMode == Mode::COMMAND_LINE && commandString.find("Error:") == std::string::npos
            && commandString.find("Unsaved changes") == std::string::npos) {
            currentMode = Mode::NORMAL;
        }
        break;
    case KEY_BACKSPACE: // 退格键
    case 127:           // ASCII退格 (某些终端)
    case 8:             // ASCII退格 (另一些终端)
        if (!commandString.empty()) {
            commandString.pop_back(); // 删除命令字符串的最后一个字符
        }
        break;
    default:                                                                   // 其他可打印字符
        if (isprint(ch) && commandString.length() < (size_t) totalWidth - 7) { // 限制命令长度，留出余量
            commandString += static_cast<char>(ch);                            // 追加到命令字符串
        }
        break;
    }
}

// 解析并执行命令行模式下输入的命令
void HexCliView::processCommand() {
    std::string cmd_full = commandString;    // 完整命令
    std::string cmd_main;                    // 命令主体 (如 "q", "w", "wq")
    std::string cmd_arg;                     // 命令参数 (如文件名)
    size_t first_space = cmd_full.find(' '); // 查找第一个空格以分离参数

    if (first_space != std::string::npos) { // 如果有参数
        cmd_main = cmd_full.substr(0, first_space);
        cmd_arg = cmd_full.substr(first_space + 1);
        // 清理参数字符串两端的空白字符
        cmd_arg.erase(0, cmd_arg.find_first_not_of(" \t\n\r\f\v"));
        cmd_arg.erase(cmd_arg.find_last_not_of(" \t\n\r\f\v") + 1);
    } else { // 没有参数
        cmd_main = cmd_full;
    }

    bool quit_flag_for_main_loop = false; // 标记是否需要通知主循环退出

    if (cmd_main == "q") {  // 退出命令
        if (dataModified) { // 如果有未保存的修改
            commandString = "Unsaved changes. Use :q! to force quit, or :w to save.";
        } else { // 无修改，可以安全退出
            quit_flag_for_main_loop = true;
        }
    } else if (cmd_main == "q!") { // 强制退出命令
        quit_flag_for_main_loop = true;
    } else if (cmd_main == "w") {                                                                 // 保存命令
        std::string path_to_save = cmd_arg.empty() ? currentBinaryData->OpenedFileName : cmd_arg; // 获取保存路径
        if (path_to_save.empty()) {
            commandString = "Error: No filename.";
        } else {
            if (saveFile(path_to_save)) {   // 调用保存文件方法
                currentMode = Mode::NORMAL; // 保存成功后返回普通模式
                // commandString 已被 saveFile 设置为成功消息
            } // 如果保存失败，commandString 已被 saveFile 设置为错误消息，模式保持COMMAND_LINE
        }
    } else if (cmd_main == "wq") { // 保存并退出命令
        std::string path_to_save = cmd_arg.empty() ? currentBinaryData->OpenedFileName : cmd_arg;
        if (path_to_save.empty()) {
            commandString = "Error: :wq No filename.";
        } else {
            if (saveFile(path_to_save)) {       // 先保存
                quit_flag_for_main_loop = true; // 保存成功则标记退出
            } // 如果保存失败，commandString包含错误，不退出
        }
    } else { // 未知命令
        commandString = "Error: Unknown command: " + cmd_full;
    }

    // 如果需要退出，设置一个特殊的commandString值，由show()的主循环捕获
    if (quit_flag_for_main_loop) {
        commandString = "__QUIT_REQUESTED__";
    }
    // 如果命令已处理且没有导致模式保持在COMMAND_LINE（例如出错），
    // 则在handleCommandLineInput中，如果commandString不包含错误，会自动切回NORMAL模式。
}

// 显示编辑器界面并处理用户输入的主循环
void HexCliView::show() {
    // 创建主内容窗口和状态栏窗口
    // hexWin高度为终端总高度减1 (给状态栏留出空间)
    hexWin = newwin(LINES - 1, totalWidth, 0, 0);
    statusWin = newwin(1, totalWidth, LINES - 1, 0); // 状态栏在屏幕最底一行

    if (!hexWin || !statusWin) {
        // 创建窗口失败，通常因为终端太小。
        // 此处应有机制通知调用者（如main函数）发生错误，
        // 以便可以在curses环境清理后安全地打印错误信息。
        // 例如，可以设置一个错误状态成员变量或返回错误码。
        return;
    }
    keypad(hexWin, TRUE); // 为hexWin启用功能键（箭头、Home/End等）

    bool running = true;                   // 主循环控制标志
    int ch;                                // 存储用户输入的字符
    std::string last_status_message_check; // 用于控制状态栏临时消息的显示

    // 初始化时，如果commandString不是来自构造函数中的错误，则清空它
    if (commandString.find("Error:") == std::string::npos) {
        commandString.clear();
    }

    // 主事件循环
    while (running) {
        // 记录当前状态栏消息，用于判断下一帧是否需要清除它
        last_status_message_check = commandString;

        // 绘制界面元素
        drawHexContent();    // 绘制十六进制和ASCII数据区
        drawStatusBar();     // 绘制状态栏
        wrefresh(hexWin);    // 刷新主内容窗口到屏幕
        wrefresh(statusWin); // 刷新状态栏窗口到屏幕

        ch = getch(); // 获取用户输入 (阻塞等待)

        // 在接收到新输入后，如果当前不是命令行模式，
        // 并且旧的状态栏消息不是错误或重要提示，则清除它。
        if (currentMode != Mode::COMMAND_LINE && commandString.find("Error:") == std::string::npos
            && commandString.find("Unsaved changes") == std::string::npos) {
            commandString.clear();
        }

        // 根据当前模式处理输入
        if (currentMode == Mode::NORMAL) {
            handleNormalModeInput(ch);
            if (ch == 'q' && currentMode == Mode::NORMAL) { // 在普通模式下按'q'退出
                if (!dataModified) {                        // 如果没有未保存的修改
                    running = false;                        // 直接退出循环
                } else {                                    // 有未保存的修改，提示用户
                    commandString = "Unsaved changes. Use :q! or :w.";
                    last_status_message_check = commandString; // 确保此提示被记录以显示
                }
            }
        } else if (currentMode == Mode::EDIT) {
            handleEditModeInput(ch);
        } else if (currentMode == Mode::COMMAND_LINE) {
            handleCommandLineInput(ch);                  // 内部可能会调用processCommand
            if (commandString == "__QUIT_REQUESTED__") { // 如果命令处理结果是退出
                running = false;                         // 退出循环
                commandString.clear();                   // 清理特殊退出标记
            }
        }

        // 处理终端尺寸变化事件
        if (ch == KEY_RESIZE) {
            // 删除旧窗口
            if (hexWin) {
                delwin(hexWin);
                hexWin = nullptr;
            }
            if (statusWin) {
                delwin(statusWin);
                statusWin = nullptr;
            }

            // ncurses在SIGWINCH后，getch()或refresh()通常会更新LINES和COLS
            clear();   // 清理stdscr，为重绘做准备
            refresh(); // 刷新stdscr以应用新的终端尺寸

            calculateLayout(); // 根据新的LINES/COLS重新计算布局参数

            // 重新创建窗口
            hexWin = newwin(LINES - 1, totalWidth, 0, 0);
            statusWin = newwin(1, totalWidth, LINES - 1, 0);

            if (!hexWin || !statusWin) { // 如果重创窗口失败
                running = false;         // 退出循环
                // 此处也应有机制通知main函数错误
            } else {
                keypad(hexWin, TRUE); // 为新窗口启用功能键
                // 强制ncurses在下一轮重绘整个屏幕
                clearok(stdscr, TRUE);
            }
        }
    } // 主循环结束
}