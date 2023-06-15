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
#include "BaseLib.h"
#include "HexViewDialog.h"
#include "HexWindow.h"
#include "openssl/sha.h"
#include "openssl/md5.h"

void QHexView::keyPressEvent(QKeyEvent *event) {
    if (!startFromMainWindow && event->key() == Qt::Key_Escape) {
        parentWidget->close();
    }

    if (!fileOpened)
        return;

    if( (event ->modifiers() & Qt::ControlModifier) != 0 && event ->key() == Qt::Key_F ) {
        qDebug() << "Hex search";
        actionSearch();
    } else if( (event ->modifiers() & Qt::ControlModifier) != 0 && event ->key() == Qt::Key_G ) {
        actionGoto();
    } else if( (event ->modifiers() & Qt::ControlModifier) != 0 && event ->key() == Qt::Key_S ) {
        BinaryEdited = false;
        if (!startFromMainWindow) {
            ((HexViewDialog*)parentWidget)->setNewHexBuffer(m_pdata);
            ((HexViewDialog*)parentWidget)->setEditedState(BinaryEdited);
            ((HexViewDialog*)parentWidget)->saveImage();
        } else {
            ((HexViewWindow*)parentWidget)->setNewHexBuffer(m_pdata);
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
        setCursorPos(m_cursorPos + 1);
        resetSelection(m_cursorPos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToPreviousChar))
    {
        setCursorPos(m_cursorPos - 1);
        resetSelection(m_cursorPos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToEndOfLine))
    {
        setCursorPos(m_cursorPos | ((m_bytesPerLine * 2) - 2));
        resetSelection(m_cursorPos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToStartOfLine))
    {
        setCursorPos(m_cursorPos | (m_cursorPos % (m_bytesPerLine * 2)));
        resetSelection(m_cursorPos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToPreviousLine))
    {
        setCursorPos(m_cursorPos - m_bytesPerLine * 2);
        resetSelection(m_cursorPos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToNextLine))
    {
        setCursorPos(m_cursorPos + m_bytesPerLine * 2);
        resetSelection(m_cursorPos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToNextPage))
    {
        setCursorPos(m_cursorPos + (viewport()->height() / m_charHeight - 2) *
                                       2 * m_bytesPerLine);
        resetSelection(m_cursorPos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToPreviousPage))
    {
        setCursorPos(m_cursorPos - (viewport()->height() / m_charHeight - 2) *
                                       2 * m_bytesPerLine);
        resetSelection(m_cursorPos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToEndOfDocument))
    {
        if (m_pdata.size())
            setCursorPos(m_pdata.size() * 2);

        resetSelection(m_cursorPos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToStartOfDocument))
    {
        setCursorPos(0);
        resetSelection(m_cursorPos);
        setVisible = true;
    }

    /*****************************************************************************/
    /* Select commands */
    /*****************************************************************************/
    if (event->matches(QKeySequence::SelectAll))
    {
        resetSelection(0);

        if (m_pdata.size())
            setSelection(2 * m_pdata.size() - 1);

        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectNextChar))
    {
        int pos = m_cursorPos + 2;
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectPreviousChar))
    {
        int pos = m_cursorPos - 2;
        setSelection(pos);
        setCursorPos(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectEndOfLine))
    {
        int pos = m_cursorPos - (m_cursorPos % (2 * m_bytesPerLine)) + (2 * m_bytesPerLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectStartOfLine))
    {
        int pos = m_cursorPos - (m_cursorPos % (2 * m_bytesPerLine));
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectPreviousLine))
    {
        int pos = m_cursorPos - (2 * m_bytesPerLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectNextLine))
    {
        int pos = m_cursorPos + (2 * m_bytesPerLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectNextPage))
    {
        int pos = m_cursorPos + (((viewport()->height() / m_charHeight) - 2) *
                                 2 * m_bytesPerLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectPreviousPage))
    {
        int pos = m_cursorPos - (((viewport()->height() / m_charHeight) - 2) *
                                 2 * m_bytesPerLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectEndOfDocument))
    {
        int pos = 0;

        if (m_pdata.size())
            pos = m_pdata.size() * 2;

        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectStartOfDocument))
    {
        int pos = 0;
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::Copy))
    {
        if (m_pdata.size())
        {
            QString res;
            int idx = 0;
            int copyOffset = 0;

            QByteArray data = m_pdata.mid(m_selectBegin / 2,
                                          (m_selectEnd - m_selectBegin) / 2 + 2);

            if (m_selectBegin % 2)
            {
                res += QString::number((data.at((idx + 2) / 2) & 0xF), 16);
                res += " ";
                idx++;
                copyOffset = 1;
            }

            int selectedSize = m_selectEnd - m_selectBegin;

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

                    if ((idx / 2) % m_bytesPerLine == (m_bytesPerLine - 1))
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
    if (!fileOpened || event->buttons() & Qt::RightButton)
        return;
    int actPos = cursorPos(event->pos());

    if (actPos >= 0 && actPos < m_pdata.size() * 2)
    {
        setCursorPos(actPos);
        if (startFromAscii){
            if (actPos >= (int)m_selectInit)
                setSelection(actPos + 2);
            else
                setSelection(actPos);
        } else
            setSelection(actPos);
    }

    viewport()->update();
}

void QHexView::mousePressEvent(QMouseEvent *event) {
    if (!fileOpened || event->buttons() & Qt::RightButton)
        return;
    showCursor = true;
    timer->stop();

    if (event->pos().x() > (int)m_posAscii - (GAP_HEX_ASCII / 2))
        startFromAscii = true;
    else
        startFromAscii = false;

    int cPos = cursorPos(event->pos());

    if ((QApplication::keyboardModifiers() & Qt::ShiftModifier) && event->button() == Qt::LeftButton)
        setSelection(cPos);
    else {
        resetSelection(cPos);
    }

    if (cPos != std::numeric_limits<int>::max()){
        if (startFromAscii) {
            setCursorPos(cPos);
            resetSelection(cPos);
            setSelection(cPos + 2);
        } else
            setCursorPos(cPos);
    }

    viewport()->update();
}

void QHexView::mouseReleaseEvent(QMouseEvent *event) {
    if (!fileOpened)
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
    int actPos = cursorPos(event->pos());

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

    if (event->pos().x() < (int)m_posAscii - (GAP_HEX_ASCII / 2) && event->pos().x() > m_posHex && isSelected(actPos)) {
        if (m_selectBegin % 2 != 0) {
            m_selectBegin -= 1;
        }
        if (m_selectEnd % 2 == 0) {
            m_selectEnd += 1;
        }

        CopyContent->setEnabled(true);
        RightMenu->addAction(SaveContent);
//        DigestMenu->setIcon(key);

        ChecksumMenu->addAction(CheckSum8);
        ChecksumMenu->addAction(CheckSum16);
        ChecksumMenu->addAction(CheckSum32);

        if ((m_selectEnd - m_selectBegin + 1) % 4 == 0)
            CheckSum16->setEnabled(true);
        else
            CheckSum16->setEnabled(false);

        if ((m_selectEnd - m_selectBegin + 1) % 8 == 0)
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
    unsigned int idx = (unsigned int)(m_cursorPos / 2);
    EditedPos.push_back(m_cursorPos);
    bool leftHex = (m_cursorPos % 2) == 0;
    char newHex;
    if (leftHex) {
        newHex = (inputChar << 4) | (m_pdata.at(idx) & 0xF);
    } else {
        newHex = (m_pdata.at(idx) & 0xF0) | inputChar;
    }
    m_pdata.replace(idx, 1, QByteArray(&newHex, 1));
    setCursorPos(m_cursorPos + 1);
    BinaryEdited = true;
    if (!startFromMainWindow)
        ((HexViewDialog*)parentWidget)->setEditedState(BinaryEdited);
    else
        ((HexViewWindow*)parentWidget)->setEditedState(BinaryEdited);
}

void QHexView::CopyFromSelectedContent() {
    if (m_selectBegin < m_selectEnd) {
        qDebug() << "CopyFromSelectedContent";
        if (m_selectBegin % 2 != 0) {
            m_selectBegin -= 1;
        }
        if (m_selectEnd % 2 == 0) {
            m_selectEnd += 1;
        }
        int length {0};
        getSelectedBuffer(CopiedData, &length);
    }
}

void QHexView::PasteToContent() {
    if (ReadOnly || CopiedData.size() == 0) {
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

    if (m_cursorPos % 2 != 0) {
        m_cursorPos += 1;
    }
    unsigned int idx = (unsigned int)(m_cursorPos / 2);
    for (int i = 0; i < EditedPos.size(); ++i) {
        if (EditedPos.at(i) >= m_cursorPos) {
            EditedPos.at(i) += CopiedData.size() * 2;
        }
    }
    for (unsigned int var = m_cursorPos; var < m_cursorPos + CopiedData.size() * 2; ++var) {
        EditedPos.push_back(var);
    }
    m_pdata.insert(idx, CopiedData);
    setAddressLength();
    setCursorPos(m_cursorPos + CopiedData.size() * 2);

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
    if (m_cursorPos + CopiedData.size() * 2 > m_pdata.size() * 2) {
        int choice = QMessageBox::warning(this,
                                          tr("Hex Viewer"),
                                          tr("The pasted content exceeds the end of file, Do you still want to paste? "),
                                          QMessageBox::Yes | QMessageBox::Cancel,
                                          QMessageBox::Yes);
        if (choice == QMessageBox::Cancel) {
            return;
        }
    }

    if (m_cursorPos % 2 != 0) {
        m_cursorPos += 1;
    }
    unsigned int idx = (unsigned int)(m_cursorPos / 2);
    for (unsigned int var = m_cursorPos; var < m_cursorPos + CopiedData.size() * 2; ++var) {
        EditedPos.push_back(var);
    }
    m_pdata.replace(idx, CopiedData.size(), CopiedData);
    setAddressLength();
    setCursorPos(m_cursorPos + CopiedData.size() * 2);

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
    int length {0};
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
    int length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    UINT8 *itemData = (UINT8 *)SelectedBuffer.data();
    UINT8 sum = BaseLibrarySpace::Buffer::CaculateSum8(itemData, length);
    QMessageBox::about(this, tr("Checksum"), "0x" + QString("%1").arg(sum, 2, 16, QLatin1Char('0')).toUpper());
}

void QHexView::getChecksum16() {
    QByteArray SelectedBuffer;
    int length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    UINT16 *itemData = (UINT16 *)SelectedBuffer.data();
    UINT8 sum = BaseLibrarySpace::Buffer::CaculateSum16(itemData, length);
    QMessageBox::about(this, tr("Checksum"), "0x" + QString("%1").arg(sum, 4, 16, QLatin1Char('0')).toUpper());
}

void QHexView::getChecksum32() {
    QByteArray SelectedBuffer;
    int length {0};
    if (!getSelectedBuffer(SelectedBuffer, &length)) {
        return;
    }
    UINT32 *itemData = (UINT32 *)SelectedBuffer.data();
    UINT8 sum = BaseLibrarySpace::Buffer::CaculateSum32(itemData, length);
    QMessageBox::about(this, tr("Checksum"), "0x" + QString("%1").arg(sum, 8, 16, QLatin1Char('0')).toUpper());
}

void QHexView::getMD5() {
    QByteArray SelectedBuffer;
    int length {0};
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
    QMessageBox::about(this, tr("MD5"), hash);
}

void QHexView::getSHA1() {
    QByteArray SelectedBuffer;
    int length {0};
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
    QMessageBox::about(this, tr("SHA1"), hash);
}

void QHexView::getSHA224() {
    QByteArray SelectedBuffer;
    int length {0};
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
    QMessageBox::about(this, tr("SHA224"), hash);
}

void QHexView::getSHA256() {
    QByteArray SelectedBuffer;
    int length {0};
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
    QMessageBox::about(this, tr("SHA256"), hash);
}

void QHexView::getSHA384() {
    QByteArray SelectedBuffer;
    int length {0};
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
    QMessageBox::about(this, tr("SHA384"), hash);
}

void QHexView::getSHA512() {
    QByteArray SelectedBuffer;
    int length {0};
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
    QMessageBox::about(this, tr("SHA512"), hash);
}
