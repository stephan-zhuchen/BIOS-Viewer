#include <QSharedMemory>
#include <QDebug>
#include <QProcess>
#include <QString>
#include <QFile>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;

enum ToolType { NONE=0, PECOFF=1, ACPI=2 };
struct SharedData {
    ToolType    Type;
    char        stringData[256];
    long long   Offset;
    long long   Size;
};

/**
* @brief Save binary data to a file.
*
* This function saves binary data to a file given by the filename parameter.
*
* @param filename The name of the file to save the binary data to.
* @param address Pointer to the memory address containing the binary data.
* @param offset The offset from the beginning of the data to start saving.
* @param size The size, in bytes, of the binary data to save.
*
* @return void
*
*/
void saveBinary(const string& filename, char* address, long long offset, long long size) {
    ofstream outFile(filename, ios::out | ios::binary);
    outFile.write((char*)(address + offset), size);
    outFile.close();
}

int main(int argc, char *argv[])
{
    string path;
    char *data;
    ToolType  type {ToolType::NONE};
    long long offset;
    long long size;
    bool ParaValid {false};
    bool DataValid {false};

    QSharedMemory sharedParameter("BiosViewerSharedParameter");
    if (sharedParameter.attach()) {
        ParaValid = true;
        SharedData *parameter = (SharedData*)sharedParameter.constData();
        type = parameter->Type;
        offset = parameter->Offset;
        size = parameter->Size;
        path = parameter->stringData;
    }

    QSharedMemory sharedMemory("BiosViewerSharedData");
    if (sharedMemory.attach()) {
        DataValid = true;
        data = (char*)sharedMemory.constData();
    }

    if (ParaValid && DataValid && type == ToolType::PECOFF) {
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
        saveBinary(path, data, offset, size);
    }

    if (ParaValid && DataValid && type == ToolType::ACPI) {
        QString filepath = QString::fromStdString(path) + "/tool/acpitemp.bin";
        QString Dslpath = QString::fromStdString(path) + "/tool/acpitemp.dsl";
        QString toolpath = QString::fromStdString(path) + "/tool/ACPI/iasl.exe";

        saveBinary(filepath.toStdString(), data, offset, size);
        QFile DslFile(Dslpath);
        if(DslFile.exists()) {
            DslFile.remove();
        }

        auto *process = new QProcess();
        process->start(toolpath, QStringList() << "-d" << filepath);
        process->waitForFinished();
        delete process;
        QFile tempFile(filepath);
        if(tempFile.exists()) {
            tempFile.remove();
        }
    }

    return 0;
}
