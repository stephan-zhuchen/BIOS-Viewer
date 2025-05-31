#ifndef CLIVIEW_H
#define CLIVIEW_H

#include "BiosView/BiosCli.h"
#include "DataModel.h"
#include "HexView/HexCli.h"
#include "Volume.h"


enum class ViewType { BiosView, ElfView, HexView };

class CliView {
public:
    BinaryData *binary{nullptr};
    BiosCliView *ui{nullptr};
    HexCliView *hexUi{nullptr};

    CliView() = default;
    ~CliView();
    int openFile(const string &path, ViewType type);
};

#endif // CLIVIEW_H
