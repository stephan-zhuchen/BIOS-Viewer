//
// Created by stephan on 25-3-16.
//

#include "HexCli.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>


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
    dataWidth = BYTES_PER_LINE * 3;

    // ASCII data panel width (fixed): 16 characters
    int asciiPanelWidth = BYTES_PER_LINE;
    int hex_data_start_x = addressWidth + 4;

    // ASCII separator '|' is after hex data panel.
    int ascii_separator_x = hex_data_start_x + dataWidth;

    // ASCII data starts after separator and a space.
    int ascii_data_start_x = ascii_separator_x + 2; // | Space ASCII

    // totalWidth = addressWidth + dataWidth + 3;
    totalWidth = ascii_data_start_x + asciiPanelWidth + 1;
}

HexCliView::HexCliView(UINT8 *image, INT64 imageSize):InputImage(image), InputImageSize(imageSize) {
    calculateLayout();
}

HexCliView::~HexCliView() {
    // std::cout << "HexCliView::~HexCliView" << std::endl;
    if (hexWin)
    {
        delwin(hexWin);
        hexWin = nullptr;
    }
    if (isendwin() == FALSE)
    { // Check if endwin has already been called
        endwin();
    }
}

void HexCliView::drawHex() {
    VISIBLE_LINES = LINES - 4;
    if (VISIBLE_LINES < 1) {
        VISIBLE_LINES = 1;
    }

    werase(hexWin);

    // Define column start positions for drawing (consistent with calculateLayout)
    int addr_text_print_x = 1;
    int sep1_print_x = addressWidth + 2;
    int hex_col_header_start_x = addressWidth + 3; // For " 0 ", " 1 ", ...
    int hex_data_print_x = addressWidth + 4;       // For "XX " data
    int ascii_sep_print_x = hex_data_print_x + dataWidth;
    int ascii_data_print_x = ascii_sep_print_x + 2;

    // 列标题（保持与之前一致）
    wattron(hexWin, A_BOLD);
    mvwprintw(hexWin, 0, 7, " ");
    for (int i = 0; i < BYTES_PER_LINE; ++i) {
        mvwprintw(hexWin, 0, hex_col_header_start_x + i*3, " %01X ", i);
    }
    // ASCII column header
    mvwprintw(hexWin, 0, ascii_data_print_x, "Characters");
    wattroff(hexWin, A_BOLD);

    // Top Separator Line (hline)
    int hline_start_x = hex_col_header_start_x;
    int hline_end_x = ascii_data_print_x + BYTES_PER_LINE - 1;
    mvwhline(hexWin, 1, hline_start_x, ACS_HLINE, hline_end_x - hline_start_x + 1);

    // Calculate display range
    const int totalLines = (InputImageSize > 0) ? (static_cast<int>((InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE)) : 0;
    const int maxScroll = std::max(0, totalLines - VISIBLE_LINES);
    scrollOffset = std::clamp(scrollOffset, 0, maxScroll);

    // Data Rows
    int drawLine = 0; // Relative line number in the visible window area
    for (int line_idx = scrollOffset;
         line_idx < scrollOffset + VISIBLE_LINES && line_idx < totalLines;
         ++line_idx, ++drawLine)
    {
        constexpr int DATA_START_ROW_IN_WINDOW = 2; // Start drawing data from 3rd row (0-indexed)
        const INT64 currentOffset = static_cast<INT64>(line_idx) * BYTES_PER_LINE;

        // Address Display
        std::stringstream addrStrStream;
        // addressWidth includes "0x", hex digits for offset, and 1 trailing space.
        // The setw here is for the hex digit part only.
        addrStrStream << "0x" << std::hex << std::setw(addressWidth - 2 - 1) // setw for hexDigits part
                      << std::setfill('0') << currentOffset;
        mvwprintw(hexWin, DATA_START_ROW_IN_WINDOW + drawLine, addr_text_print_x, "%s", addrStrStream.str().c_str());

        // Hexadecimal Data
        std::stringstream hexDataStrStream;
        std::string asciiDisplayStr;
        asciiDisplayStr.reserve(BYTES_PER_LINE);

        int validBytesInLine = std::min(static_cast<INT64>(BYTES_PER_LINE),
                                        InputImageSize - currentOffset);

        for (int i = 0; i < validBytesInLine; ++i)
        {
            UINT8 byteValue = InputImage[currentOffset + i];
            // Hex part
            hexDataStrStream << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                             << static_cast<int>(byteValue) << " ";
            // ASCII part
            asciiDisplayStr += (isprint(byteValue) ? static_cast<char>(byteValue) : '.');
        }

        mvwprintw(hexWin, DATA_START_ROW_IN_WINDOW + drawLine, hex_data_print_x, "%s", hexDataStrStream.str().c_str());
        mvwprintw(hexWin, DATA_START_ROW_IN_WINDOW + drawLine, ascii_data_print_x, "%s", asciiDisplayStr.c_str());
    }

    // Draw vertical separator lines only once if they span multiple rows
    // Or, ensure they are drawn for the number of actual data lines if VISIBLE_LINES changes dynamically.
    // For simplicity, draw them based on VISIBLE_LINES (max possible data lines shown)
    if (totalLines > 0)
    { // Only draw if there's data
        constexpr int DATA_START_ROW_IN_WINDOW = 2;
        mvwvline(hexWin, DATA_START_ROW_IN_WINDOW, sep1_print_x, ACS_VLINE, std::min(VISIBLE_LINES, totalLines - scrollOffset));
        mvwvline(hexWin, DATA_START_ROW_IN_WINDOW, ascii_sep_print_x, ACS_VLINE, std::min(VISIBLE_LINES, totalLines - scrollOffset));
    }

    // Bottom Status Separator Line
    mvwhline(hexWin, LINES - 2, hline_start_x, ACS_HLINE, hline_end_x - hline_start_x + 1);

    // Bottom Status Text
    mvwprintw(hexWin, LINES - 1, 2,
              "Offset: 0x%0*X Lines: %d/%d | Use Up/Down to scroll | Q:Exit",
              addressWidth - 3, // Width for offset hex number part
              scrollOffset * BYTES_PER_LINE,
              scrollOffset + std::min(VISIBLE_LINES, totalLines - scrollOffset), totalLines);
}

void HexCliView::show()
{
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

    // Create hex window
    // Ensure totalWidth is at least some minimum if calculated very small
    if (totalWidth < 20)
        totalWidth = 20; // Arbitrary minimum
    if (LINES < 5)
    { // Arbitrary minimum lines
        endwin();
        std::cerr << "Terminal too small." << std::endl;
        return;
    }

    hexWin = newwin(LINES, totalWidth, 0, 0);
    if (!hexWin) {
        endwin();
        std::cerr << "Error creating window. Terminal might be too small for calculated width: " << totalWidth << std::endl;
        return;
    }

    int ch;
    do {
        drawHex();
        wrefresh(hexWin);
        ch = getch(); // Get input (blocking by default)

        switch(ch) {
            case KEY_UP:
                scrollOffset = std::max(0, scrollOffset - 1);
                break;
            case KEY_DOWN: {
                // (InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE gives total lines of data
                int numTotalDataLines = (InputImageSize > 0) ? (static_cast<int>((InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE)) : 0;
                if (numTotalDataLines > 0 && scrollOffset < numTotalDataLines - VISIBLE_LINES) {
                     // Ensure scrollOffset doesn't go beyond max possible while allowing last page to be full
                    if (scrollOffset < std::max(0, numTotalDataLines - VISIBLE_LINES) ) {
                         ++scrollOffset;
                    }
                } else if (numTotalDataLines <= VISIBLE_LINES) { // Not enough lines to scroll
                    scrollOffset = 0;
                } else { // At the last possible scroll position
                     scrollOffset = std::max(0, numTotalDataLines - VISIBLE_LINES);
                }

                // Simpler logic for KEY_DOWN, clamp will handle it in drawHex
                // int total_data_lines = (InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE;
                // int max_scroll = std::max(0, total_data_lines - VISIBLE_LINES);
                // if (scrollOffset < max_scroll) {
                //    scrollOffset++;
                // }
                break;
            }
            case KEY_PPAGE: // Page Up
                 scrollOffset = std::max(0, scrollOffset - VISIBLE_LINES);
                 break;
            case KEY_NPAGE: // Page Down
                {
                    int numTotalDataLines = (InputImageSize > 0) ? (static_cast<int>((InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE)) : 0;
                    int max_s = std::max(0, numTotalDataLines - VISIBLE_LINES);
                    scrollOffset = std::min(max_s, scrollOffset + VISIBLE_LINES);
                }
                break;
            case KEY_HOME: // Home key - go to the beginning
                scrollOffset = 0;
                break;
            case KEY_END: // End key - go to the last page
                {
                    int numTotalDataLines = (InputImageSize > 0) ? (static_cast<int>((InputImageSize + BYTES_PER_LINE - 1) / BYTES_PER_LINE)) : 0;
                    if (numTotalDataLines > 0) {
                        scrollOffset = std::max(0, numTotalDataLines - VISIBLE_LINES);
                    } else {
                        scrollOffset = 0;
                    }
                    break;
                }
                
            case KEY_RESIZE: // Terminal was resized
                // Recalculate layout and redraw fully if LINES or COLS changed significantly
                // For simplicity, ncurses handles SIGWINCH and resizes stdscr; subwindows might need explicit handling.
                // Here, we'll just let it redraw. For more complex UIs, might need delwin/newwin.
                calculateLayout(); // Recalculate based on potentially new LINES
                // If totalWidth changed, the window needs to be recreated
                delwin(hexWin);
                hexWin = newwin(LINES, totalWidth, 0, 0);
                if (!hexWin) { // Failsafe
                    endwin();
                    std::cerr << "Error recreating window after resize." << std::endl;
                    ch = 'q'; // Force exit
                }
                clear(); // Clear stdscr
                refresh(); // Refresh stdscr
                break;
            case 'Q':
            case 'q':
                ch = 'q'; // Set to 'q' to ensure loop termination
                break;
        }
    } while(ch != 'q');
}