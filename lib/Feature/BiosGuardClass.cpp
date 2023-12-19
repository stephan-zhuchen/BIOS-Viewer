//
// Created by stephan on 9/4/2023.
//

#include "BiosGuardClass.h"
#include "BaseLib.h"
#include <QStringList>
#include <QDebug>
#include <utility>

using namespace BaseLibrarySpace;

QString BgslOperation::getOperation() {
    QString OperationLine;
    QStringList OpeartionPattern;

    bool OpCodeValid = true;
    bool Op1Valid    = true;
    bool Op2Valid    = true;
    bool OpNumValid  = true;
    switch (OpCode) {
        case 0x0000:
            OpeartionPattern << "NOP" << "" << "" << "";
            break;
        case 0x0001:
            OpeartionPattern << "begin" << "" << "" << "";
            break;
        case 0x00FF:
            OpeartionPattern << "end" << "" << "" << "";
            break;
        case 0x0010:
            OpeartionPattern << "write" << "F" << "B" << "I";
            break;
        case 0x0011:
            OpeartionPattern << "write" << "F" << "B" << "Imm";
            break;
        case 0x0012:
            OpeartionPattern << "read" << "B" << "F" << "I";
            break;
        case 0x0013:
            OpeartionPattern << "read" << "B" << "F" << "Imm";
            break;
        case 0x0014:
            OpeartionPattern << "EraseBlk" << "F" << "" << "";
            break;
        case 0x0015:
            OpeartionPattern << "Erase64kBlk" << "F" << "" << "";
            break;
        case 0x0020:
            OpeartionPattern << "EcCmdWr" << "I" << "" << "";
            break;
        case 0x0021:
            OpeartionPattern << "EcCmdWr" << "0" << "" << "Imm";
            break;
        case 0x0025:
            OpeartionPattern << "EcDataRd" << "I" << "" << "";
            break;
        case 0x0030:
            OpeartionPattern << "add" << "I" << "I" << "";
            break;
        case 0x0031:
            OpeartionPattern << "add" << "I" << "" << "Imm";
            break;
        case 0x0032:
            OpeartionPattern << "add" << "B" << "I" << "";
            break;
        case 0x0033:
            OpeartionPattern << "add" << "B" << "" << "Imm";
            break;
        case 0x0034:
            OpeartionPattern << "add" << "F" << "I" << "";
            break;
        case 0x0035:
            OpeartionPattern << "add" << "F" << "" << "Imm";
            break;
        case 0x0036:
            OpeartionPattern << "sub" << "I" << "I" << "";
            break;
        case 0x0037:
            OpeartionPattern << "sub" << "I" << "" << "Imm";
            break;
        case 0x0038:
            OpeartionPattern << "sub" << "B" << "I" << "";
            break;
        case 0x0039:
            OpeartionPattern << "sub" << "B" << "" << "Imm";
            break;
        case 0x003A:
            OpeartionPattern << "sub" << "F" << "I" << "";
            break;
        case 0x003B:
            OpeartionPattern << "sub" << "F" << "" << "Imm";
            break;
        case 0x0040:
            OpeartionPattern << "And" << "I" << "I" << "";
            break;
        case 0x0041:
            OpeartionPattern << "And" << "I" << "" << "Imm";
            break;
        case 0x0042:
            OpeartionPattern << "Or" << "I" << "I" << "";
            break;
        case 0x0043:
            OpeartionPattern << "Or" << "I" << "" << "Imm";
            break;
        case 0x0044:
            OpeartionPattern << "ShiftR" << "I" << "" << "Imm";
            break;
        case 0x0045:
            OpeartionPattern << "ShiftL" << "I" << "" << "Imm";
            break;
        case 0x0046:
            OpeartionPattern << "RotateR" << "I" << "" << "Imm";
            break;
        case 0x0047:
            OpeartionPattern << "RotateL" << "I" << "" << "Imm";
            break;
        case 0x0050:
            OpeartionPattern << "set" << "I" << "I" << "";
            break;
        case 0x0051:
            OpeartionPattern << "set" << "I" << "" << "Imm";
            break;
        case 0x0052:
            OpeartionPattern << "set" << "B" << "I" << "";
            break;
        case 0x0053:
            OpeartionPattern << "set" << "B" << "" << "Imm";
            break;
        case 0x0054:
            OpeartionPattern << "set" << "F" << "I" << "";
            break;
        case 0x0055:
            OpeartionPattern << "set" << "F" << "" << "Imm";
            break;
        case 0x0060:
            OpeartionPattern << "LoadByte" << "I" << "B" << "";
            break;
        case 0x0061:
            OpeartionPattern << "LoadWord" << "I" << "B" << "";
            break;
        case 0x0062:
            OpeartionPattern << "LoadDword" << "I" << "B" << "";
            break;
        case 0x0063:
            OpeartionPattern << "StoreByte" << "B" << "I" << "";
            break;
        case 0x0064:
            OpeartionPattern << "StoreWord" << "B" << "I" << "";
            break;
        case 0x0065:
            OpeartionPattern << "StoreDword" << "B" << "I" << "";
            break;
        case 0x0070:
            OpeartionPattern << "Compare" << "I" << "I" << "";
            break;
        case 0x0071:
            OpeartionPattern << "Compare" << "I" << "" << "Imm";
            break;
        case 0x0072:
            OpeartionPattern << "Compare" << "B" << "I" << "";
            break;
        case 0x0073:
            OpeartionPattern << "Compare" << "B" << "" << "Imm";
            break;
        case 0x0074:
            OpeartionPattern << "Compare" << "F" << "I" << "";
            break;
        case 0x0075:
            OpeartionPattern << "Compare" << "F" << "" << "Imm";
            break;
        case 0x0076:
            OpeartionPattern << "Compare" << "B" << "B" << "I";
            break;
        case 0x0077:
            OpeartionPattern << "Compare" << "B" << "B" << "Imm";
            break;
        case 0x0080:
            OpeartionPattern << "Copy" << "B" << "B" << "I";
            break;
        case 0x0081:
            OpeartionPattern << "Copy" << "B" << "B" << "Imm";
            break;
        case 0x0090:
            OpeartionPattern << "Jmp" << "" << "" << "_label";
            break;
        case 0x0091:
            OpeartionPattern << "JE" << "" << "" << "_label";
            break;
        case 0x0092:
            OpeartionPattern << "JNE" << "" << "" << "_label";
            break;
        case 0x0093:
            OpeartionPattern << "JG" << "" << "" << "_label";
            break;
        case 0x0094:
            OpeartionPattern << "JGE" << "" << "" << "_label";
            break;
        case 0x0095:
            OpeartionPattern << "JL" << "" << "" << "_label";
            break;
        case 0x0096:
            OpeartionPattern << "JLE" << "" << "" << "_label";
            break;
        case 0x0097:
            OpeartionPattern << "Jmp" << "I" << "" << "";
            break;
        case 0x00A0:
            OpeartionPattern << "Log" << "Imm8" << "I" << "";
            break;
        case 0x00A1:
            OpeartionPattern << "Log" << "Imm8" << "" << "Imm";
            break;
        case 0x00B0:
            OpeartionPattern << "RdSts" << "I" << "" << "";
            break;
        case 0x00B1:
            OpeartionPattern << "RdKeySlot" << "I" << "" << "";
            break;
        case 0x00B2:
            OpeartionPattern << "RdRand" << "I" << "" << "";
            break;
        case 0x00C0:
            OpeartionPattern << "Stall" << "" << "" << "Imm";
            break;
        case 0x00C1:
            OpeartionPattern << "RdTS" << "I" << "" << "";
            break;
        case 0x00C2:
            OpeartionPattern << "SetTS" << "" << "" << "";
            break;
        case 0x00C3:
            OpeartionPattern << "ClearTS" << "" << "" << "";
            break;
        default:
            OpCodeValid = false;
            break;
    }

    if (OpCodeValid == false) {
        return "Invalid Op";
    }

    if (OpeartionPattern.at(1) == "" && Op1 != 0) {
        Op1Valid = false;
    } else if (OpeartionPattern.at(1) == "Imm8") {
        Op1Valid = true;
        OpeartionPattern.replace(1, "0x" + QString::number(OpNum, 16));
    } else if (OpeartionPattern.at(1) != "") {
        Op1Valid = true;
        OpeartionPattern.replace(1, OpeartionPattern.at(1) + QString::number(Op1, 16));
    } else {
        Op1Valid = true;
    }

    if (OpeartionPattern.at(2) == "" && Op2 != 0) {
        Op2Valid = false;
    } else if (OpeartionPattern.at(2) != "") {
        Op2Valid = true;
        OpeartionPattern.replace(2, OpeartionPattern.at(2) + QString::number(Op2, 16));
    } else {
        Op2Valid = true;
    }

    if (OpeartionPattern.at(3) == "" && OpNum != 0) {
        OpNumValid = false;
    } else if (OpeartionPattern.at(3) == "Imm") {
        OpNumValid = true;
        OpeartionPattern.replace(3, "0x" + QString::number(OpNum, 16));
    } else if (OpeartionPattern.at(3) == "_label") {
        OpNumValid = true;
        OpeartionPattern.replace(3, ":0x" + QString::number(OpNum, 16));
    } else if (OpeartionPattern.at(3) != "") {
        OpNumValid = true;
        OpeartionPattern.replace(3, OpeartionPattern.at(3) + QString::number(OpNum, 16));
    } else {
        OpNumValid = true;
    }

    if (OpCodeValid && Op1Valid && Op2Valid && OpNumValid) {
        OperationLine = OpeartionPattern.join(" ").simplified();
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
        qDebug() << "Invalid BGUPC";
    }

    // todo: assert ModulusSize == 0
    UINT8* Temp = data + sizeof(BGUP_HEADER) + BgupHeader.ScriptSectionSize + sizeof(BGUPC_HEADER);
    ModulusData = new UINT8[ModulusSize];
    for (int idx = 0; idx < ModulusSize; ++idx) {
        ModulusData[idx] = Temp[ModulusSize - idx - 1];
    }

    UINT32 ModulusTail = *(UINT32*)(data + sizeof(BGUP_HEADER) + BgupHeader.ScriptSectionSize + sizeof(BGUPC_HEADER) + ModulusSize);
    if (ModulusTail != 0x00010001) {
        qDebug() << "invalid Algorithm";
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
        qDebug() << "Invalid Bgsl size = " << length;
        return;
    }

    QStringList Script;
    INT32 fixedWidth = 10;
    INT64 NumOfOp = length / 8;
    for (INT32 idx = 0; idx < NumOfOp; ++idx) {
        BgslOperation OpLine = *(BgslOperation*)(buffer + idx * 8);
        QString OpLineStr = OpLine.getOperation();
        QStringList parts = OpLineStr.split(" ");
        QString resultString;

        for (int i = 0; i < parts.size(); ++i) {
            QString part = parts[i];
            if (i < parts.size() - 1) {
                part = part.leftJustified(fixedWidth, ' ');
            }
            resultString += part;
        }
        QString LineNumber = "0x" + QString::number(idx, 16) + ": ";
        LineNumber = LineNumber.rightJustified(6, ' ');
        LineNumber += resultString;

        Script << LineNumber;
    }

    BiosGuardScript = Script.join("\n");
}

QString BiosGuardClass::getPlatID() {
    QString PlatID = QString::fromStdString(charToString((CHAR8*)BgupHeader.PlatId, 16));
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
       << BiosGuardScript.toStdString();

    ss << "\n\nBGUPC_HEADER\n";
    width = 12;
    ss << setw(width) << "Version:"   << hex << uppercase << BgupCHeader.Version << "h\n"
       << setw(width) << "Algorithm:" << hex << uppercase << BgupCHeader.Algorithm << "h (" << Algorithm.toStdString() << ")\n"
       << "Modulus=\n"
       << DumpHex(ModulusData, ModulusSize) << "\n\n"
       << "Update Package Digest:\n"
       << DumpHex(UpdatePackageDigest, RSAKeySize);

    InfoStr = QString::fromStdString(ss.str());
}

QStringList BiosGuardClass::getUserDefinedName() const {
    QStringList UserDefinedName;
    UserDefinedName << "BGUP - " + Content;
    return UserDefinedName;
}

void BiosGuardClass::setContent(QString content) {
    Content = std::move(content);
}
