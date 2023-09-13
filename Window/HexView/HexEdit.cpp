#include "HexView.h"
#include <QClipboard>
#include <QKeyEvent>
#include <QPainter>
#include <QMessageBox>
#include <QBuffer>
#include "BaseLib.h"
#include "HexViewDialog.h"
#include "HexWindow.h"

using namespace BaseLibrarySpace;

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
    } else if( (event ->modifiers() & Qt::ControlModifier) != 0 && event ->key() == Qt::Key_V ) {
        PasteToContent();
    } else if ((event ->modifiers() & Qt::ControlModifier) == 0 && event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
        inputKey = static_cast<CHAR8>((event->key() - Qt::Key_0) & 0xF);
        binaryEdit(inputKey);
    } else if ((event ->modifiers() & Qt::ControlModifier) == 0 && event->key() >= Qt::Key_A && event->key() <= Qt::Key_F) {
        inputKey = static_cast<CHAR8>((event->key() - 0x37) & 0xF);
        binaryEdit(inputKey);
    }

    bool setVisible = false;
    restartTimer();

    /*****************************************************************************/
    /* Cursor movements */
    /*****************************************************************************/
    if (event->key() == Qt::Key_Home) {
        setCursorPos(0);
        ResetSelection(CursorPosition);
        setVisible = true;
    } else if (event->key() == Qt::Key_End) {
        if (HexDataArray.size())
            setCursorPos((INT64)HexDataArray.size() * 2);
        ResetSelection(CursorPosition);
        setVisible = true;
    } else if (event->matches(QKeySequence::MoveToNextChar)) {
        setCursorPos(CursorPosition + 1);
        ResetSelection(CursorPosition);
        setVisible = true;
    } else if (event->matches(QKeySequence::MoveToPreviousChar)) {
        setCursorPos(CursorPosition - 1);
        ResetSelection(CursorPosition);
        setVisible = true;
    } else if (event->matches(QKeySequence::MoveToEndOfLine)) {
        setCursorPos(CursorPosition | ((BytesPerHexLine * 2) - 2));
        ResetSelection(CursorPosition);
        setVisible = true;
    } else if (event->matches(QKeySequence::MoveToStartOfLine)) {
        setCursorPos(CursorPosition | (CursorPosition % (BytesPerHexLine * 2)));
        ResetSelection(CursorPosition);
        setVisible = true;
    } else if (event->matches(QKeySequence::MoveToPreviousLine)) {
        setCursorPos(CursorPosition - BytesPerHexLine * 2);
        ResetSelection(CursorPosition);
        setVisible = true;
    } else if (event->matches(QKeySequence::MoveToNextLine)) {
        setCursorPos(CursorPosition + BytesPerHexLine * 2);
        ResetSelection(CursorPosition);
        setVisible = true;
    } else if (event->matches(QKeySequence::MoveToNextPage)) {
        setCursorPos(CursorPosition + (viewport()->height() / CharHeight - 2) * 2 * BytesPerHexLine);
        ResetSelection(CursorPosition);
        setVisible = true;
    } else if (event->matches(QKeySequence::MoveToPreviousPage)) {
        setCursorPos(CursorPosition - (viewport()->height() / CharHeight - 2) * 2 * BytesPerHexLine);
        ResetSelection(CursorPosition);
        setVisible = true;
    }

    /*****************************************************************************/
    /* Select commands */
    /*****************************************************************************/
    else if (event->matches(QKeySequence::SelectAll)) {
        ResetSelection(0);
        if (HexDataArray.size())
            setSelection((INT64)HexDataArray.size() * 2 - 1);
        setVisible = true;
    } else if (event->matches(QKeySequence::SelectNextChar)) {
        INT64 pos = CursorPosition + 2;
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    } else if (event->matches(QKeySequence::SelectPreviousChar)) {
        INT64 pos = CursorPosition - 2;
        setSelection(pos);
        setCursorPos(pos);
        setVisible = true;
    } else if (event->matches(QKeySequence::SelectEndOfLine)) {
        INT64 pos = CursorPosition - (CursorPosition % (2 * BytesPerHexLine)) + (2 * BytesPerHexLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    } else if (event->matches(QKeySequence::SelectStartOfLine)) {
        INT64 pos = CursorPosition - (CursorPosition % (2 * BytesPerHexLine));
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    } else if (event->matches(QKeySequence::SelectPreviousLine)) {
        INT64 pos = CursorPosition - (2 * BytesPerHexLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    } else if (event->matches(QKeySequence::SelectNextLine)) {
        INT64 pos = CursorPosition + (2 * BytesPerHexLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    } else if (event->matches(QKeySequence::SelectNextPage)) {
        INT64 pos = CursorPosition + (((viewport()->height() / CharHeight) - 2) * 2 * BytesPerHexLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    } else if (event->matches(QKeySequence::SelectPreviousPage)) {
        INT64 pos = CursorPosition - (((viewport()->height() / CharHeight) - 2) * 2 * BytesPerHexLine);
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    } else if (event->matches(QKeySequence::SelectEndOfDocument)) {
        INT64 pos = 0;
        if (HexDataArray.size())
            pos = (INT64)HexDataArray.size() * 2;

        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    } else if (event->matches(QKeySequence::SelectStartOfDocument)) {
        INT64 pos = 0;
        setCursorPos(pos);
        setSelection(pos);
        setVisible = true;
    } else if (event->matches(QKeySequence::Copy)) {
        CopyFromSelectedContent();
    }

    if (setVisible)
        ScrollToSelectedHexContent();

    viewport()->update();
}

void QHexView::mouseMoveEvent(QMouseEvent *event) {
    if (!FileOpened || HexDataArray.size() == 0 || event->buttons() & Qt::RightButton)
        return;
    INT64 actPos = cursorPos(event->pos());

    if (actPos >= 0 && actPos < HexDataArray.size() * 2) {
        setCursorPos(actPos);
        if (StartFromAscii){
            if (actPos >= (INT64)SelectionInit)
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

    if (event->pos().x() > (INT64)AsciiCharPosition - (HexAsciiGap / 2))
        StartFromAscii = true;
    else
        StartFromAscii = false;

    INT64 cPos = cursorPos(event->pos());

    if ((QApplication::keyboardModifiers() & Qt::ShiftModifier) && event->button() == Qt::LeftButton)
        setSelection(cPos);
    else {
        ResetSelection(cPos);
    }

    if (cPos != std::numeric_limits<INT64>::max()){
        if (StartFromAscii) {
            setCursorPos(cPos);
            ResetSelection(cPos);
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
    connect(CopyContent, SIGNAL(triggered(bool)), this, SLOT(CopyFromSelectedContent()));

    PasteInsertContent = new QAction("Paste and Insert");
    connect(PasteInsertContent, SIGNAL(triggered(bool)), this, SLOT(PasteAndInsertToContent()));

    PasteOverlapContent = new QAction("Paste and Overlap");
    connect(PasteOverlapContent, SIGNAL(triggered(bool)), this, SLOT(PasteAndOverlapToContent()));

    DiscardChange = new QAction("Discard changes");
    connect(DiscardChange, SIGNAL(triggered(bool)), this, SLOT(DiscardChangedContent()));

    EnableEditing = new QAction("Enable Editing");
    connect(EnableEditing, SIGNAL(toggled(bool)), this, SLOT(SetEditingState(bool)));

    SaveContent = new QAction("Save selected content");
    connect(SaveContent, SIGNAL(triggered(bool)), this, SLOT(SaveSelectedContent()));

    CheckSum8 = new QAction("CheckSum8");
    connect(CheckSum8, SIGNAL(triggered(bool)), this, SLOT(getChecksum8()));

    CheckSum16 = new QAction("CheckSum16");
    connect(CheckSum16, SIGNAL(triggered(bool)), this, SLOT(getChecksum16()));

    CheckSum32 = new QAction("CheckSum32");
    connect(CheckSum32, SIGNAL(triggered(bool)), this, SLOT(getChecksum32()));

    md5_Menu = new QAction("MD5");
    connect(md5_Menu,SIGNAL(triggered(bool)),this,SLOT(getMD5()));

    sha1_Menu = new QAction("SHA1");
    connect(sha1_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA1()));

    sha224_Menu = new QAction("SHA224");
    connect(sha224_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA224()));

    sha256_Menu = new QAction("SHA256");
    connect(sha256_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA256()));

    sha384_Menu = new QAction("SHA384");
    connect(sha384_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA384()));

    sha512_Menu = new QAction("SHA512");
    connect(sha512_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA512()));

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
    safeDelete(RightMenu);
    safeDelete(DigestMenu);
    safeDelete(ChecksumMenu);
    safeDelete(CopyContent);
    safeDelete(PasteInsertContent);
    safeDelete(PasteOverlapContent);
    safeDelete(DiscardChange);
    safeDelete(EnableEditing);
    safeDelete(SaveContent);
    safeDelete(CheckSum8);
    safeDelete(CheckSum16);
    safeDelete(CheckSum32);
    safeDelete(md5_Menu);
    safeDelete(sha1_Menu);
    safeDelete(sha224_Menu);
    safeDelete(sha256_Menu);
    safeDelete(sha384_Menu);
    safeDelete(sha512_Menu);
}

/**
 * @brief Handles the context menu event for the QHexView class.
 *
 * This function is called when a context menu event is triggered in the QHexView.
 * It allows the user to perform specific actions when the right mouse button is clicked.
 *
 * @param event The context menu event that was triggered.
 */

void QHexView::contextMenuEvent(QContextMenuEvent *event) {
    if (HexDataArray.size() == 0)
        return;
    INT64 actPos = cursorPos(event->pos());

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

    if (event->pos().x() < (INT64)AsciiCharPosition - (HexAsciiGap / 2) && event->pos().x() > (INT64)HexCharPosition && isSelected(actPos)) {
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

void QHexView::resizeEvent(QResizeEvent *event) {
    if (CharWidth == 0)
        return;
    INT32 MaxHexCharNum = 64;
    INT32 MaxLineLength = + HexCharPosition + (MaxHexCharNum + (MaxHexCharNum / 2) + (MaxHexCharNum / 16)) * CharWidth + HexAsciiGap + CharWidth * 32;
    INT32 width = this->width();
    if (DoubleHexLine && width < MaxLineLength) {
        DoubleHexLine = false;
        UpdateHexPosition();
    }
    else if (!DoubleHexLine && width >= MaxLineLength) {
        DoubleHexLine = true;
        UpdateHexPosition();
    }
}

void QHexView::binaryEdit(char inputChar) {
    if (ReadOnly) {
        return;
    }
    INT64 idx = CursorPosition / 2;
    EditedPos.push_back(CursorPosition);
    bool leftHex = (CursorPosition % 2) == 0;
    char newHex;
    if (leftHex) {
        newHex = static_cast<char>((inputChar << 4) | (HexDataArray.at(idx) & 0xF));
    } else {
        newHex = static_cast<char>((HexDataArray.at(idx) & 0xF0) | inputChar);
    }
    HexDataArray.replace(idx, 1, QByteArray(&newHex, 1));
    setCursorPos(CursorPosition + 1);
    BinaryEdited = true;
    if (!startFromMainWindow)
        ((HexViewDialog*)parentWidget)->setEditedState(BinaryEdited);
    else
        ((HexViewWindow*)parentWidget)->setEditedState(BinaryEdited);
}

void QHexView::SetEditingState(bool state) {
    ReadOnly = !state;
    if (ReadOnly)
        setting.setValue("EnableHexEditing", "false");
    else
        setting.setValue("EnableHexEditing", "true");
}

