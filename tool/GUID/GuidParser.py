import os
import re
import sys
import pickle
import argparse

Data1Set = set()
GuidNameSet = set()
Data1SetPath = "Data1Set.pickle"
GuidNameSetPath = "GuidNameSet.pickle"
GuidDifinitionData = open("GuidDifinition.txt", 'a')
GuidNameData = open("GuidName.txt", 'a')

def get_files(DirName, FilePaths, suffix):
    if not os.path.exists(DirName):
        return;
    print("Parsing GUID definition in " + DirName + " " + suffix)
    for filepath,dirnames,filenames in os.walk(DirName):
        for filename in filenames:
            file_name, file_extension = os.path.splitext(filename)
            if file_extension == suffix:
                DecPath = os.path.join(filepath, filename)
                FilePaths.append(DecPath)

def parseGuidInDec(DirPaths, Guidlist):
    lines = []
    for DecPath in DirPaths[:]:
        with open(DecPath, 'r') as F:
            for line in F.readlines():
                line = line.strip()
                lines.append(line)

        ProtocolsBeginIdx = 0
        PpisBeginIdx = 0
        GuidsBeginIdx = 0
        for (idx, line) in enumerate(lines):
            if line == "[Protocols]":
                ProtocolsBeginIdx = idx
            elif line == "[Ppis]":
                PpisBeginIdx = idx
            elif line == "[Guids]":
                GuidsBeginIdx = idx

        for line in lines[ProtocolsBeginIdx + 1:]:
            if line != "":
                if line[0] == "#":
                    continue
                if line[0] == "[":
                    break
                line = line.split("#")[0].strip()
                Guidlist.append(line)

        for line in lines[PpisBeginIdx + 1:]:
            if line != "":
                if line[0] == "#":
                    continue
                if line[0] == "[":
                    break
                line = line.split("#")[0].strip()
                Guidlist.append(line)

        for line in lines[GuidsBeginIdx + 1:]:
            if line != "":
                if line[0] == "#":
                    continue
                if line[0] == "[":
                    break
                line = line.split("#")[0].strip()
                Guidlist.append(line)

    for GuidDefinition in Guidlist:
        Name = GuidDefinition.split("=")[0].strip().replace("-", "_")
        Guid = GuidDefinition.split("=")[1].strip()

        while Name in GuidNameSet:
            Name = "_" + Name
        GuidNameSet.add(Name)

        GuidList = Guid.split(",")
        Data1 = GuidList[0]
        Data1 = Data1.split("{")[1].strip()
        num = int(Data1, 16)
        if num in Data1Set:
            continue
        Data1Set.add(num)

        Data1 = "{:#010x}".format(int(Data1, 16))

        Data2 = GuidList[1].strip()
        Data2 = "{:#06x}".format(int(Data2, 16))

        Data3 = GuidList[2].strip()
        Data3 = "{:#06x}".format(int(Data3, 16))

        Data41 = GuidList[3].split("{")[1].strip()
        Data41 = "{:#04x}".format(int(Data41, 16))

        Data42 = GuidList[4].strip()
        Data42 = "{:#04x}".format(int(Data42, 16))

        Data43 = GuidList[5].strip()
        Data43 = "{:#04x}".format(int(Data43, 16))

        Data44 = GuidList[6].strip()
        Data44 = "{:#04x}".format(int(Data44, 16))

        Data45 = GuidList[7].strip()
        Data45 = "{:#04x}".format(int(Data45, 16))

        Data46 = GuidList[8].strip()
        Data46 = "{:#04x}".format(int(Data46, 16))

        Data47 = GuidList[9].strip()
        Data47 = "{:#04x}".format(int(Data47, 16))

        Data48 = GuidList[10].split("}")[0].strip()
        Data48 = "{:#04x}".format(int(Data48, 16))

        OrganizedGuid = "{" + Data1 + ", " + Data2 + ", " + Data3 + ", {" + Data41 + ", " + Data42 + ", " + Data43 + ", " + Data44 + ", " + Data45 + ", " + Data46 + ", " + Data47 + ", " + Data48 + "}}"
        
        KeyName = re.sub(r'^_+', '', Name)
        CPP_Command_Line = "{" + Name + ".Data1, \"" + KeyName + "\"},"
        GuidNameData.write(CPP_Command_Line + "\n")

        NameLen = 45
        if len(Name) < NameLen:
            Name = Name + " " * (NameLen - len(Name))
        CPP_Command_Line = "constexpr static EFI_GUID " + Name + " = " + OrganizedGuid + ";"
        GuidDifinitionData.writelines(CPP_Command_Line + "\n")

def parseGuidInInf(DirPaths, GuidDict):
    for InfPath in DirPaths[:]:
        lines = []
        DefinesLines = []
        # print(InfPath)
        with open(InfPath, 'r') as F:
            for line in F.readlines():
                line = line.strip()
                lines.append(line)

        DefinesBeginIdx = 0
        for (idx, line) in enumerate(lines):
            if line == "[Defines]":
                DefinesBeginIdx = idx
        
        for line in lines[DefinesBeginIdx + 1:]:
            if line != "":
                if line[0] == "#":
                    continue
                if line[0] == "[":
                    break
                line = line.split("#")[0].strip()
                DefinesLines.append(line)

        Name = ''
        for GuidDefinition in DefinesLines:
            Key = GuidDefinition.split("=")[0].strip()
            if Key == "BASE_NAME":
                Name = GuidDefinition.split("=")[1].strip().replace("-", "_")
                if Name[0].isnumeric():
                    Name = "_" + Name
            elif Key == "FILE_GUID":
                Guid = GuidDefinition.split("=")[1].strip()
        if Name != '':
            while Name in GuidDict or Name in GuidNameSet:
                Name = "_" + Name
            GuidDict[Name] = Guid

    for key, value in GuidDict.items():
        if key in GuidNameSet:
            continue
        GuidNameSet.add(key)
        
        try:
            GuidList = value.split("-")
            Data1 = GuidList[0]
            assert len(Data1) == 8
            # Data1 = Data1.split("{")[1].strip()
            num = int(Data1, 16)
            if num in Data1Set:
                continue
            Data1Set.add(num)

            Data1 = "{:#010x}".format(int(Data1, 16))

            Data2 = GuidList[1].strip()
            assert len(Data2) == 4
            Data2 = "{:#06x}".format(int(Data2, 16))

            Data3 = GuidList[2].strip()
            assert len(Data3) == 4
            Data3 = "{:#06x}".format(int(Data3, 16))

            Data4 = GuidList[3].strip()
            assert len(Data4) == 4
            Data41 = Data4[:2]
            Data42 = Data4[2:]
            Data41 = "{:#04x}".format(int(Data41, 16))
            Data42 = "{:#04x}".format(int(Data42, 16))

            Data5 = GuidList[4].strip()
            assert len(Data5) == 12
            Data5_len = len(Data5);
            if Data5_len < 12:
                Data5 = "0" * (12 - Data5_len) + Data5
            Data43 = Data5[:2]
            Data44 = Data5[2:4]
            Data45 = Data5[4:6]
            Data46 = Data5[6:8]
            Data47 = Data5[8:10]
            Data48 = Data5[10:12]
            Data43 = "{:#04x}".format(int(Data43, 16))
            Data44 = "{:#04x}".format(int(Data44, 16))
            Data45 = "{:#04x}".format(int(Data45, 16))
            Data46 = "{:#04x}".format(int(Data46, 16))
            Data47 = "{:#04x}".format(int(Data47, 16))
            Data48 = "{:#04x}".format(int(Data48, 16))
            OrganizedGuid = "{" + Data1 + ", " + Data2 + ", " + Data3 + ", {" + Data41 + ", " + Data42 + ", " + Data43 + ", " + Data44 + ", " + Data45 + ", " + Data46 + ", " + Data47 + ", " + Data48 + "}}"
            
            KeyName = re.sub(r'^_+', '', key)
            CPP_Command_Line = "{" + key + ".Data1, \"" + KeyName + "\"},"
            GuidNameData.write(CPP_Command_Line + "\n")
            
            NameLen = 45
            # key += ".inf"
            if len(key) < NameLen:
                key = key + " " * (NameLen - len(key))
            CPP_Command_Line = "constexpr static EFI_GUID " + key + " = " + OrganizedGuid + ";"
            GuidDifinitionData.writelines(CPP_Command_Line + "\n")
        except Exception as e:
            print(InfPath)
            print(e)
            continue

def parseGuidInFdf(DirPaths, GuidDict):
    for FdfPath in DirPaths[:]:
        lines = []
        with open(FdfPath, 'r') as F:
            for line in F.readlines():
                line = line.strip()
                lines.append(line)

        # Get FvNameGuid in [FV.] block 
        FvBeginIdxes = []
        for (idx, line) in enumerate(lines):
            if line[:4] == "[FV.":
                FvBeginIdxes.append(idx)
        for (idx, FvLine) in enumerate(FvBeginIdxes):
            if idx == len(FvBeginIdxes) - 1:
                EndLineIdx = len(lines) - 1
            else:
                EndLineIdx = FvBeginIdxes[idx + 1]
            for line in lines[FvLine:EndLineIdx]:
                if line[:10] == "FvNameGuid":
                    FvName = lines[FvLine][4:].split("]")[0].strip().replace("-", "_")
                    Guid = line.split("=")[1].strip()
                    while FvName in GuidDict or FvName in GuidNameSet:
                        FvName = "_" + FvName
                    GuidDict[FvName] = Guid
                    break

        # Get FV_IMAGE GUID in SECTION FV_IMAGE block
        FvBeginIdxes = []
        for (idx, line) in enumerate(lines):
            if line[:13] == "FILE FV_IMAGE":
                FvBeginIdxes.append(idx)
        for (idx, FvLine) in enumerate(FvBeginIdxes):
            for line in lines[FvLine : FvLine + 3]:
                if line[:16] == "SECTION FV_IMAGE":
                    FvName = line.split("=")[1].strip().replace("-", "_")
                    FvName = FvName.split("/")[-1].split(".")[0]
                    Guid = lines[FvLine].split("=")[1].split("{")[0].strip()
                    while FvName in GuidDict or FvName in GuidNameSet:
                        FvName = "_" + FvName
                    GuidDict[FvName] = Guid
                    break

    for key, value in GuidDict.items():
        GuidList = value.split("-")
        Data1 = GuidList[0]
        try:
            num = int(Data1, 16)
        except ValueError:
            continue
        if num in Data1Set:
            continue
        Data1Set.add(num)

        if key in GuidNameSet:
            continue
        GuidNameSet.add(key)

        Data1 = "{:#010x}".format(int(Data1, 16))

        Data2 = GuidList[1].strip()
        Data2 = "{:#06x}".format(int(Data2, 16))

        Data3 = GuidList[2].strip()
        Data3 = "{:#06x}".format(int(Data3, 16))

        Data4 = GuidList[3].strip()
        Data41 = Data4[:2]
        Data42 = Data4[2:]
        Data41 = "{:#04x}".format(int(Data41, 16))
        Data42 = "{:#04x}".format(int(Data42, 16))

        Data5 = GuidList[4].strip()
        Data5_len = len(Data5);
        if Data5_len < 12:
            Data5 = "0" * (12 - Data5_len) + Data5
        Data43 = Data5[:2]
        Data44 = Data5[2:4]
        Data45 = Data5[4:6]
        Data46 = Data5[6:8]
        Data47 = Data5[8:10]
        Data48 = Data5[10:12]
        Data43 = "{:#04x}".format(int(Data43, 16))
        Data44 = "{:#04x}".format(int(Data44, 16))
        Data45 = "{:#04x}".format(int(Data45, 16))
        Data46 = "{:#04x}".format(int(Data46, 16))
        Data47 = "{:#04x}".format(int(Data47, 16))
        Data48 = "{:#04x}".format(int(Data48, 16))
        OrganizedGuid = "{" + Data1 + ", " + Data2 + ", " + Data3 + ", {" + Data41 + ", " + Data42 + ", " + Data43 + ", " + Data44 + ", " + Data45 + ", " + Data46 + ", " + Data47 + ", " + Data48 + "}}"
        KeyName = re.sub(r'^_+', '', key)
        CPP_Command_Line = "{" + key + ".Data1, \"" + KeyName + "\"},"
        GuidNameData.write(CPP_Command_Line + "\n")
       
        NameLen = 45
        if len(key) < NameLen:
            key = key + " " * (NameLen - len(key))
        CPP_Command_Line = "constexpr static EFI_GUID " + key + " = " + OrganizedGuid + ";"
        GuidDifinitionData.writelines(CPP_Command_Line + "\n")

if __name__ == "__main__":
    if os.path.exists(Data1SetPath):
        with open('Data1Set.pickle', 'rb') as f:
            Data1Set = pickle.load(f)
    if os.path.exists(GuidNameSetPath):
        with open('GuidNameSet.pickle', 'rb') as f:
            GuidNameSet = pickle.load(f)

    parser = argparse.ArgumentParser()
    parser.add_argument ("-c", "--code", dest = 'CodeBaseFolder', type = str, action='append', required = True,
                            help = "CodeBase Folder to parse GUID definition")

    args = parser.parse_args()
    Project_Code_Path = []

    for path in args.CodeBaseFolder:
        Project_Code_Path.append(path)

    for RepoPath in Project_Code_Path:
        DecPaths = []
        InfPaths = []
        FdfPaths = []
        get_files(RepoPath, DecPaths, ".dec")
        get_files(RepoPath, InfPaths, ".inf")
        get_files(RepoPath, FdfPaths, ".fdf")

        GuidInstances = []
        parseGuidInDec(DecPaths, GuidInstances)

        GuidInstances = {}
        parseGuidInInf(InfPaths, GuidInstances)

        GuidInstances = {}
        parseGuidInFdf(FdfPaths, GuidInstances)

    GuidDifinitionData.close()
    GuidNameData.close()
    with open('Data1Set.pickle', 'wb') as f:
        pickle.dump(Data1Set, f)
    with open('GuidNameSet.pickle', 'wb') as f:
        pickle.dump(GuidNameSet, f)


# python GuidParser.py -c E:\Code\LNL_EXT -c E:\Code\MTL_EXT -c E:\Code\PTL_ISE -c E:\Code\RPL -c E:\Code\Server\GNR -c E:\Code\Server\SPR