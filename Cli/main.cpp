#include <iostream>
#include "BaseLib.h"
#include "cliview.h"
// #include "vld.h"

int main(int argc, char *argv[])
{
    CliView view;
    if (argc == 2){
        view.openFile(argv[1]);
    }
    return 0;
}

