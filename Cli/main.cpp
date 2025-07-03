#include <iostream>
#include <string>
#include "BaseLib.h"
#include "cliview.h"

#ifdef _WIN32
    #include "curses.h"
#else
    #include <ncurses/ncurses.h>
    #include <clocale>

#endif
// #include "vld.h"

void help() {
    std::cout << "Usage: binary-tool <option> <filename>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help, -h       Show this help message" << std::endl;
    std::cout << "  --version, -v    Show version information" << std::endl;
    std::cout << "  --bios, -b       Open binary in BIOS view" << std::endl;
    std::cout << "  --hex, -x        Open binary in Hex view" << std::endl;
    std::cout << "  --elf, -e        Open binary in ELF view" << std::endl;
    std::cout << "  <filename>       Path to the binary file to open" << std::endl;
    std::cout << "Example: binary-tool myfile.bin" << std::endl;
    std::cout << "This tool opens a binary file and displays its contents in a suitable view." << std::endl;
}

class CursesManager {
  public:
    CursesManager() : initialized_successfully(false) {
#ifndef _WIN32
        setlocale(LC_ALL, ""); // 支持Unicode等
#endif
        // 初始化curses屏幕
        if (initscr() == NULL) {
            fprintf(stderr, "Error: unable to init curses.\n");
            // 此时curses环境未建立，可以直接使用fprintf
            return; // 初始化失败
        }

#ifndef _WIN32 // PDCurses的颜色处理可能略有不同或自动处理
        if (has_colors()) {
            if (start_color() == ERR) {
                // 颜色初始化失败，可以忽略或报告
                // endwin(); // 如果start_color失败，可能需要清理initscr
                // fprintf(stderr, "警告：无法初始化颜色支持。\n");
                // return; // 也可以选择退出
            } else {
                use_default_colors(); // 使用终端默认颜色
            }
        }
#endif
        cbreak();             // 行缓冲禁止，字符立即可用
        noecho();             // 禁止输入回显
        keypad(stdscr, TRUE); // 允许功能键 (F1,箭头等)
        curs_set(0);          // 初始隐藏光标 (HexCliView内部会管理)
        // timeout(-1);        // getch() 阻塞等待输入 (HexCliView会使用，这里设为默认)
        refresh(); // 刷新stdscr一次
        initialized_successfully = true;
    }

    ~CursesManager() {
        // 确保curses已初始化且endwin()尚未被调用
        if (initialized_successfully && !isendwin()) {
            endwin(); // 退出curses模式，恢复终端
        }
    }

    bool isInitialized() const {
        return initialized_successfully;
    }

  private:
    bool initialized_successfully;
};

int main(int argc, char *argv[]) {
    CliView view;
    if (argc <= 1) {
        std::cerr << "Error: No arguments provided." << std::endl;
        help();
        return 1;
    } else if (argc == 2) {
        if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
            help();
            return 0;
        }
        if (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-v") {
            std::cout << "Binary Tool Version 1.16" << std::endl;
            return 0;
        }
        CursesManager cursesMgr;
        if (!cursesMgr.isInitialized()) {
            std::cerr << "Error: Failed to initialize curses." << std::endl;
            return 1;
        }
        // 如果只有一个参数且不是帮助或版本，则尝试打开文件
        view.openFile(argv[1], ViewType::HexView);
    } else if (argc == 3) {
        ViewType type;
        if ((std::string(argv[1]) == "--bios" || std::string(argv[1]) == "-b")) {
            type = ViewType::BiosView;
        } else if ((std::string(argv[1]) == "--hex" || std::string(argv[1]) == "-x")) {
            type = ViewType::HexView;
        } else if ((std::string(argv[1]) == "--elf" || std::string(argv[1]) == "-e")) {
            type = ViewType::ElfView;
        } else {
            help();
            return 1;
        }
        CursesManager cursesMgr;
        if (!cursesMgr.isInitialized()) {
            std::cerr << "Error: Failed to initialize curses." << std::endl;
            return 1;
        }
        // 如果有两个参数，且第一个是选项，则尝试打开文件
        view.openFile(argv[2], type);
    } else if (argc > 3) {
        std::cerr << "Error: Too many arguments provided." << std::endl;
        help();
        return 1;
    }
    return 0;
}
