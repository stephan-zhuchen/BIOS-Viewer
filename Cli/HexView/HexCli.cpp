//
// Created by stephan on 25-3-16.
//

#include "HexCli.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cmath>
#include <filesystem>
#include <functional>

void HexCliView::calculateLayout() {
    // 计算需要的地址位数
    INT64 maxOffset = InputImageSize > 0 ? InputImageSize - 1 : 0;
    int hexDigits = 1;

    if(maxOffset > 0) {
        hexDigits = static_cast<int>(log2(maxOffset) / 4) + 1;
        hexDigits = std::clamp(hexDigits, 1, 8); // 限制1-8位
    }

    // 计算地址列宽度："0x" + 数字 + 1空格
    addressWidth = 2 + hexDigits + 1;
    addressWidth = std::max(6, addressWidth); // 至少显示0x0000

    // 数据列固定格式：16字节*(2数字+1空格)  = 48
    dataWidth = 48;
    totalWidth = addressWidth + dataWidth + 3;
}

HexCliView::HexCliView(UINT8 *image, INT64 imageSize):InputImage(image), InputImageSize(imageSize) {
    calculateLayout();
}

HexCliView::~HexCliView() {
    std::cout << "HexCliView::~HexCliView" << std::endl;
    if(hexWin) delwin(hexWin);
    endwin();
}

void HexCliView::drawHex() {
    VISIBLE_LINES = LINES - 4;
    werase(hexWin);

    // 列标题（保持与之前一致）
    wattron(hexWin, A_BOLD);
    mvwprintw(hexWin, 0, 7, " ");
    for(int i = 0; i < BYTES_PER_LINE; i += 1) {
        mvwprintw(hexWin, 0, addressWidth + 3 + i*3, " %01X ", i);
    }
    wattroff(hexWin, A_BOLD);

    // 分隔线
    mvwhline(hexWin, 1, addressWidth + 3, '-', totalWidth - 6);

    // 计算显示范围
    const int totalLines = (InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE;
    const int maxScroll = std::max(0, totalLines - VISIBLE_LINES);
    scrollOffset = std::clamp(scrollOffset, 0, maxScroll);

    // 数据行绘制
    int drawLine = 0;
    for(int line = scrollOffset;
        line < scrollOffset + VISIBLE_LINES && line < totalLines;
        ++line, ++drawLine)
    {
        constexpr int DATA_START_ROW = 2;
        const INT64 offset = line * BYTES_PER_LINE;

        // 地址显示
        std::stringstream addr;
        addr << "0x" << std::hex << std::setw(addressWidth-2)
             << std::setfill('0') << offset;
        mvwprintw(hexWin, DATA_START_ROW + drawLine, 1, "%s", addr.str().c_str());
        mvwvline(hexWin, DATA_START_ROW, addressWidth + 2, '|', VISIBLE_LINES);

        // 十六进制数据
        std::stringstream hexStr;
        int validBytes = std::min(BYTES_PER_LINE,
                             static_cast<int>(InputImageSize - offset));

        for(int i=0; i<validBytes; ++i) {
            hexStr << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                   << static_cast<int>(InputImage[offset+i]) << " ";
        }

        mvwprintw(hexWin, DATA_START_ROW + drawLine, addressWidth + 4, "%s", hexStr.str().c_str());
    }

    // 底部状态
    mvwhline(hexWin, LINES - 2, addressWidth + 3, '-', totalWidth - 6);
    mvwprintw(hexWin, LINES-1, 2,
            "Offset: 0x%04X  Use Up/Down to scroll  Q:Exit",
            scrollOffset * BYTES_PER_LINE);
}

void HexCliView::show() {
    initscr();
#ifdef _WIN32
    // Windows保持原样
#else
    setlocale(LC_ALL, ""); // Linux下支持Unicode
    start_color();         // 必须调用才能使用颜色属性
    use_default_colors();  // 使用终端默认颜色
#endif
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    refresh();

    hexWin = newwin(LINES, totalWidth, 0, 0);
    int ch;

    do {
        drawHex();
        wrefresh(hexWin);
        ch = getch();

        switch(ch) {
            case KEY_UP:
                scrollOffset = std::max(0, scrollOffset - 1);
            break;
            case KEY_DOWN: {
                int totalLines = (InputImageSize + 0x0F) / 0x10;
                if(scrollOffset < totalLines - VISIBLE_LINES)
                    ++scrollOffset;
                break;
            }
            case 'Q':
            case 'q':
                ch = 'q';
            break;
        }
    } while(ch != 'q');
}