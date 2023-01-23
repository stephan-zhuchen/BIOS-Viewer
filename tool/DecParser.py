import os
import sys

path = os.getcwd();
EDK2_PATH = os.path.join(path, "EDK2")
EDK2PLATFORMS_PATH = os.path.join(path, "Edk2Platforms")
INTEL_PATH = os.path.join(path, "INTEL")

DecPaths = []
GuidInstances = []

def get_files(DirName):
    for filepath,dirnames,filenames in os.walk(DirName):
        for filename in filenames:
            file_name, file_extension = os.path.splitext(filename)
            if file_extension == ".dec":
                DecPath = os.path.join(filepath, filename)
                DecPaths.append(DecPath)

if __name__ == "__main__":
    get_files(EDK2_PATH)
    get_files(EDK2PLATFORMS_PATH)
    get_files(INTEL_PATH)

    lines = []
    for DecPath in DecPaths[:]:
        print(DecPath)
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
                # print(line)
                GuidInstances.append(line)
        
        for line in lines[PpisBeginIdx + 1:]:
            if line != "":
                if line[0] == "#":
                    continue
                if line[0] == "[":
                    break
                line = line.split("#")[0].strip()
                # print(line)
                GuidInstances.append(line)

        for line in lines[GuidsBeginIdx + 1:]:
            if line != "":
                if line[0] == "#":
                    continue
                if line[0] == "[":
                    break
                line = line.split("#")[0].strip()
                # print(line)
                GuidInstances.append(line)

    Data1Set = set()
    GuidData = open("GuidName.txt", 'w+')
    for GuidDefinition in GuidInstances:
        Name = GuidDefinition.split("=")[0].strip()
        Guid = GuidDefinition.split("=")[1].strip()

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

        # NameLen = 45
        # if len(Name) < NameLen:
        #     Name = Name + " " * (NameLen - len(Name))

        # CPP_Command_Line = "constexpr static EFI_GUID " + Name + " = " + OrganizedGuid + ";\n"

        CPP_Command_Line = "{" + Name + ".Data1, VNAME(" + Name + ")},"

        print(CPP_Command_Line)
        GuidData.writelines(CPP_Command_Line + "\n")

        # print(OrganizedGuid)
        # GuidData.write(GuidDefinition + "\n")
    
    GuidData.close()