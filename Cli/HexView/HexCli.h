//
// Created by stephan on 25-3-16.
//

#ifndef HEXCLI_H
#define HEXCLI_H

#include "BaseLib.h"
#include <curses.h>

class HexCliView {
private:
    UINT8  *InputImage{nullptr};
    INT64  InputImageSize{};
    WINDOW *hexWin{nullptr};
    int addressWidth = 6;    // 默认0x0000格式
    int dataWidth = 47;      // 16字节*3字符 -1
    int totalWidth = 56;     // addressWidth(6) + dataWidth(47) + 3(分隔)
    int scrollOffset = 0;          // 滚动偏移
    int VISIBLE_LINES = 20;  // 可视区域行数
    const int BYTES_PER_LINE = 0x10;

    void calculateLayout();
public:
    HexCliView(UINT8 *image, INT64 imageSize);
    ~HexCliView();

    void drawHex();
    void show();

};


#endif //HEXCLI_H
