#ifndef CLIVIEW_H
#define CLIVIEW_H

#include "Volume.h"
#include "BiosView/BiosCli.h"
#include "HexView/HexCli.h"

class CliView
{
public:
    BinaryData     *binary{nullptr};
    BiosCliView    *ui{nullptr};
    HexCliView     *hexUi{nullptr};

    CliView() = default;
    ~CliView();
    int openFile(const string &path);
};

#endif // CLIVIEW_H
