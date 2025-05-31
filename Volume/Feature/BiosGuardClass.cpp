//
// Created by stephan on 9/4/2023.
//

#include "BiosGuardClass.h"
#include "BaseLib.h"
#include <utility>
#include <iostream>
#include <sstream>
#include <cstring>
#include <iomanip>

using namespace BaseLibrarySpace;

string BgslOperation::getOperation() {
    string OperationLine;
    vector<std::string> OperationPattern;

    bool OpCodeValid = true;
    bool Op1Valid    = true;
    bool Op2Valid    = true;
    bool OpNumValid  = true;
    switch (OpCode) {
    case 0x0000:
        OperationPattern = {"NOP", "", "", ""};
        break;
    case 0x0001:
        OperationPattern = {"begin", "", "", ""};
        break;
    case 0x00FF:
        OperationPattern = {"end", "", "", ""};
        break;
    case 0x0010:
        OperationPattern = {"write", "F", "B", "I"};
        break;
    case 0x0011:
        OperationPattern = {"write", "F", "B", "Imm"};
        break;
    case 0x0012:
        OperationPattern = {"read", "B", "F", "I"};
        break;
    case 0x0013:
        OperationPattern = {"read", "B", "F", "Imm"};
        break;
    case 0x0014:
        OperationPattern = {"EraseBlk", "F", "", ""};
        break;
    case 0x0015:
        OperationPattern = {"Erase64kBlk", "F", "", ""};
        break;
    case 0x0020:
        OperationPattern = {"EcCmdWr", "I", "", ""};
        break;
    case 0x0021:
        OperationPattern = {"EcCmdWr", "0", "", "Imm"};
        break;
    case 0x0025:
        OperationPattern = {"EcDataRd", "I", "", ""};
        break;
    case 0x0030:
        OperationPattern = {"add", "I", "I", ""};
        break;
    case 0x0031:
        OperationPattern = {"add", "I", "", "Imm"};
        break;
    case 0x0032:
        OperationPattern = {"add", "B", "I", ""};
        break;
    case 0x0033:
        OperationPattern = {"add", "B", "", "Imm"};
        break;
    case 0x0034:
        OperationPattern = {"add", "F", "I", ""};
        break;
    case 0x0035:
        OperationPattern = {"add", "F", "", "Imm"};
        break;
    case 0x0036:
        OperationPattern = {"sub", "I", "I", ""};
        break;
    case 0x0037:
        OperationPattern = {"sub", "I", "", "Imm"};
        break;
    case 0x0038:
        OperationPattern = {"sub", "B", "I", ""};
        break;
    case 0x0039:
        OperationPattern = {"sub", "B", "", "Imm"};
        break;
    case 0x003A:
        OperationPattern = {"sub", "F", "I", ""};
        break;
    case 0x003B:
        OperationPattern = {"sub", "F", "", "Imm"};
        break;
    case 0x0040:
        OperationPattern = {"And", "I", "I", ""};
        break;
    case 0x0041:
        OperationPattern = {"And", "I", "", "Imm"};
        break;
    case 0x0042:
        OperationPattern = {"Or", "I", "I", ""};
        break;
    case 0x0043:
        OperationPattern = {"Or", "I", "", "Imm"};
        break;
    case 0x0044:
        OperationPattern = {"ShiftR", "I", "", "Imm"};
        break;
    case 0x0045:
        OperationPattern = {"ShiftL", "I", "", "Imm"};
        break;
    case 0x0046:
        OperationPattern = {"RotateR", "I", "", "Imm"};
        break;
    case 0x0047:
        OperationPattern = {"RotateL", "I", "", "Imm"};
        break;
    case 0x0050:
        OperationPattern = {"set", "I", "I", ""};
        break;
    case 0x0051:
        OperationPattern = {"set", "I", "", "Imm"};
        break;
    case 0x0052:
        OperationPattern = {"set", "B", "I", ""};
        break;
    case 0x0053:
        OperationPattern = {"set", "B", "", "Imm"};
        break;
    case 0x0054:
        OperationPattern = {"set", "F", "I", ""};
        break;
    case 0x0055:
        OperationPattern = {"set", "F", "", "Imm"};
        break;
    case 0x0060:
        OperationPattern = {"LoadByte", "I", "B", ""};
        break;
    case 0x0061:
        OperationPattern = {"LoadWord", "I", "B", ""};
        break;
    case 0x0062:
        OperationPattern = {"LoadDword", "I", "B", ""};
        break;
    case 0x0063:
        OperationPattern = {"StoreByte", "B", "I", ""};
        break;
    case 0x0064:
        OperationPattern = {"StoreWord", "B", "I", ""};
        break;
    case 0x0065:
        OperationPattern = {"StoreDword", "B", "I", ""};
        break;
    case 0x0070:
        OperationPattern = {"Compare", "I", "I", ""};
        break;
    case 0x0071:
        OperationPattern = {"Compare", "I", "", "Imm"};
        break;
    case 0x0072:
        OperationPattern = {"Compare", "B", "I", ""};
        break;
    case 0x0073:
        OperationPattern = {"Compare", "B", "", "Imm"};
        break;
    case 0x0074:
        OperationPattern = {"Compare", "F", "I", ""};
        break;
    case 0x0075:
        OperationPattern = {"Compare", "F", "", "Imm"};
        break;
    case 0x0076:
        OperationPattern = {"Compare", "B", "B", "I"};
        break;
    case 0x0077:
        OperationPattern = {"Compare", "B", "B", "Imm"};
        break;
    case 0x0080:
        OperationPattern = {"Copy", "B", "B", "I"};
        break;
    case 0x0081:
        OperationPattern = {"Copy", "B", "B", "Imm"};
        break;
    case 0x0090:
        OperationPattern = {"Jmp", "", "", "_label"};
        break;
    case 0x0091:
        OperationPattern = {"JE", "", "", "_label"};
        break;
    case 0x0092:
        OperationPattern = {"JNE", "", "", "_label"};
        break;
    case 0x0093:
        OperationPattern = {"JG", "", "", "_label"};
        break;
    case 0x0094:
        OperationPattern = {"JGE", "", "", "_label"};
        break;
    case 0x0095:
        OperationPattern = {"JL", "", "", "_label"};
        break;
    case 0x0096:
        OperationPattern = {"JLE", "", "", "_label"};
        break;
    case 0x0097:
        OperationPattern = {"Jmp", "I", "", ""};
        break;
    case 0x00A0:
        OperationPattern = {"Log", "Imm8", "I", ""};
        break;
    case 0x00A1:
        OperationPattern = {"Log", "Imm8", "", "Imm"};
        break;
    case 0x00B0:
        OperationPattern = {"RdSts", "I", "", ""};
        break;
    case 0x00B1:
        OperationPattern = {"RdKeySlot", "I", "", ""};
        break;
    case 0x00B2:
        OperationPattern = {"RdRand", "I", "", ""};
        break;
    case 0x00C0:
        OperationPattern = {"Stall", "", "", "Imm"};
        break;
    case 0x00C1:
        OperationPattern = {"RdTS", "I", "", ""};
        break;
    case 0x00C2:
        OperationPattern = {"SetTS", "", "", ""};
        break;
    case 0x00C3:
        OperationPattern = {"ClearTS", "", "", ""};
        break;
    default:
        OpCodeValid = false;
        break;
    }

    if (!OpCodeValid) {
        return "Invalid Op";
    }

    auto replacePattern = [](vector<string>& pattern, size_t index, const string& value) {
        if (index < pattern.size()) {
            pattern[index] = value;
        }
    };

    auto toHexString = [](int value) {
        std::stringstream ss;
        ss << "0x" << std::hex << value;
        return ss.str();
    };

    if (OperationPattern[1].empty() && Op1 != 0) {
        Op1Valid = false;
    } else if (OperationPattern[1] == "Imm8") {
        Op1Valid = true;
        replacePattern(OperationPattern, 1, toHexString(OpNum));
    } else if (!OperationPattern[1].empty()) {
        Op1Valid = true;
        replacePattern(OperationPattern, 1, OperationPattern[1] + toHexString(Op1));
    } else {
        Op1Valid = true;
    }

    if (OperationPattern[2].empty() && Op2 != 0) {
        Op2Valid = false;
    } else if (!OperationPattern[2].empty()) {
        Op2Valid = true;
        replacePattern(OperationPattern, 2, OperationPattern[2] + toHexString(Op2));
    } else {
        Op2Valid = true;
    }

    if (OperationPattern[3].empty() && OpNum != 0) {
        OpNumValid = false;
    } else if (OperationPattern[3] == "Imm") {
        OpNumValid = true;
        replacePattern(OperationPattern, 3, toHexString(OpNum));
    } else if (OperationPattern[3] == "_label") {
        OpNumValid = true;
        replacePattern(OperationPattern, 3, ":0x" + toHexString(OpNum));
    } else if (!OperationPattern[3].empty()) {
        OpNumValid = true;
        replacePattern(OperationPattern, 3, OperationPattern[3] + toHexString(OpNum));
    } else {
        OpNumValid = true;
    }

    if (OpCodeValid && Op1Valid && Op2Valid && OpNumValid) {
        for (const auto& part : OperationPattern) {
            if (!part.empty()) {
                OperationLine += part + " ";
            }
        }
        // Remove the trailing space
        if (!OperationLine.empty()) {
            OperationLine.pop_back();
        }
    } else {
        OperationLine = "Invalid Op";
    }
    return OperationLine;
}

BiosGuardClass::BiosGuardClass(UINT8* buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) { }

INT64 BiosGuardClass::SelfDecode() {
    Type = VolumeType::BiosGuardPackage;
    BgupHeader = *(BGUP_HEADER*)data;
    if (BgupHeader.Version != 0x2) {
        return 0;
    }
    decodeBgsl(data + sizeof(BGUP_HEADER), BgupHeader.ScriptSectionSize);
    INT64 BgupCSize = size - sizeof(BGUP_HEADER) - BgupHeader.ScriptSectionSize;
    BgupCHeader = *(BGUPC_HEADER*)(data + sizeof(BGUP_HEADER) + BgupHeader.ScriptSectionSize);

    ModulusSize = 0;
    RSAKeySize = 0;
    switch (BgupCHeader.Algorithm) {
        case BGUPC_ALG_PKCS1_15_SHA256_RSA2048:
            Algorithm = "PKCS1 1.5, SHA-256 hash, RSA 2048 key";
            ModulusSize = 256;
            RSAKeySize = 256;
            break;
        case BGUPC_ALG_PKCS1_21_SHA256_RSA2048:
            Algorithm = "PKCS1 2.1, SHA-256 hash, RSA 2048 key";
            ModulusSize = 256;
            RSAKeySize = 256;
            break;
        case BGUPC_ALG_PKCS1_15_SHA256_RSA3072:
            Algorithm = "PKCS1 1.5, SHA-256 hash, RSA 3072 key";
            ModulusSize = 384;
            RSAKeySize = 384;
            break;
        case BGUPC_ALG_PKCS1_21_SHA256_RSA3072:
            Algorithm = "PKCS1 2.1, SHA-256 hash, RSA 3072 key";
            ModulusSize = 384;
            RSAKeySize = 384;
            break;
        case BGUPC_ALG_PKCS1_15_SHA384_RSA3072:
            Algorithm = "PKCS1 1.5, SHA-384 hash, RSA 3072 key";
            ModulusSize = 384;
            RSAKeySize = 384;
            break;
        case BGUPC_ALG_PKCS1_21_SHA384_RSA3072:
            Algorithm = "PKCS1 2.1, SHA-384 hash, RSA 3072 key";
            ModulusSize = 384;
            RSAKeySize = 384;
            break;
        default:
            Algorithm = "";
            break;
    }

    if (BgupCSize != sizeof(BGUPC_HEADER) + ModulusSize + sizeof(UINT32) + RSAKeySize) {
        std::cout << "Invalid BGUPC" << std::endl;
    }

    // todo: assert ModulusSize == 0
    UINT8* Temp = data + sizeof(BGUP_HEADER) + BgupHeader.ScriptSectionSize + sizeof(BGUPC_HEADER);
    ModulusData = new UINT8[ModulusSize];
    for (int idx = 0; idx < ModulusSize; ++idx) {
        ModulusData[idx] = Temp[ModulusSize - idx - 1];
    }

    UINT32 ModulusTail = *(UINT32*)(data + sizeof(BGUP_HEADER) + BgupHeader.ScriptSectionSize + sizeof(BGUPC_HEADER) + ModulusSize);
    if (ModulusTail != 0x00010001) {
        std::cout << "invalid Algorithm" << std::endl;
    }

    UpdatePackageDigest = new UINT8[RSAKeySize];
    UINT8* Src = data + sizeof(BGUP_HEADER) + BgupHeader.ScriptSectionSize + sizeof(BGUPC_HEADER) + ModulusSize + sizeof(ModulusTail);
    memcpy(UpdatePackageDigest, Src, RSAKeySize);
    return size;
}

BiosGuardClass::~BiosGuardClass() {
    safeArrayDelete(ModulusData);
    safeArrayDelete(UpdatePackageDigest);
}

void BiosGuardClass::decodeBgsl(UINT8 *buffer, INT64 length) {
    if ((length % 8) != 0) {
        std::cout << "Invalid Bgsl size = " << length << std::endl;
        return;
    }

    vector<string> Script;
    INT32 fixedWidth = 10;
    INT64 NumOfOp = length / 8;

    for (INT32 idx = 0; idx < NumOfOp; ++idx) {
        BgslOperation OpLine = *(BgslOperation*)(buffer + idx * 8);
        string OpLineStr = OpLine.getOperation();

        // Split the operation string into parts
        vector<string> parts;
        std::stringstream ss(OpLineStr);
        string part;
        while (ss >> part) {
            parts.push_back(part);
        }

        // Format the parts with fixed width
        string resultString;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i < parts.size() - 1) {
                // Left-justify the part with fixed width
                parts[i].resize(fixedWidth, ' ');
            }
            resultString += parts[i];
        }

        // Format the line number
        std::stringstream lineNumberStream;
        lineNumberStream << "0x" << std::hex << idx << ": ";
        string LineNumber = lineNumberStream.str();
        LineNumber.resize(6, ' '); // Right-justify to 6 characters
        LineNumber += resultString;

        Script.push_back(LineNumber);
    }

    // Join the script lines with newline characters
    std::stringstream scriptStream;
    for (const auto& line : Script) {
        scriptStream << line << "\n";
    }
    BiosGuardScript = scriptStream.str();
}

string BiosGuardClass::getPlatID() {
    string PlatID = charToString((CHAR8*)BgupHeader.PlatId, 16);
    return PlatID;
}

void BiosGuardClass::setInfoStr() {
    INT32 width = 20;
    stringstream ss;
    ss.setf(ios::left);

    ss << "BGUP_HEADER\n";
    ss << setw(width) << "Version:"           << hex << uppercase << BgupHeader.Version << "h\n"
       << setw(width) << "PlatId:"            << hex << uppercase << charToString((CHAR8*)BgupHeader.PlatId, 16) << "\n"
       << setw(width) << "PkgAttributes:"     << hex << uppercase << BgupHeader.PkgAttributes << "h\n"
       << setw(width) << "PslMajorVer:"       << hex << uppercase << BgupHeader.PslMajorVer << "h\n"
       << setw(width) << "PslMinorVer:"       << hex << uppercase << BgupHeader.PslMinorVer << "h\n"
       << setw(width) << "ScriptSectionSize:" << hex << uppercase << BgupHeader.ScriptSectionSize << "h\n"
       << setw(width) << "DataSectionSize:"   << hex << uppercase << BgupHeader.DataSectionSize << "h\n"
       << setw(width) << "BiosSvn:"           << hex << uppercase << BgupHeader.BiosSvn << "h\n"
       << setw(width) << "EcSvn:"             << hex << uppercase << BgupHeader.EcSvn << "h\n"
       << setw(width) << "VendorSpecific:"    << hex << uppercase << BgupHeader.VendorSpecific << "h\n";

    ss << "\nBios Guard Script:\n"
       << BiosGuardScript;

    ss << "\n\nBGUPC_HEADER\n";
    width = 12;
    ss << setw(width) << "Version:"   << hex << uppercase << BgupCHeader.Version << "h\n"
       << setw(width) << "Algorithm:" << hex << uppercase << BgupCHeader.Algorithm << "h (" << Algorithm << ")\n"
       << "Modulus=\n"
       << DumpHex(ModulusData, ModulusSize) << "\n\n"
       << "Update Package Digest:\n"
       << DumpHex(UpdatePackageDigest, RSAKeySize);

    InfoStr = ss.str();
}

vector<string> BiosGuardClass::getUserDefinedName() const {
    vector<string> UserDefinedName;
    UserDefinedName.push_back("BGUP - " + Content);
    return UserDefinedName;
}

void BiosGuardClass::setContent(string content) {
    Content = std::move(content);
}
