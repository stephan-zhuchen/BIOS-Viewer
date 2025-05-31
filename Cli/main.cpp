#include <iostream>
#include "BaseLib.h"
#include "cliview.h"
#include <string>
// #include "vld.h"
 
void help()
{
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

int main(int argc, char *argv[])
{
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
        view.openFile(argv[1], ViewType::HexView);
    } else if (argc == 3) {
        if ((std::string(argv[1]) == "--bios" || std::string(argv[1]) == "-b")) {
            view.openFile(argv[2], ViewType::BiosView);
        } else if ((std::string(argv[1]) == "--hex" || std::string(argv[1]) == "-x")) {
            view.openFile(argv[2], ViewType::HexView);
        } else if ((std::string(argv[1]) == "--elf" || std::string(argv[1]) == "-e")) {
            view.openFile(argv[2], ViewType::ElfView);
        } else {
            help();
            return 1;
        }
    }
    else if (argc > 3) {
        std::cerr << "Error: Too many arguments provided." << std::endl;
        help();
        return 1;
    }
    return 0;
}
