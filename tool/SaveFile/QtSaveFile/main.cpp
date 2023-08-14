#include <QSharedMemory>
#include <QDebug>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

struct SharedData {
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
    long long offset;
    long long size;
    bool ParaValid {false};
    bool DataValid {false};

    qDebug() << "Hello";
    QSharedMemory sharedParameter("BiosViewerSharedParameter");
    if (sharedParameter.attach()) {
        ParaValid = true;
        SharedData *parameter = (SharedData*)sharedParameter.constData();

        offset = parameter->Offset;
        size = parameter->Size;
        path = parameter->stringData;
    }

    QSharedMemory sharedMemory("BiosViewerSharedData");
    if (sharedMemory.attach()) {
        DataValid = true;
        qDebug() << "Shared";
        data = (char*)sharedMemory.constData();
    }

    if (ParaValid && DataValid) {
        saveBinary(path, data, offset, size);
    }

    return 0;
}
