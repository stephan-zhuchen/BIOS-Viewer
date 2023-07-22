#include "qhexview.h"
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QMessageBox>
#include <QMenu>
#include <QFileDialog>
#include <QClipboard>
#include <QMimeData>
#include <QBuffer>
#include "BaseLib.h"
#include "HexViewDialog.h"
#include "HexWindow.h"
#include "openssl/sha.h"
#include "openssl/md5.h"

void QHexView::keyPressEvent(QKeyEvent *event) {
    if (!startFromMainWindow && event->key() == Qt::Key_Escape) {
        parentWidget->close();
    }

    if (!FileOpened || HexDataArray.size() == 0)
        return;

    if( (event ->modifiers() & Qt::ControlModifier) != 0 && event ->key() == Qt::Key_F ) {
        actionSearch();
    } else if( (event ->modifiers() & Qt::ControlModifier) != 0 && event ->key() == Qt::Key_G ) {
        actionGoto();
    } else if( (event ->modifiers() & Qt::ControlModifier) != 0 && event ->key() == Qt::Key_S && BinaryEdited ) {
        BinaryEdited = false;
        if (!startFromMainWindow) {
            ((HexViewDialog*)parentWidget)->setNewHexBuffer(HexDataArray);
            ((HexViewDialog*)parentWidget)->setEditedState(BinaryEdited);
            ((HexViewDialog*)parentWidget)->saveImage();
        } else {
            ((HexViewWindow*)parentWidget)->setNewHexBuffer(HexDataArray);
            ((HexViewWindow*)parentWidget)->setEditedState(BinaryEdited);
            ((HexViewWindow*)parentWidget)->saveImage();
        }
    } else if( (event ->modifiers() & Qt::ControlModifier) != 0 && event ->key() == Qt::Key_C ) {
        CopyFromSelectedContent();
    } else if( (event ->modifiers() & Qt::ControlModifier) != 0 && event ->key() == Qt::Key_V ) {
        PasteToContent();
    } else if ((event ->modifiers() & Qt::ControlModifier) == 0 && event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
        inputKey = (event->key() - Qt::Key_0) & 0xF;
        binaryEdit(inputKey);
    } else if ((event ->modifiers() & Qt::ControlModifier) == 0 && event->key() >= Qt::Key_A && event->key() <= Qt::Key_F) {
        inputKey = (event->key() - 0x37) & 0xF;
        binaryEdit(inputKey);
    }

    bool setVisible = false;
    restartTimer();

    /*****************************************************************************/
    /* Cursor movements */
    /*****************************************************************************/
    if (event->matches(QKeySequence::MoveToNextChar))
    {
        setCursorPos(CursorPosition + 1);
        resetSelection(CursorPosition);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToPreviousChar))
    {
        setCursorPos(CursorPosition - 1);
        resetSelection(CursorPosition);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToEndOfLine))
    {
        setCursorPos(CursorPosition | ((BytesPerHexLine * 2) - 2));
        resetSelection(CursorPosition);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToStartOfLine))
    {
        setCursorPos(CursorPosition | (CursorPosition % (BytesPerHexLine * 2)));
        resetSelection(CursorPosition);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToPreviousLine))
    {
        setCursorPos(CursorPosition - BytesPerHexLine * 2);
        resetSelection(CursorPosition);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToNextLine))
    {
        setCursorPos(CursorPosition + BytesPerHexLine * 2);
        resetSelection(CursorPosition);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToNextPage))
    {
        setCursorPos(CursorPosition + (viewport()->height() / CharHeight - 2) *
                                       2 * BytesPerHexLine);
        resetSelection(CursorPosition);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToPreviousPage))
    {
        setCursorPos(CursorPosition - (viewport()->height() / CharHeight - 2) *
                                       2 * BytesPerHexLine);
        resetSelection(CursorPosition);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToEndOfDocument))
    {
        if (HexDataArray.size())
            setCursorPos(HexDataArray.size() * 2);

        resetSelection(CursorPosition);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToStartOfDocument))
    {
        setCursorPos(0);
        resetSelection(CursorPosition);
        setVisible = true;
    }

    /*****************************************************************************/
    /* Select commands */
    /*****************************************************************************/
    if (event->matches(QKeySequence::SelectAll))
    {
        resetSelection(0);

        if (HexDataArray.size())
            setSelection(2 * HexDataArray.size() - 1);

        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectNextChar))
    {
        INT32 pos = CursorPosition + 2;
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectPreviousChar))
    {
        INT32 pos = CursorPosition - 2;
        setSelection(pos);
        setCursorPos(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectEndOfLine))
    {
        INT32 pos = CursorPosition - (CursorPosition % (2 * BytesPerHexLine)) + (2 * BytesPerHexLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectStartOfLine))
    {
        INT32 pos = CursorPosition - (CursorPosition % (2 * BytesPerHexLine));
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectPreviousLine))
    {
        INT32 pos = CursorPosition - (2 * BytesPerHexLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectNextLine))
    {
        INT32 pos = CursorPosition + (2 * BytesPerHexLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectNextPage))
    {
        INT32 pos = CursorPosition + (((viewport()->height() / CharHeight) - 2) *
                                 2 * BytesPerHexLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectPreviousPage))
    {
        INT32 pos = CursorPosition - (((viewport()->height() / CharHeight) - 2) *
                                 2 * BytesPerHexLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectEndOfDocument))
    {
        INT32 pos = 0;

        if (HexDataArray.size())
            pos = HexDataArray.size() * 2;

        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectStartOfDocument))
    {
        INT32 pos = 0;
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::Copy))
    {
        if (HexDataArray.size())
        {
            QString res;
            INT32 idx = 0;
            INT32 copyOffset = 0;

            QByteArray data = HexDataArray.mid(SelectionBegin / 2,
                                          (SelectionEnd - SelectionBegin) / 2 + 2);

            if (SelectionBegin % 2)
            {
                res += QString::number((data.at((idx + 2) / 2) & 0xF), 16);
                res += " ";
                idx++;
                copyOffset = 1;
            }

            INT32 selectedSize = SelectionEnd - SelectionBegin;

            for (; idx < selectedSize; idx += 2)
            {
                if (data.size() > (copyOffset + idx) / 2)
                {
                    QString val = QString::number(
                        (data.at((copyOffset + idx) / 2) & 0xF0) >> 4, 16);

                    if (idx + 2 < selectedSize)
                    {
                        val += QString::number(
                            (data.at((copyOffset + idx) / 2) & 0xF), 16);
                        val += " ";
                    }

                    res += val;

                    if ((idx / 2) % BytesPerHexLine == (BytesPerHexLine - 1))
                        res += "\n";
                }
            }

            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(res);
        }
    }

    if (setVisible)
        ensureVisible();

    viewport()->update();
}

void QHexView::mouseMoveEvent(QMouseEvent *event) {
    if (!FileOpened || HexDataArray.size() == 0 || event->buttons() & Qt::RightButton)
        return;
    INT32 actPos = cursorPos(event->pos());

    if (actPos >= 0 && actPos < HexDataArray.size() * 2)
    {
        setCursorPos(actPos);
        if (StartFromAscii){
            if (actPos >= (INT32)SelectionInit)
                setSelection(actPos + 2);
            else
                setSelection(actPos);
        } else
            setSelection(actPos);
    }

    viewport()->update();
}

void QHexView::mousePressEvent(QMouseEvent *event) {
    if (!FileOpened || HexDataArray.size() == 0 || event->buttons() & Qt::RightButton)
        return;
    ShowCursor = true;
    timer->stop();

    if (event->pos().x() > (INT32)AsciiCharPosition - (GAP_HEX_ASCII / 2))
        StartFromAscii = true;
    else
        StartFromAscii = false;

    INT32 cPos = cursorPos(event->pos());

    if ((QApplication::keyboardModifiers() & Qt::ShiftModifier) && event->button() == Qt::LeftButton)
        setSelection(cPos);
    else {
        resetSelection(cPos);
    }

    if (cPos != std::numeric_limits<INT32>::max()){
        if (StartFromAscii) {
            setCursorPos(cPos);
            resetSelection(cPos);
            setSelection(cPos + 2);
        } else
            setCursorPos(cPos);
    }

    viewport()->update();
}

void QHexView::mouseReleaseEvent(QMouseEvent *event) {
    if (!FileOpened || HexDataArray.size() == 0)
        return;
    restartTimer();
}

void QHexView::initRightMenu() {
    RightMenu = new QMenu;
    DigestMenu = new QMenu("Digest");
    ChecksumMenu = new QMenu("CheckSum");

    if (!isDarkMode) {
        RightMenu->setStyleSheet("QMenu::item:disabled {background: rgb(240, 240, 240, 255); color:rgba(100,100,100,1);}");
    }

    CopyContent = new QAction("Copy");
    CopyContent->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
    this->connect(CopyContent, SIGNAL(triggered(bool)), this, SLOT(CopyFromSelectedContent()));

    PasteInsertContent = new QAction("Paste and Insert");
    this->connect(PasteInsertContent, SIGNAL(triggered(bool)), this, SLOT(PasteAndInsertToContent()));

    PasteOverlapContent = new QAction("Paste and Overlap");
    this->connect(PasteOverlapContent, SIGNAL(triggered(bool)), this, SLOT(PasteAndOverlapToContent()));

    DiscardChange = new QAction("Discard changes");
    this->connect(DiscardChange, SIGNAL(triggered(bool)), this, SLOT(DiscardChangedContent()));

    EnableEditing = new QAction("Enable Editing");
    this->connect(EnableEditing, SIGNAL(toggled(bool)), this, SLOT(SetEditingState(bool)));

    SaveContent = new QAction("Save selected content");
    this->connect(SaveContent, SIGNAL(triggered(bool)), this, SLOT(SaveSelectedContent()));

    CheckSum8 = new QAction("CheckSum8");
    this->connect(CheckSum8, SIGNAL(triggered(bool)), this, SLOT(getChecksum8()));

    CheckSum16 = new QAction("CheckSum16");
    this->connect(CheckSum16, SIGNAL(triggered(bool)), this, SLOT(getChecksum16()));

    CheckSum32 = new QAction("CheckSum32");
    this->connect(CheckSum32, SIGNAL(triggered(bool)), this, SLOT(getChecksum32()));

    md5_Menu = new QAction("MD5");
    this->connect(md5_Menu,SIGNAL(triggered(bool)),this,SLOT(getMD5()));

    sha1_Menu = new QAction("SHA1");
    this->connect(sha1_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA1()));

    sha224_Menu = new QAction("SHA224");
    this->connect(sha224_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA224()));

    sha256_Menu = new QAction("SHA256");
    this->connect(sha256_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA256()));

    sha384_Menu = new QAction("SHA384");
    this->connect(sha384_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA384()));

    sha512_Menu = new QAction("SHA512");
    this->connect(sha512_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA512()));

    if (isDarkMode) {
        CopyContent->setIcon(QIcon(":/copy_light.svg"));
        PasteInsertContent->setIcon(QIcon(":/paste_light.svg"));
        PasteOverlapContent->setIcon(QIcon(":/paste_light.svg"));
        SaveContent->setIcon(QIcon(":/save_light.svg"));
        DigestMenu->setIcon(QIcon(":/key_light.svg"));
    } else {
        CopyContent->setIcon(QIcon(":/copy.svg"));
        PasteInsertContent->setIcon(QIcon(":/paste.svg"));
        PasteOverlapContent->setIcon(QIcon(":/paste.svg"));
        SaveContent->setIcon(QIcon(":/save.svg"));
        DigestMenu->setIcon(QIcon(":/key.svg"));
    }
}

void QHexView::finiRightMenu() {
    if (RightMenu != nullptr)
        delete RightMenu;
    if (DigestMenu != nullptr)
        delete DigestMenu;
    if (ChecksumMenu != nullptr)
        delete ChecksumMenu;
    if (CopyContent != nullptr)
        delete CopyContent;
    if (PasteInsertContent != nullptr)
        delete PasteInsertContent;
    if (PasteOverlapContent != nullptr)
        delete PasteOverlapContent;
    if (DiscardChange != nullptr)
        delete DiscardChange;
    if (EnableEditing != nullptr)
        delete EnableEditing;
    if (SaveContent != nullptr)
        delete SaveContent;
    if (CheckSum8 != nullptr)
        delete CheckSum8;
    if (CheckSum16 != nullptr)
        delete CheckSum16;
    if (CheckSum32 != nullptr)
        delete CheckSum32;
    if (md5_Menu != nullptr)
        delete md5_Menu;
    if (sha1_Menu != nullptr)
        delete sha1_Menu;
    if (sha224_Menu != nullptr)
        delete sha224_Menu;
    if (sha256_Menu != nullptr)
        delete sha256_Menu;
    if (sha384_Menu != nullptr)
        delete sha384_Menu;
    if (sha512_Menu != nullptr)
        delete sha512_Menu;
}

void QHexView::contextMenuEvent(QContextMenuEvent *event) {
    if (HexDataArray.size() == 0)
        return;
    INT32 actPos = cursorPos(event->pos());

    RightMenu->clear();
    ChecksumMenu->clear();
    DigestMenu->clear();

    CopyContent->setDisabled(true);
    RightMenu->addAction(CopyContent);

    if (ReadOnly || CopiedData.size() == 0) {
        PasteInsertContent->setDisabled(true);
        PasteOverlapContent->setDisabled(true);
    } else {
        PasteInsertContent->setEnabled(true);
        PasteOverlapContent->setEnabled(true);
    }
    if (!startFromMainWindow)
        PasteInsertContent->setDisabled(true);

    RightMenu->addAction(PasteInsertContent);
    RightMenu->addAction(PasteOverlapContent);

    EnableEditing->setCheckable(true);
    if (ReadOnly)
        EnableEditing->setChecked(false);
    else
        EnableEditing->setChecked(true);
    RightMenu->addAction(EnableEditing);

    if (BinaryEdited)
        DiscardChange->setEnabled(true);
    else
        DiscardChange->setEnabled(false);
    RightMenu->addAction(DiscardChange);

    if (event->pos().x() < (INT32)AsciiCharPosition - (GAP_HEX_ASCII / 2) && event->pos().x() > (INT32)HexCharPosition && isSelected(actPos)) {
        if (SelectionBegin % 2 != 0) {
            SelectionBegin -= 1;
        }
        if (SelectionEnd % 2 == 0) {
            SelectionEnd += 1;
        }

        CopyContent->setEnabled(true);
        RightMenu->addAction(SaveContent);
        ChecksumMenu->addAction(CheckSum8);
        ChecksumMenu->addAction(CheckSum16);
        ChecksumMenu->addAction(CheckSum32);

        if ((SelectionEnd - SelectionBegin + 1) % 4 == 0)
            CheckSum16->setEnabled(true);
        else
            CheckSum16->setEnabled(false);

        if ((SelectionEnd - SelectionBegin + 1) % 8 == 0)
            CheckSum32->setEnabled(true);
        else
            CheckSum32->setEnabled(false);

        DigestMenu->addAction(md5_Menu);
        DigestMenu->addAction(sha1_Menu);
        DigestMenu->addAction(sha224_Menu);
        DigestMenu->addAction(sha256_Menu);
        DigestMenu->addAction(sha384_Menu);
        DigestMenu->addAction(sha512_Menu);

        RightMenu->addMenu(ChecksumMenu);
        RightMenu->addMenu(DigestMenu);
    }
    RightMenu->exec(QCursor::pos());
}

void QHexView::binaryEdit(char inputChar) {
    if (ReadOnly) {
        return;
    }
    INT32 idx = (INT32)(CursorPosition / 2);
    EditedPos.push_back(CursorPosition);
    bool leftHex = (CursorPosition % 2) == 0;
    char newHex;
    if (leftHex) {
        newHex = (inputChar << 4) | (HexDataArray.at(idx) & 0xF);
    } else {
        newHex = (HexDataArray.at(idx) & 0xF0) | inputChar;
    }
    HexDataArray.replace(idx, 1, QByteArray(&newHex, 1));
    setCursorPos(CursorPosition + 1);
    BinaryEdited = true;
    if (!startFromMainWindow)
        ((HexViewDialog*)parentWidget)->setEditedState(BinaryEdited);
    else
        ((HexViewWindow*)parentWidget)->setEditedState(BinaryEdited);
}

void QHexView::CopyFromSelectedContent() {
    if (SelectionBegin < SelectionEnd) {
        qDebug() << "CopyFromSelectedContent";
        if (SelectionBegin % 2 != 0) {
            SelectionBegin -= 1;
        }
        if (SelectionEnd % 2 == 0) {
            SelectionEnd += 1;
        }
        INT32 length {0};
        getSelectedBuffer(CopiedData, &length);
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("Hexedit/CopiedData", CopiedData);
    clipboard->setMimeData(mimeData);

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
    INT32 idx = (INT32)(CursorPosition / 2);
    for (INT32 i = 0; i < EditedPos.size(); ++i) {
        if (EditedPos.at(i) >= CursorPosition) {
            EditedPos.at(i) += CopiedData.size() * 2;
        }
    }
    for (INT32 var = CursorPosition; var < CursorPosition + CopiedData.size() * 2; ++var) {
        EditedPos.push_back(var);
    }
    HexDataArray.insert(idx, CopiedData);
    setAddressLength();
    setCursorPos(CursorPosition + CopiedData.size() * 2);

    BinaryEdited = true;
    if (!startFromMainWindow)
        ((HexViewDialog*)parentWidget)->setEditedState(BinaryEdited);
    else
        ((HexViewWindow*)parentWidget)->setEditedState(BinaryEdited);
}

void QHexView::PasteAndOverlapToContent() {
    if (ReadOnly || CopiedData.size() == 0) {
        return;
    }
    if (CursorPosition + CopiedData.size() * 2 > HexDataArray.size() * 2) {
        INT32 choice = QMessageBox::warning(this,
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
    INT32 idx = CursorPosition / 2;
    for (INT32 var = CursorPosition; var < CursorPosition + CopiedData.size() * 2; ++var) {
        EditedPos.push_back(var);
    }
    HexDataArray.replace(idx, CopiedData.size(), CopiedData);
    setAddressLength();
    setCursorPos(CursorPosition + CopiedData.size() * 2);

    BinaryEdited = true;
    if (!startFromMainWindow)
        ((HexViewDialog*)parentWidget)->setEditedState(BinaryEdited);
    else
        ((HexViewWindow*)parentWidget)->setEditedState(BinaryEdited);
}

void QHexView::DiscardChangedContent() {
    BinaryEdited = false;
    if (!startFromMainWindow) {
        loadFromBuffer(((HexViewDialog*)parentWidget)->hexBuffer);
        ((HexViewDialog*)parentWidget)->setEditedState(BinaryEdited);
    }
    else {
        loadFromBuffer(((HexViewWindow*)parentWidget)->hexBuffer);
        ((HexViewWindow*)parentWidget)->setEditedState(BinaryEdited);
    }
}

void QHexView::SetEditingState(bool state) {
    ReadOnly = !state;
    if (ReadOnly)
        setting.setValue("EnableHexEditing", "false");
    else
        setting.setValue("EnableHexEditing", "true");
}

void QHexView::SaveSelectedContent() {
    QByteArray SelectedBuffer;
    INT32 length {0};
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
    BaseLibrarySpace::Buffer::saveBinary(extractVolumeName.toStdString(), (UINT8*)SelectedBuffer.data(), 0, length);
}

void QHexView::getChecksum8() {
    QByteArray SelectedBuffer;
    INT32 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    UINT8 *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 sum = BaseLibrarySpace::Buffer::CaculateSum8(itemData, length);
    QMessageBox::about(this, tr("Checksum"), "0x" + QString("%1").arg(sum, 2, 16, QLatin1Char('0')).toUpper());
}

void QHexView::getChecksum16() {
    QByteArray SelectedBuffer;
    INT32 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    UINT16 *itemData = (UINT16 *)SelectedBuffer.data();
    UINT8 sum = BaseLibrarySpace::Buffer::CaculateSum16(itemData, length / 2);
    QMessageBox::about(this, tr("Checksum"), "0x" + QString("%1").arg(sum, 4, 16, QLatin1Char('0')).toUpper());
}

void QHexView::getChecksum32() {
    QByteArray SelectedBuffer;
    INT32 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    UINT32 *itemData = (UINT32 *)SelectedBuffer.data();
    UINT8 sum = BaseLibrarySpace::Buffer::CaculateSum32(itemData, length / 4);
    QMessageBox::about(this, tr("Checksum"), "0x" + QString("%1").arg(sum, 8, 16, QLatin1Char('0')).toUpper());
}

void QHexView::getMD5() {
    QByteArray SelectedBuffer;
    INT32 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    UINT8 *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 md[MD5_DIGEST_LENGTH];
    MD5(itemData, length, md);

    QString hash;
    for (INT32 i = 0; i < MD5_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
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
    INT32 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    UINT8 *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 md[SHA_DIGEST_LENGTH];
    SHA1(itemData, length, md);

    QString hash;
    for (INT32 i = 0; i < SHA_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
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
    INT32 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    UINT8 *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 md[SHA224_DIGEST_LENGTH];
    SHA224(itemData, length, md);

    QString hash;
    for (INT32 i = 0; i < SHA224_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
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
    INT32 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    UINT8 *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 md[SHA256_DIGEST_LENGTH];
    SHA256(itemData, length, md);

    QString hash;
    for (INT32 i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
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
    INT32 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    UINT8 *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 md[SHA384_DIGEST_LENGTH];
    SHA384(itemData, length, md);

    QString hash;
    for (INT32 i = 0; i < SHA384_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
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
    INT32 length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    UINT8 *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 md[SHA512_DIGEST_LENGTH];
    SHA512(itemData, length, md);

    QString hash;
    for (INT32 i = 0; i < SHA512_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
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
