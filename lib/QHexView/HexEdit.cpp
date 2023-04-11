#include "qhexview.h"
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QMessageBox>
#include "../../HexViewDialog.h"

void QHexView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        parentWidget->close();
    }

    if (!fileOpened)
        return;

    if( (event ->modifiers()& Qt::ControlModifier) != 0 && event ->key() == Qt::Key_F ) {
        actionSearch();
    } else if( (event ->modifiers()& Qt::ControlModifier) != 0 && event ->key() == Qt::Key_G ) {
        actionGoto();
    } else if( (event ->modifiers()& Qt::ControlModifier) != 0 && event ->key() == Qt::Key_S ) {
        BinaryEdited = false;
        ((HexViewDialog*)parentWidget)->setNewHexBuffer(m_pdata);
        ((HexViewDialog*)parentWidget)->setEditedState(BinaryEdited);
    } else if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
        inputKey = (event->key() - Qt::Key_0) & 0xF;
        binaryEdit(inputKey);
    } else if (event->key() >= Qt::Key_A && event->key() <= Qt::Key_F) {
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
            setSelection(2 * m_pdata.size() + 2);

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
        int pos = m_cursorPos - (m_cursorPos % (2 * m_bytesPerLine)) +
                  (2 * m_bytesPerLine);
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

void QHexView::mouseMoveEvent(QMouseEvent *event)
{
    if (!fileOpened)
        return;
    int actPos = cursorPos(event->pos());
    //  qDebug("mouseMoveEvent, actPos = %x", actPos);

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

void QHexView::mousePressEvent(QMouseEvent *event)
{
    if (!fileOpened)
        return;
    showCursor = true;
    timer->stop();

    if (event->pos().x() > (int)m_posAscii - (GAP_HEX_ASCII / 2))
        startFromAscii = true;
    else
        startFromAscii = false;

    int cPos = cursorPos(event->pos());
    //  qDebug("mousePressEvent, cPos = %x", cPos);

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

void QHexView::mouseReleaseEvent(QMouseEvent *event)
{
    if (!fileOpened)
        return;
    restartTimer();
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
    ((HexViewDialog*)parentWidget)->setEditedState(BinaryEdited);
}
