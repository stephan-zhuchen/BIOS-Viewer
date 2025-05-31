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
#define VLINE '|'
#define HLINE '-'
#else
#define VLINE ACS_VLINE
#define HLINE ACS_HLINE
#endif

// 辅助函数：将字符转换为十六进制值
int hexCharToVal(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return '.'; // 非法字符返回'.'
}

void HexCliView::calculateLayout() {
    INT64 maxOffset = currentBinaryData->InputImageSize > 0 ? currentBinaryData->InputImageSize - 1 : 0;
    int hexDigits = 1;

    if (maxOffset > 0) {
        hexDigits = static_cast<int>(log2(static_cast<double>(maxOffset)) / 4) + 1;
        hexDigits = std::clamp(hexDigits, 1, 8); // 限制1-8位
    }

    // 地址列宽度："0x" + 数字 + 1空格
    addressWidth = 2 + hexDigits + 1;
    addressWidth = std::max(6, addressWidth); // 至少显示 "0x0000 "

    // 十六进制数据面板宽度：16字节 * ("XX" + 空格) = 16 * 3 = 48
    dataWidth = BYTES_PER_LINE * 3;

    // ASCII数据面板宽度：16字符
    int asciiPanelWidth = BYTES_PER_LINE;

    // 布局: [地址] | [十六进制数据] | [ASCII字符]
    // 地址区起始X: 1 (留出边框或空白)
    // 十六进制数据区起始X: addressWidth + 1 (分隔符) + 1 (空格) = addressWidth + 2 (如果地址后有空格，则是 addressWidth
    // + 1 + 1) 我们在drawHexContent中具体定位
    int hex_data_start_x = addressWidth + 3; // "0xAAAA " (addressWidth) + "|" + " " + HexData
                                             //  1        aw-1         aw aw+1 aw+2 aw+3
    int ascii_separator_x = hex_data_start_x + dataWidth;
    int ascii_data_start_x = ascii_separator_x + 2; // ...HexData + " " + "|" + " " + AsciiData

    totalWidth = ascii_data_start_x + asciiPanelWidth + 1; // +1 为了右边距
}

HexCliView::HexCliView(BinaryData *binData) : currentBinaryData(binData) {
    if (!currentBinaryData) {
        // 应该在调用show之前处理这个错误，或者让show优雅退出
        // 这里可以抛出异常或设置一个错误状态
        // 为了简单，假设binData总是有效的
    }
    calculateLayout();
    scrollOffset = 0;
    editCursorLine = 0;
    editCursorByteInLine = 0;
    editCursorNibble = 0;
    currentMode = Mode::NORMAL;
    dataModified = false;
}

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
    dataModified = false;
    currentBinaryData->OpenedFileName = filePathToSave;
    commandString = "File saved to " + filePathToSave;
    return true;
}

void HexCliView::drawHexContent() {
    // LINES是全局curses变量, 表示终端总行数
    // -1 为状态栏, -1 为hexWin自己的顶部边框/标题行
    VISIBLE_LINES = LINES - 1 - 1;
    if (VISIBLE_LINES < 1)
        VISIBLE_LINES = 1;

    werase(hexWin);

    // 定义绘制的起始X坐标 (从1开始，0是边框)
    int addr_text_print_x = 1;
    // 地址和HEX区隔 "|": addressWidth(含末尾空格) + 1(for '|')
    int sep1_print_x = addressWidth + 1; // "0x1234 | XX"
    // HEX数据区起始: addressWidth + 1(分隔符) + 1(空格)
    int hex_data_print_x = addressWidth + 1 + 1; // "0x1234 | XX"
    // HEX列头起始: hex_data_print_x -1 (为了对齐 %01X 前的空格)
    int hex_col_header_start_x = hex_data_print_x - 1;

    // ASCII区隔 "|": hex_data_print_x + dataWidth (hex区总宽度) + 1 (空格)
    int ascii_sep_print_x = hex_data_print_x + dataWidth; // "XX XX | ASCII"
    // ASCII数据区起始: ascii_sep_print_x + 1 (空格)
    int ascii_data_print_x = ascii_sep_print_x + 1 + 1; // "... | ASCII"

    // 列标题
    wattron(hexWin, A_BOLD);
    // mvwprintw(hexWin, 0, 7, " "); // 这个旧的偏移可能不再准确
    for (int i = 0; i < BYTES_PER_LINE; i += 1) {
        mvwprintw(hexWin, 0, hex_col_header_start_x + i * 3, " %01X ", i);
    }
    mvwprintw(hexWin, 0, ascii_data_print_x, "Character"); // "Characters" -> "字符"
    wattroff(hexWin, A_BOLD);

    // 顶部分隔线 (从地址列后开始，到ASCII列尾)
    int hline_start_x = sep1_print_x + 1; // 从第一个分隔符后开始
    int hline_len = (ascii_data_print_x + BYTES_PER_LINE - 1) - hline_start_x;
    mvwhline(hexWin, 1, hline_start_x, HLINE, hline_len);

    // 计算总数据行数并确保光标可见 (这会调整scrollOffset)
    const int totalDataLines =
            (currentBinaryData->InputImageSize > 0)
                    ? (static_cast<int>((currentBinaryData->InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE))
                    : 0;
    if (currentMode == Mode::EDIT) { // 只有编辑模式才强制光标可见
        ensureCursorVisible();
    } else { // 普通模式下，clamp scrollOffset
        scrollOffset = std::clamp(scrollOffset, 0, std::max(0, totalDataLines - VISIBLE_LINES));
    }

    // 数据行绘制
    int draw_y_in_win = 0; // 在hexWin中的相对绘制行号 (从0开始)
    for (int line_idx = scrollOffset; // line_idx 是绝对数据行号
         line_idx < scrollOffset + VISIBLE_LINES && line_idx < totalDataLines; ++line_idx, ++draw_y_in_win) {
        constexpr int DATA_START_ROW_IN_WINDOW = 2; // 数据从窗口的第2行开始绘制 (0是列头, 1是分隔线)
        const INT64 currentFileOffset = static_cast<INT64>(line_idx) * BYTES_PER_LINE;

        // 地址显示 (addressWidth 包括 "0x", 数字, 和末尾的一个空格)
        // setw作用于数字部分
        std::stringstream addrStrStream;
        addrStrStream << "0x" << std::hex << std::setw(addressWidth - 2 - 1) << std::setfill('0') << currentFileOffset;
        mvwprintw(hexWin, DATA_START_ROW_IN_WINDOW + draw_y_in_win, addr_text_print_x, "%s",
                  addrStrStream.str().c_str());

        std::string asciiDisplayStr;
        asciiDisplayStr.reserve(BYTES_PER_LINE);

        int validBytesInLine =
                std::min(static_cast<INT64>(BYTES_PER_LINE), currentBinaryData->InputImageSize - currentFileOffset);

        for (int byte_col_idx = 0; byte_col_idx < BYTES_PER_LINE; ++byte_col_idx) {
            if (byte_col_idx < validBytesInLine) {
                UINT8 byteValue = currentBinaryData->InputImage[currentFileOffset + byte_col_idx];

                bool isCursorOnThisByte = (currentMode == Mode::EDIT && line_idx == editCursorLine &&
                                           byte_col_idx == editCursorByteInLine);

                // 绘制十六进制字节 (每个字节XX占2个字符，后跟1个空格)
                for (int nibble_idx = 0; nibble_idx < 2; ++nibble_idx) { // 0: 高位, 1: 低位
                    bool isCursorOnThisNibble = isCursorOnThisByte && (nibble_idx == editCursorNibble);
                    if (isCursorOnThisNibble)
                        wattron(hexWin, A_REVERSE); // 反色显示光标

                    char nibble_char;
                    if (nibble_idx == 0)
                        nibble_char = "0123456789ABCDEF"[(byteValue >> 4) & 0xF];
                    else
                        nibble_char = "0123456789ABCDEF"[byteValue & 0xF];

                    mvwaddch(hexWin, DATA_START_ROW_IN_WINDOW + draw_y_in_win,
                             hex_data_print_x + byte_col_idx * 3 + nibble_idx, nibble_char);

                    if (isCursorOnThisNibble)
                        wattroff(hexWin, A_REVERSE);
                }
                // 在 "XX" 后打印空格 (除非是最后一个字节且不需要额外空格)
                if (byte_col_idx < BYTES_PER_LINE - 1) {
                    mvwaddch(hexWin, DATA_START_ROW_IN_WINDOW + draw_y_in_win, hex_data_print_x + byte_col_idx * 3 + 2,
                             ' ');
                }

                // ASCII 字符
                char displayChar = (isprint(byteValue) ? static_cast<char>(byteValue) : '.');
                mvwaddch(hexWin, DATA_START_ROW_IN_WINDOW + draw_y_in_win, ascii_data_print_x + byte_col_idx,
                         displayChar);
            } else { // 如果行不满，用空格填充剩余部分
                mvwprintw(hexWin, DATA_START_ROW_IN_WINDOW + draw_y_in_win, hex_data_print_x + byte_col_idx * 3,
                          "   "); // 3 spaces for "XX "
                mvwaddch(hexWin, DATA_START_ROW_IN_WINDOW + draw_y_in_win, ascii_data_print_x + byte_col_idx, ' ');
            }
        }
    }

    // 绘制垂直分隔线 (只在有数据行时绘制)
    if (totalDataLines > 0) {
        constexpr int DATA_START_ROW_IN_WINDOW = 2;
        int actual_drawn_data_lines = std::min(VISIBLE_LINES, totalDataLines - scrollOffset);
        if (actual_drawn_data_lines > 0) { // 确保至少有一行数据被绘制
            mvwvline(hexWin, DATA_START_ROW_IN_WINDOW, sep1_print_x, VLINE, actual_drawn_data_lines);
            mvwvline(hexWin, DATA_START_ROW_IN_WINDOW, ascii_sep_print_x, VLINE, actual_drawn_data_lines);
        }
    }
    // wrefresh(hexWin); // show() 中的主循环会刷新
}

void HexCliView::drawStatusBar() {
    werase(statusWin);
    // box(statusWin,0,0); // 可选的状态栏边框

    std::string modeStr;
    switch (currentMode) {
        case Mode::NORMAL:
            modeStr = "-- NORMAL --";
            break;
        case Mode::EDIT:
            modeStr = "-- INSERT --";
            break;
        case Mode::COMMAND_LINE:
            modeStr = ":";
            break;
    }

    if (currentMode == Mode::COMMAND_LINE) {
        mvwprintw(statusWin, 0, 0, ":%s", commandString.c_str());
        wmove(statusWin, 0, 1 + commandString.length()); // 移动光标到冒号后
        curs_set(1); // 显示终端光标
    } else {
        curs_set(0); // 在普通/编辑模式隐藏终端光标 (我们自己绘制编辑光标)
        std::string modifiedIndicator = dataModified ? "*" : "";
        std::string statusLeft = modeStr + " " + currentBinaryData->OpenedFileName + modifiedIndicator;

        INT64 currentByteOffsetDisplay = 0;
        int currentLineDisplay = 0;
        int totalLinesDisplay =
                (currentBinaryData->InputImageSize > 0)
                        ? (static_cast<int>((currentBinaryData->InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE))
                        : 0;

        if (currentBinaryData->InputImageSize > 0) {
            if (currentMode == Mode::EDIT) {
                currentByteOffsetDisplay = getCursorAbsoluteOffset();
                currentLineDisplay = editCursorLine + 1; // 1-based for display
            } else { // NORMAL mode
                currentByteOffsetDisplay = static_cast<INT64>(scrollOffset) * BYTES_PER_LINE;
                currentLineDisplay = scrollOffset + 1; // Show top of visible screen
                if (totalLinesDisplay == 0)
                    currentLineDisplay = 0;
            }
        }

        std::stringstream rightStatus;
        rightStatus << "Offset:0x" << std::hex << std::setw(addressWidth - 3) << std::setfill('0')
                    << currentByteOffsetDisplay << "  line:" << std::dec << currentLineDisplay << "/"
                    << totalLinesDisplay << "  size:" << currentBinaryData->InputImageSize;

        std::string rs_str = rightStatus.str();
        mvwprintw(statusWin, 0, 0, "%s", statusLeft.c_str());
        mvwprintw(statusWin, 0, std::max(0, totalWidth - 1 - (int) rs_str.length()), "%s", rs_str.c_str());

        // 如果有短时消息 (非错误，非命令输入)
        if (!commandString.empty() && currentMode != Mode::COMMAND_LINE) {
            // 确保不覆盖左边或右边的状态
            int mid_pos = (totalWidth - commandString.length()) / 2;
            mid_pos = std::max((int) statusLeft.length() + 2, mid_pos);
            if (mid_pos + (int) commandString.length() < totalWidth - 1 - (int) rs_str.length() - 2) {
                mvwprintw(statusWin, 0, mid_pos, "%s", commandString.c_str());
            }
        }
    }
    // wrefresh(statusWin); // show() 中的主循环会刷新
}

INT64 HexCliView::getCursorAbsoluteOffset() const {
    if (currentBinaryData->InputImageSize == 0)
        return 0;
    INT64 offset = static_cast<INT64>(editCursorLine) * BYTES_PER_LINE + editCursorByteInLine;
    return std::min(offset, currentBinaryData->InputImageSize - 1); // 确保不越界
}

void HexCliView::ensureCursorVisible() {
    if (currentMode != Mode::EDIT)
        return; // 只在编辑模式下强制

    // 确保 editCursorLine 在 scrollOffset 和 scrollOffset + VISIBLE_LINES - 1 之间
    if (editCursorLine < scrollOffset) {
        scrollOffset = editCursorLine;
    } else if (editCursorLine >= scrollOffset + VISIBLE_LINES) {
        scrollOffset = editCursorLine - VISIBLE_LINES + 1;
    }
    // 再次 clamp scrollOffset 以防万一
    const int totalDataLines =
            (currentBinaryData->InputImageSize > 0)
                    ? (static_cast<int>((currentBinaryData->InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE))
                    : 0;
    scrollOffset = std::clamp(scrollOffset, 0, std::max(0, totalDataLines - VISIBLE_LINES));
}

void HexCliView::moveEditCursor(int dLine, int dByteOrNibble, bool isNibbleMove) {
    if (currentBinaryData->InputImageSize == 0 && !(dLine == 0 && dByteOrNibble == 0))
        return;

    if (!isNibbleMove) { // 按字节移动 (通常是方向键)
        editCursorByteInLine += dByteOrNibble;
        editCursorLine += dLine;
        editCursorNibble = 0; // 移动到字节时，默认在高半位
    } else { // 按半字节移动 (通常是输入十六进制字符后)
        editCursorNibble += dByteOrNibble; // dByteOrNibble 此时代表半字节的移动 (+1 or -1)
        if (editCursorNibble > 1) { // 从低半位前进到高半位 (下一字节)
            editCursorNibble = 0;
            editCursorByteInLine++;
        } else if (editCursorNibble < 0) { // 从高半位后退到低半位 (前一字节)
            editCursorNibble = 1;
            editCursorByteInLine--;
        }
        editCursorLine += dLine; // dLine 仍然是行的变化
    }

    // 处理行内字节的溢出/不足
    while (editCursorByteInLine >= BYTES_PER_LINE) {
        editCursorByteInLine -= BYTES_PER_LINE;
        editCursorLine++;
    }
    while (editCursorByteInLine < 0) {
        editCursorByteInLine += BYTES_PER_LINE;
        editCursorLine--;
    }

    const int totalDataLines =
            (currentBinaryData->InputImageSize > 0)
                    ? (static_cast<int>((currentBinaryData->InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE))
                    : 1; // 至少为1行（即使是空文件，逻辑上也有第0行）

    // 限制行号
    editCursorLine = std::clamp(editCursorLine, 0, std::max(0, totalDataLines - 1));

    // 如果在最后一行，限制字节列号
    if (editCursorLine == totalDataLines - 1 && currentBinaryData->InputImageSize > 0) {
        int bytesInLastLine = currentBinaryData->InputImageSize % BYTES_PER_LINE;
        if (bytesInLastLine == 0)
            bytesInLastLine = BYTES_PER_LINE; // 如果刚好整除
        editCursorByteInLine = std::clamp(editCursorByteInLine, 0, std::max(0, bytesInLastLine - 1));
        // 如果光标试图移到最后一个有效字节的低半位之后，则将其移回最后一个有效字节的低半位
        if (editCursorByteInLine == bytesInLastLine - 1 && editCursorNibble > 1) {
            editCursorNibble = 1;
        } else if (editCursorByteInLine >= bytesInLastLine) { // 如果不小心超出了最后一个字节
            editCursorByteInLine = std::max(0, bytesInLastLine - 1);
            editCursorNibble = 0; // 或1，取决于你想要的行为
        }
    } else if (currentBinaryData->InputImageSize == 0) { // 空文件特殊处理
        editCursorLine = 0;
        editCursorByteInLine = 0;
        editCursorNibble = 0;
    }
    // ensureCursorVisible(); // 调用者通常会调用，或者在drawHexContent中调用
}

void HexCliView::handleNormalModeInput(int ch) {
    // commandString = ""; // 清除上一条非持久消息
    int totalDataLines =
            (currentBinaryData->InputImageSize > 0)
                    ? (static_cast<int>((currentBinaryData->InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE))
                    : 0;
    int max_scroll = std::max(0, totalDataLines - VISIBLE_LINES);

    switch (ch) {
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
        case 'i':
            if (currentBinaryData->InputImageSize > 0 || totalDataLines > 0) { // 允许在空文件（逻辑上的第一行）开始编辑
                currentMode = Mode::EDIT;
                // 将编辑光标定位到当前屏幕的左上角或合理位置
                editCursorLine = scrollOffset;
                editCursorByteInLine = 0;
                editCursorNibble = 0;
                ensureCursorVisible(); // 确保光标可见并进行必要的clamp
            } else {
                commandString = "File is empty, cannot enter edit mode.";
            }
            break;
        case ':':
            currentMode = Mode::COMMAND_LINE;
            commandString.clear(); // 准备接收命令
            break;
    }
}

void HexCliView::handleEditModeInput(int ch) {
    // commandString = "";
    if (currentBinaryData->InputImageSize == 0 && ch != 27 /* ESC */) {
        if (ch == 27) {
            currentMode = Mode::NORMAL;
            curs_set(0);
        }
        commandString = "File is empty. Press ESC to exit edit mode.";
        return;
    }

    switch (ch) {
        case 27: // ESC key
            currentMode = Mode::NORMAL;
            curs_set(0);
            break;
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
            break; // 移动到前一个字节
        case KEY_RIGHT:
            moveEditCursor(0, 1, false);
            ensureCursorVisible();
            break; // 移动到后一个字节
        // 可以增加 Ctrl+Left/Right 来按半字节移动
        default: {
            int val = hexCharToVal(static_cast<char>(ch));
            INT64 offset = getCursorAbsoluteOffset();
            if (offset < currentBinaryData->InputImageSize) { // 确保在文件范围内
                UINT8 currentByte = currentBinaryData->InputImage[offset];
                if (editCursorNibble == 0) { // 修改高半位
                    currentByte = (currentByte & 0x0F) | (static_cast<UINT8>(val) << 4);
                } else { // 修改低半位
                    currentByte = (currentByte & 0xF0) | static_cast<UINT8>(val);
                }
                currentBinaryData->InputImage[offset] = currentByte;
                dataModified = true;
                moveEditCursor(0, 1, true); // 移动到下一个半字节
                ensureCursorVisible();
            } else {
                commandString = "End of File";
            }
            break;
        }
    }
}

void HexCliView::handleCommandLineInput(int ch) {
    switch (ch) {
        case 27: // ESC key
            currentMode = Mode::NORMAL;
            commandString.clear();
            curs_set(0);
            break;
        case KEY_ENTER:
        case '\n':
        case '\r':
            if (!commandString.empty()) {
                processCommand(); // 处理命令, processCommand会更新commandString作为结果/错误信息
            } else { // 如果只输入了冒号然后回车
                currentMode = Mode::NORMAL; // 返回普通模式
                commandString.clear();
            }
            // processCommand 可能会改变 currentMode (例如，如果命令是 :q 且成功)
            // 如果命令执行后仍在COMMAND_LINE模式（通常是出错），状态栏会显示错误
            // 否则，通常会回到NORMAL模式
            if (currentMode == Mode::COMMAND_LINE && commandString.find("Error:") == std::string::npos &&
                commandString.find("Unsaved") == std::string::npos) {
                // 如果没有特定错误信息阻止，则返回NORMAL模式
                currentMode = Mode::NORMAL;
            }
            break;
        case KEY_BACKSPACE:
        case 127: // ASCII backspace
        case 8: // ASCII backspace
            if (!commandString.empty()) {
                commandString.pop_back();
            }
            break;
        default:
            if (isprint(ch) && commandString.length() < (size_t) totalWidth - 2 - 5) { // -2 for ':', -5 for some margin
                commandString += static_cast<char>(ch);
            }
            break;
    }
}

void HexCliView::processCommand() {
    std::string cmd_full = commandString;
    std::string cmd_main;
    std::string cmd_arg;
    size_t first_space = cmd_full.find(' ');

    if (first_space != std::string::npos) {
        cmd_main = cmd_full.substr(0, first_space);
        cmd_arg = cmd_full.substr(first_space + 1);
        // 清理参数前后的空格
        cmd_arg.erase(0, cmd_arg.find_first_not_of(" \t\n\r\f\v"));
        cmd_arg.erase(cmd_arg.find_last_not_of(" \t\n\r\f\v") + 1);
    } else {
        cmd_main = cmd_full;
    }

    bool quit_flag_for_main_loop = false; // 用于通知show()的主循环退出

    if (cmd_main == "q") {
        if (dataModified) {
            commandString = "Unsaved changes. Use :q! to force quit, or :w to save.";
            // currentMode 保持 COMMAND_LINE 以显示此消息
        } else {
            quit_flag_for_main_loop = true;
        }
    } else if (cmd_main == "q!") {
        quit_flag_for_main_loop = true;
    } else if (cmd_main == "w") {
        std::string path_to_save = cmd_arg.empty() ? currentBinaryData->OpenedFileName : cmd_arg;
        if (path_to_save.empty()) {
            commandString = "Error: No filename.";
        } else {
            if (saveFile(path_to_save)) {
                // commandString 由 saveFile 设置成功消息
                currentMode = Mode::NORMAL; // 保存成功后返回Normal模式
            } // else commandString 由 saveFile 设置错误消息, currentMode 保持 COMMAND_LINE
        }
    } else if (cmd_main == "wq") {
        std::string path_to_save = cmd_arg.empty() ? currentBinaryData->OpenedFileName : cmd_arg;
        if (path_to_save.empty()) {
            commandString = "Error: :wq No filename.";
        } else {
            if (saveFile(path_to_save)) {
                quit_flag_for_main_loop = true; // 保存成功，准备退出
            } // else commandString 由 saveFile 设置错误消息
        }
    } else {
        commandString = "Error: Unknown command: " + cmd_full;
        // currentMode 保持 COMMAND_LINE
    }

    if (quit_flag_for_main_loop) {
        // 如何通知主循环？这是一个问题。
        // 可以在 HexCliView 中设置一个bool quit_requested = true;
        // show()的主循环检查这个标志。
        // 为了简单，我们假设 `show()` 中的 ch 被设置为一个特殊值，或者 `running` 标志被设置。
        // 这里，我们直接修改 commandString 以便 show() 可以识别并退出。
        // 这是一个 hacky 的方式，更好的方式是使用一个专门的退出标志。
        commandString = "__QUIT_REQUESTED__"; // 特殊标记
    } else if (currentMode != Mode::COMMAND_LINE) { // 如果命令已处理且没有错误阻止模式切换
                                                    // commandString 可能已被设为成功消息，如 "文件已保存"
                                                    // 此时应该返回NORMAL模式，状态栏会显示这个消息一小段时间
    }
}

void HexCliView::show() {
    // 创建窗口
    // Hex 内容窗口高度为终端高度 - 1 (给状态栏)
    hexWin = newwin(LINES - 1, totalWidth, 0, 0);
    statusWin = newwin(1, totalWidth, LINES - 1, 0);

    if (!hexWin || !statusWin) {
        return; // 通知调用者失败
    }
    keypad(hexWin, TRUE); // 为 hexWin 启用功能键
    // keypad(statusWin, TRUE); // 状态栏通常不直接接收复杂输入

    bool running = true;
    int ch;
    std::string last_status_message; // 用于临时消息的显示控制

    while (running) {
        // 在绘制前，清除上一帧的临时状态消息 (如果它不是错误消息)
        if (currentMode != Mode::COMMAND_LINE) {
            if (!last_status_message.empty() && last_status_message.find("Error:") == std::string::npos &&
                last_status_message.find("Unsaved") == std::string::npos &&
                last_status_message.find("__QUIT_REQUESTED__") == std::string::npos) {
                commandString.clear(); // 清除非持久性消息
            }
        }
        last_status_message = commandString; // 记录当前消息以备下一帧比较

        drawHexContent();
        drawStatusBar();
        wrefresh(hexWin); // 单独刷新每个窗口
        wrefresh(statusWin);

        ch = getch(); // 等待输入

        if (currentMode == Mode::NORMAL) {
            handleNormalModeInput(ch);
            if (ch == 'q' && currentMode == Mode::NORMAL) { // 在普通模式下按 'q'
                if (!dataModified) {
                    running = false; // 直接退出
                } else {
                    commandString = "Unsaved changes. Use :q! or :w.";
                    last_status_message = commandString; // 确保显示
                }
            }
        } else if (currentMode == Mode::EDIT) {
            handleEditModeInput(ch);
        } else if (currentMode == Mode::COMMAND_LINE) {
            handleCommandLineInput(ch); // 内部会调用 processCommand
            if (commandString == "__QUIT_REQUESTED__") {
                running = false;
                commandString.clear(); // 清理特殊标记
            }
        }

        if (ch == KEY_RESIZE) {
            // ncurses 通常会自动处理 SIGWINCH 并更新 LINES 和 COLS
            // 我们需要删除旧窗口，重新计算布局，创建新窗口

            delwin(hexWin);
            hexWin = nullptr;
            delwin(statusWin);
            statusWin = nullptr;

            // ncurses的refresh()或getch()在SIGWINCH后会更新LINES/COLS
            // 但有时需要 endwin(), refresh(), 然后重新 initscr() 和窗口。
            // 为了简单起见，这里假设LINES/COLS已更新。
            // 如果遇到问题，标准的resize处理是:
            // endwin(); initscr(); /* re-init colors, cbreak etc. */; clear(); refresh();

            // 重新计算布局
            calculateLayout();

            // 重新创建窗口
            hexWin = newwin(LINES - 1, totalWidth, 0, 0);
            statusWin = newwin(1, totalWidth, LINES - 1, 0);

            if (!hexWin || !statusWin) {
                // 如果调整后窗口还是无法创建，则退出
                running = false; // 应该通知上层出错了
                // endwin(); // 如果这里负责initscr
                // std::cerr << "错误: 终端调整后尺寸过小。" << std::endl;
            } else {
                keypad(hexWin, TRUE);
                clear(); // 清理屏幕，因为 LINES/COLS 可能变化很大
                refresh(); // 刷新 stdscr
                // 下一轮循环会自动重绘所有内容
            }
        }
    }
}
