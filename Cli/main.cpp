#include <iostream>
#include "BaseLib.h"
#include "cliview.h"
#include <string>
// #include "vld.h"
 
void help()
{
    std::cout << "Usage: binary-tool <filename>" << std::endl;
}

int main(int argc, char *argv[])
{
    CliView view;
    if (argc == 2) {
        if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
            help();
            return 0;
        }
        if (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-v") {
            std::cout << "Binary Tool Version 1.16" << std::endl;
            return 0;
        }
        view.openFile(argv[1]);
    } else {
        help();
        return 1;
    }
    return 0;
}
