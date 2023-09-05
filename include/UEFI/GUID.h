//
// Created by stephan on 8/28/2023.
//

#pragma once
#include "SymbolDefinition.h"
#include <string>
#include <sstream>
#include <iomanip>

struct EFI_GUID {
    UINT32    Data1;
    UINT16    Data2;
    UINT16    Data3;
    UINT8     Data4[8];
    bool operator==(EFI_GUID guid);
    bool operator!=(EFI_GUID guid);
    std::string str(bool upperCase = false);
};

