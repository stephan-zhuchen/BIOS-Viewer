#include <QString>
#include <QByteArray>
#include <iostream>

#define __BiosViewerVersion__   "BIOS Viewer 1.9"
#define __BiosViewerAuthor__    "Zhu, Chen"
#define __BiosViewerCopyright__ "Intel Internal Use Only"

QString xorEncrypt(const QString& str, char key) {
    QByteArray ba = str.toLatin1();
    for (int i = 0; i < ba.size(); i++)
        ba[i] = ba[i] ^ key;
    return QString(ba.toHex());
}

QString xorDecrypt(const QString& hexStr, char key) {
    QByteArray ba = QByteArray::fromHex(hexStr.toLatin1());
    for (int i = 0; i < ba.size(); i++)
        ba[i] = ba[i] ^ key;
    return QString(ba);
}

void EncryptAndShow(const QString& str) {
    QString encrypted = xorEncrypt(str, 0x5A);
    std::cout << encrypted.toStdString() << std::endl;
}

int main() {
    EncryptAndShow(__BiosViewerVersion__);
    EncryptAndShow(__BiosViewerCopyright__);
    EncryptAndShow(__BiosViewerAuthor__);
}
