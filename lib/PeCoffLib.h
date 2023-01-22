#pragma once
#include "UefiLib.h"
#include "../include/SymbolDefinition.h"
#include "../include/PeImage.h"

using namespace UefiSpace;

class PeCoff : public Volume
{
public:
    EFI_IMAGE_DOS_HEADER dosHeader;
public:
    PeCoff()=delete;
    PeCoff(UINT8* file, INT64 length, INT64 offset);
};
