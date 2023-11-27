//
// Created by stephan on 9/13/2023.
//

#include "HexView.h"
#include "BaseLib.h"
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QPushButton>
#include <QFileDialog>
#include <QMimeData>
#include "HexViewWidget.h"
#include "HexWindow.h"
#include "openssl/sha.h"
#include "openssl/md5.h"

using namespace BaseLibrarySpace;

void QHexView::getChecksum8() {
    QByteArray SelectedBuffer;
    INT64 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    auto *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 sum = BaseLibrarySpace::CalculateSum8(itemData, length);
    QMessageBox::about(this, tr("Checksum"), "0x" + QString("%1").arg(sum, 2, 16, QLatin1Char('0')).toUpper());
}

void QHexView::getChecksum16() {
    QByteArray SelectedBuffer;
    INT64 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    auto *itemData = (UINT16 *)SelectedBuffer.data();
    UINT8 sum = BaseLibrarySpace::CalculateSum16(itemData, length / 2);
    QMessageBox::about(this, tr("Checksum"), "0x" + QString("%1").arg(sum, 4, 16, QLatin1Char('0')).toUpper());
}

void QHexView::getChecksum32() {
    QByteArray SelectedBuffer;
    INT64 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    auto *itemData = (UINT32 *)SelectedBuffer.data();
    UINT8 sum = BaseLibrarySpace::CalculateSum32(itemData, length / 4);
    QMessageBox::about(this, tr("Checksum"), "0x" + QString("%1").arg(sum, 8, 16, QLatin1Char('0')).toUpper());
}

void QHexView::getMD5() {
    QByteArray SelectedBuffer;
    INT64 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    auto *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 md[MD5_DIGEST_LENGTH];
    MD5(itemData, length, md);

    QString hash;
    for (UINT8 i : md) {
        hash += QString("%1").arg(i, 2, 16, QLatin1Char('0'));
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("MD5"));
    msgBox.setText(hash);

    QPushButton *copyButton = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(hash);
    }
}

void QHexView::getSHA1() {
    QByteArray SelectedBuffer;
    INT64 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    auto *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 md[SHA_DIGEST_LENGTH];
    SHA1(itemData, length, md);

    QString hash;
    for (UINT8 i : md) {
        hash += QString("%1").arg(i, 2, 16, QLatin1Char('0'));
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("SHA1"));
    msgBox.setText(hash);

    QPushButton *copyButton = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(hash);
    }
}

void QHexView::getSHA224() {
    QByteArray SelectedBuffer;
    INT64 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    auto *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 md[SHA224_DIGEST_LENGTH];
    SHA224(itemData, length, md);

    QString hash;
    for (UINT8 i : md) {
        hash += QString("%1").arg(i, 2, 16, QLatin1Char('0'));
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("SHA224"));
    msgBox.setText(hash);

    QPushButton *copyButton = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(hash);
    }
}

void QHexView::getSHA256() {
    QByteArray SelectedBuffer;
    INT64 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    auto *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 md[SHA256_DIGEST_LENGTH];
    SHA256(itemData, length, md);

    QString hash;
    for (UINT8 i : md) {
        hash += QString("%1").arg(i, 2, 16, QLatin1Char('0'));
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("SHA256"));
    msgBox.setText(hash);

    QPushButton *copyButton = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(hash);
    }
}

void QHexView::getSHA384() {
    QByteArray SelectedBuffer;
    INT64 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    auto *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 md[SHA384_DIGEST_LENGTH];
    SHA384(itemData, length, md);

    QString hash;
    for (UINT8 i : md) {
        hash += QString("%1").arg(i, 2, 16, QLatin1Char('0'));
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("SHA384"));
    msgBox.setText(hash);

    QPushButton *copyButton = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(hash);
    }
}

void QHexView::getSHA512() {
    QByteArray SelectedBuffer;
    INT64 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    auto *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 md[SHA512_DIGEST_LENGTH];
    SHA512(itemData, length, md);

    QString hash;
    for (UINT8 i : md) {
        hash += QString("%1").arg(i, 2, 16, QLatin1Char('0'));
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("SHA512"));
    msgBox.setText(hash);

    QPushButton *copyButton = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(hash);
    }
}

void QHexView::SaveSelectedContent() {
    QByteArray SelectedBuffer;
    INT64 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    QString filename = "out.bin";
    QString outputPath = setting.value("LastFilePath").toString() + "/" + filename;
    QString DialogTitle = "Save Selected Content";
    QString extractVolumeName = QFileDialog::getSaveFileName(this,
                                                             DialogTitle,
                                                             outputPath,
                                                             tr("Files(*.rom *.bin *.fd);;All files (*.*)"));
    if (extractVolumeName.isEmpty()) {
        return;
    }
    BaseLibrarySpace::saveBinary(extractVolumeName.toStdString(), (UINT8*)SelectedBuffer.data(), 0, length);
}

void QHexView::CopyFromSelectedContent() {
    if (SelectionBegin % 2 != 0) {
        SelectionBegin -= 1;
    }
    if (SelectionEnd % 2 == 0) {
        SelectionEnd += 1;
    }

    if (SelectionBegin < SelectionEnd) {
        if (SelectionBegin % 2 != 0) {
            SelectionBegin -= 1;
        }
        if (SelectionEnd % 2 == 0) {
            SelectionEnd += 1;
        }
        INT64 length {0};
        getSelectedBuffer(CopiedData, &length);
    }

    QClipboard *dataClipboard = QGuiApplication::clipboard();
    auto *mimeData = new QMimeData;
    mimeData->setData("Hexedit/CopiedData", CopiedData);
    dataClipboard->setMimeData(mimeData);

    QString CopiedString;
    QByteArray data = HexDataArray.mid(SelectionBegin / 2, (SelectionEnd - SelectionBegin) / 2 + 1);
    for (CHAR8 var : data) {
        CopiedString += QString("%1").arg(var, 2, 16, QChar('0')).toUpper();
    }
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(CopiedString);
}

void QHexView::PasteToContent() {
    if (ReadOnly || CopiedData.size() == 0) {
        return;
    }

    if (!startFromMainWindow) {
        PasteAndOverlapToContent();
        return;
    }

    if (setting.value("PasteMode").toString() == "Ask Everytime") {
        QMessageBox msgBox;
        msgBox.setText("How to Paste your Content ?");
        QPushButton *InsertButton = msgBox.addButton(tr("Insert"), QMessageBox::ActionRole);
        QPushButton *OverlapButton = msgBox.addButton(tr("Overlap"), QMessageBox::ActionRole);
        msgBox.addButton(QMessageBox::Cancel);
        msgBox.exec();

        if (msgBox.clickedButton() == InsertButton) {
            PasteAndInsertToContent();
        } else if (msgBox.clickedButton() == OverlapButton) {
            PasteAndOverlapToContent();
        } else {
            return;
        }
    } else if (setting.value("PasteMode").toString() == "Overlap") {
        PasteAndOverlapToContent();
    } else if (setting.value("PasteMode").toString() == "Insert") {
        PasteAndInsertToContent();
    }
}

void QHexView::PasteAndInsertToContent() {
    if (ReadOnly || CopiedData.size() == 0) {
        return;
    }
    qDebug() << "Insert";

    if (CursorPosition % 2 != 0) {
        CursorPosition += 1;
    }
    INT64 idx = (INT64)(CursorPosition / 2);
    for (INT64 & Pos : EditedPos) {
        if (Pos >= CursorPosition) {
            Pos += (INT64)CopiedData.size() * 2;
        }
    }
    for (INT64 var = CursorPosition; var < CursorPosition + CopiedData.size() * 2; ++var) {
        EditedPos.push_back(var);
    }
    HexDataArray.insert(idx, CopiedData);
    setAddressLength();
    setCursorPos(CursorPosition + (INT64)CopiedData.size() * 2);

    BinaryEdited = true;
    if (!startFromMainWindow)
        ((HexViewWidget*)parentWidget)->setEditedState(BinaryEdited);
    else
        ((HexViewWindow*)parentWidget)->setEditedState(BinaryEdited);
}

void QHexView::PasteAndOverlapToContent() {
    if (ReadOnly || CopiedData.size() == 0) {
        return;
    }
    if (CursorPosition + CopiedData.size() * 2 > HexDataArray.size() * 2) {
        INT64 choice = QMessageBox::warning(this,
                                            tr("Hex Viewer"),
                                            tr("The pasted content exceeds the end of file, Do you still want to paste? "),
                                            QMessageBox::Yes | QMessageBox::Cancel,
                                            QMessageBox::Yes);
        if (choice == QMessageBox::Cancel) {
            return;
        }
    }

    if (CursorPosition % 2 != 0) {
        CursorPosition += 1;
    }
    INT64 idx = CursorPosition / 2;
    for (INT64 var = CursorPosition; var < CursorPosition + CopiedData.size() * 2; ++var) {
        EditedPos.push_back(var);
    }
    HexDataArray.replace(idx, CopiedData.size(), CopiedData);
    setAddressLength();
    setCursorPos(CursorPosition + (INT64)CopiedData.size() * 2);

    BinaryEdited = true;
    if (!startFromMainWindow)
        ((HexViewWidget*)parentWidget)->setEditedState(BinaryEdited);
    else
        ((HexViewWindow*)parentWidget)->setEditedState(BinaryEdited);
}

void QHexView::DiscardChangedContent() {
    BinaryEdited = false;
    if (!startFromMainWindow) {
        loadFromBuffer(((HexViewWidget*)parentWidget)->hexBuffer);
        ((HexViewWidget*)parentWidget)->setEditedState(BinaryEdited);
    }
    else {
        loadFromBuffer(((HexViewWindow*)parentWidget)->hexBuffer);
        ((HexViewWindow*)parentWidget)->setEditedState(BinaryEdited);
    }
}