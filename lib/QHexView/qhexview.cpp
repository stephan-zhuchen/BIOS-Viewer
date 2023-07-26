#include "qhexview.h"
#include <QClipboard>
#include <QFile>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QSize>
#include <stdexcept>
#include <QtGlobal>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "inputdialog.h"
#include "SearchDialog.h"

// valid character table ascii
#define CHAR_VALID(_character) ((_character < 0x20) || (_character > 0x7e)) ? _character = '.' : _character;

QHexView::QHexView(QWidget *parent)
    : QAbstractScrollArea(parent),
      HexDataArray(nullptr),
      HexCharPosition(ADR_LENGTH * CharWidth + GAP_ADR_HEX),
      AsciiCharPosition(HexCharPosition + MIN_HEXCHARS_IN_LINE * CharWidth + GAP_HEX_ASCII)
{
    // default configs
    if (setting.contains("HexFont") && setting.contains("HexFontSize")){
        QString font = setting.value("HexFont").toString();
        INT32 fontsize = setting.value("HexFontSize").toInt();
        FontSetting = QFont(font, fontsize, QFont::Normal, false);
    }
    setFont(FontSetting); // default font

    if (setting.value("Theme").toString() == "System") {
        if (SysSettings.value("AppsUseLightTheme", 1).toInt() == 0) {
            isDarkMode = true;
            WordColor = Qt::white;
            WordColorOpposite = Qt::black;
            SelectionColor = QColor(38, 79, 120);
            CursorColor = QColor(235, 235, 235);
        } else {
            isDarkMode = false;
            WordColor = Qt::black;
            WordColorOpposite = Qt::white;
            SelectionColor = QColor(COLOR_SELECTION);
            CursorColor = QColor(COLOR_CURSOR);
        }
    } else {
        isDarkMode = false;
    }

    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this](){ ShowCursor = !ShowCursor; });
    initRightMenu();
}

QHexView::~QHexView() {
    finiRightMenu();
}

void QHexView::refresh() {
    // default configs
    if (setting.contains("HexFont") && setting.contains("HexFontSize")){
        QString font = setting.value("HexFont").toString();
        INT32 fontsize = setting.value("HexFontSize").toInt();
        FontSetting = QFont(font, fontsize, QFont::Normal, false);
    }
    setFont(FontSetting); // default font

    if (setting.value("Theme").toString() == "System") {
        if (SysSettings.value("AppsUseLightTheme", 1).toInt() == 0) {
            WordColor = Qt::white;
            WordColorOpposite = Qt::black;
            SelectionColor = QColor(38, 79, 120);
            CursorColor = QColor(235, 235, 235);
        } else {
            WordColor = Qt::black;
            WordColorOpposite = Qt::white;
            SelectionColor = QColor(COLOR_SELECTION);
            CursorColor = QColor(COLOR_CURSOR);
        }
    }

    if (setting.value("EnableHexEditing").toString() == "false") {
        setReadOnly(true);
    } else if (setting.value("EnableHexEditing").toString() == "true") {
        setReadOnly(false);
    }
}

void QHexView::setfileOpened(bool state) {
    FileOpened = state;
}

void QHexView::loadFile(const QString& p_file) {
    QFile qFile;
    qFile.setFileName(p_file);
    qFile.open(QFile::ReadOnly);

    if (qFile.isOpen()) {
        setCursorPos(0);
        resetSelection(0);
        HexDataArray = qFile.readAll();
        qFile.close();
    }
    else
        throw std::runtime_error("Failed to open file " + p_file.toStdString());

    restartTimer();
    setfileOpened(true);

//  resetSelection(0);
}

void QHexView::loadFromBuffer(QByteArray &buffer) {
    HexDataArray = buffer;
    setCursorPos(0);
    resetSelection(0);
    EditedPos.clear();
    setAddressLength();
    setfileOpened(true);
    restartTimer();
}

// search and set offset
void QHexView::showFromOffset(INT32 offset, INT32 length) {
    if (offset + length < HexDataArray.size())
    {
        StartFromAscii = true;
        updatePositions();

        setCursorPos(offset * 2);
        resetSelection(offset * 2);
        setSelection(offset * 2 + length * 2);

        INT32 cursorY = CursorPosition / (2 * BytesPerHexLine);

        verticalScrollBar()->setValue(cursorY);
        viewport()->update();
    }
}

// clean all
void QHexView::clear() {
    verticalScrollBar()->setValue(0);
    HexDataArray.clear();
    viewport()->update();
}

QSize QHexView::fullSize() const {
    if (HexDataArray.size() == 0)
        return QSize(0, 0);

    INT32 width = AsciiCharPosition + (BytesPerHexLine * CharWidth);
    INT32 height = HexDataArray.size() / BytesPerHexLine;

    if (HexDataArray.size() % BytesPerHexLine)
        height++;

    height *= CharHeight;
    return QSize(width, height);
}

void QHexView::updatePositions() {
    CharWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
    CharHeight = fontMetrics().height();

    AddressPosition = 16;
    HexCharPosition = (AddressLength + 3) * CharWidth + AddressPosition;
    AsciiCharPosition = HexCharPosition + (BytesPerHexLine * 3 - 1) * CharWidth + GAP_HEX_ASCII;
}

/*****************************************************************************/
/* Paint Hex View  */
/*****************************************************************************/
void QHexView::paintEvent(QPaintEvent *event)
{
    if (HexDataArray.size() == 0)
        return;

    QPainter painter(viewport());

    updatePositions();
    confScrollBar();

    INT32 firstLineIdx = verticalScrollBar()->value();
    INT32 lastLineIdx = firstLineIdx + viewport()->size().height() / CharHeight;
    INT32 lastDataIdx = getLineNum() - 1;
    INT32 linePos = AsciiCharPosition - (GAP_HEX_ASCII / 2);
    INT32 horizenLinePosY = CharHeight + 2;
    INT32 yPosStart = CharHeight + horizenLinePosY;

    painter.setPen(QPen(Qt::gray, 1));
    painter.drawLine(linePos, horizenLinePosY, linePos, height());
    painter.setPen(QPen(Qt::gray, 1));
    painter.drawLine(AddressPosition, horizenLinePosY, AsciiCharPosition + BytesPerHexLine * CharWidth , horizenLinePosY);
    painter.setPen(WordColor);

  // offset drawn
    for (INT32 offsetX = HexCharPosition, i = 0; i < BytesPerHexLine; ++i, offsetX += CharWidth * 3) {
        QString offsetVal = QString::number(i, 16);
        painter.drawText(offsetX + CharWidth / 2, CharHeight - 2, offsetVal);
    }

    QByteArray data = HexDataArray.mid(firstLineIdx * BytesPerHexLine, (lastLineIdx - firstLineIdx) * BytesPerHexLine);
    painter.setPen(WordColor); // paint white characters and binary

    for (INT32 lineIdx = firstLineIdx, yPos = yPosStart; lineIdx < lastLineIdx; lineIdx += 1, yPos += CharHeight) {
        // ascii position
        for (INT32 xPosAscii = AsciiCharPosition, i = 0;
             ((lineIdx - firstLineIdx) * BytesPerHexLine + i) < data.size() && (i < BytesPerHexLine);
             i++, xPosAscii += CharWidth)
        {
            char character = data[(lineIdx - firstLineIdx) * BytesPerHexLine + i];
            CHAR_VALID(character);

            INT32 pos = ((lineIdx * BytesPerHexLine + i) * 2) + 1;
            if (isSelected(pos)) {
            painter.fillRect(QRectF(xPosAscii, yPos - CharHeight + 4, CharWidth, CharHeight), SelectionColor);
            }

            painter.drawText(xPosAscii, yPos, QString(character));

            painter.setBackground(painter.brush());
            painter.setBackgroundMode(Qt::OpaqueMode);
        }

        // binary position
        for (INT32 xPos = HexCharPosition, i = 0; i < BytesPerHexLine * 2 &&
            ((lineIdx - firstLineIdx) * BytesPerHexLine + i / 2) < data.size();
             i++, xPos += CharWidth)
        {
            INT32 pos = ((lineIdx * BytesPerHexLine) * 2) + i;
            if (isSelected(pos)) {
                painter.fillRect(QRectF(xPos, yPos - CharHeight + 4, CharWidth, CharHeight), SelectionColor);
                if ((i % 2 == 1) && isSelected(pos + 1) && (i != BytesPerHexLine * 2 - 1)){
                    painter.fillRect(QRectF(xPos + CharWidth, yPos - CharHeight + 4, CharWidth, CharHeight), SelectionColor);
                }
            }
            if (isEdited(pos)) {
                painter.fillRect(QRectF(xPos, yPos - CharHeight + 4, CharWidth, CharHeight), EditedColor);
                if ((i % 2 == 1) && isEdited(pos + 1) && (i != BytesPerHexLine * 2 - 1)){
                    painter.fillRect(QRectF(xPos + CharWidth, yPos - CharHeight + 4, CharWidth, CharHeight), EditedColor);
                }
            }
            if (isEdited(pos) && isSelected(pos)) {
                painter.fillRect(QRectF(xPos, yPos - CharHeight + 4, CharWidth, CharHeight), SlectedEditedColor);
                if ((i % 2 == 1) && isEdited(pos + 1) && (i != BytesPerHexLine * 2 - 1)){
                    painter.fillRect(QRectF(xPos + CharWidth, yPos - CharHeight + 4, CharWidth, CharHeight), SlectedEditedColor);
                }
            }

            QString val;
            char binaryNum = data.at((lineIdx - firstLineIdx) * BytesPerHexLine + i / 2);
            if (i % 2 == 0) {
                val = QString::number((binaryNum & 0xF0) >> 4, 16);
            } else {
                val = QString::number(binaryNum & 0xF, 16);
            }

            painter.drawText(xPos, yPos, val.toUpper());

            if (i % 2 == 1){
                xPos += CharWidth;
            }

            painter.setBackground(painter.brush());
            painter.setBackgroundMode(Qt::OpaqueMode);
        }

        // address drawn
        if (lineIdx <= lastDataIdx) {
            QString address = QString("%1h:").arg(lineIdx * BytesPerHexLine, AddressLength, 16, QChar('0'));
            painter.drawText(AddressPosition, yPos, address);
        }

        viewport()->update();
    }

    //cursor drawn
    if (hasFocus() && ShowCursor && (CursorPosition >= 0))
    {
        INT32 x = (CursorPosition % (2 * BytesPerHexLine));
        INT32 y = CursorPosition / (2 * BytesPerHexLine);
        if (y < firstLineIdx || y >= lastLineIdx)
            return;
        y -= firstLineIdx;
        INT32 cursorX;
        if (StartFromAscii)
            cursorX = (x / 2) * CharWidth + AsciiCharPosition;
        else
            cursorX = (((x / 2) * 3) + (x % 2)) * CharWidth + HexCharPosition;
        INT32 cursorY = y * CharHeight + 4;
        painter.fillRect(cursorX, cursorY + horizenLinePosY, CharWidth, CharHeight, CursorColor);

        if (StartFromAscii) {
            char character = data[y * BytesPerHexLine + x / 2];
            CHAR_VALID(character);
            painter.setPen(WordColorOpposite);
            painter.drawText(cursorX, (y + 1) * CharHeight + horizenLinePosY, QString(character));
            painter.setPen(WordColor);
        } else {
            QString val;
            char binaryNum = data.at(y * BytesPerHexLine + x / 2);
            if (x % 2 == 0) {
                val = QString::number((binaryNum & 0xF0) >> 4, 16);
            } else {
                val = QString::number(binaryNum & 0xF, 16);
            }
            painter.setPen(WordColorOpposite);
            painter.drawText(cursorX, (y + 1) * CharHeight + horizenLinePosY, val.toUpper());
            painter.setPen(WordColor);
        }
    }
}

INT32 QHexView::cursorPos(const QPoint &position) {
    INT32 pos = -1;
    INT32 x = 0;
    INT32 y = 0;

    if (!StartFromAscii) {
        if (position.x() < (INT32)HexCharPosition)
            x = 0;
        else if (position.x() >= (INT32)(HexCharPosition + (BytesPerHexLine * 3 - 1) * CharWidth))
            x = BytesPerHexLine * 3 - 2;
        else
            x = (position.x() - HexCharPosition) / CharWidth;

        if ((x % 3) == 0)
            x = (x / 3) * 2;
        else if((x % 3) == 1)
            x = ((x / 3) * 2) + 1;
        else
            x = ((x / 3) * 2) + 2;
    } else {
        if (position.x() < (INT32)AsciiCharPosition)
            x = 0;
        else if (position.x() >= (INT32)(AsciiCharPosition + BytesPerHexLine * CharWidth))
            x = BytesPerHexLine * 2 - 2;
        else {
            INT32 asciiNum = (position.x() - AsciiCharPosition) / CharWidth;
            x = 2 * (INT32)(asciiNum);
        }
    }

    INT32 firstLineIdx = verticalScrollBar()->value();
    INT32 lastLineIdx = viewport()->size().height() / CharHeight + 1;
    INT32 lastDataIdx = getLineNum() - firstLineIdx;
    INT32 lastPageFirstLineIdx = getLineNum() + BLANK_LINE_NUM - viewport()->size().height() / CharHeight;
    INT32 lastX_Offset = (HexDataArray.size() % BytesPerHexLine) * 2 - 1;

    y = (position.y() / (int)CharHeight);
    if (y < 0) {
        verticalScrollBar()->setValue(firstLineIdx - 1);
        firstLineIdx = (firstLineIdx > 0) ? firstLineIdx - 1 : 0;
        y = 0;
    }
    else if (y > (viewport()->size().height() / CharHeight)) {
        verticalScrollBar()->setValue(firstLineIdx + 1);
        if (firstLineIdx < lastPageFirstLineIdx)
            firstLineIdx += 1;
    }
    if (y > lastLineIdx)
        y = lastLineIdx;
    if (y > lastDataIdx)
        y = lastDataIdx;
    if (((HexDataArray.size() % BytesPerHexLine) != 0) && (y == lastDataIdx) && (x > lastX_Offset))
        x = lastX_Offset;
    y = (y == 0) ? y : y - 1;
    pos = x + y * 2 * BytesPerHexLine + firstLineIdx * BytesPerHexLine * 2;
    return pos;
}

void QHexView::resetSelection() {
    SelectionBegin = SelectionInit;
    SelectionEnd = SelectionInit;
}

void QHexView::resetSelection(INT32 pos) {
    if (pos == std::numeric_limits<INT32>::max())
        pos = 0;

    SelectionInit = pos;
    SelectionBegin = pos;
    SelectionEnd = pos;
}

void QHexView::setSelection(INT32 pos) {
    if (pos == std::numeric_limits<INT32>::max())
        pos = 0;

    if (pos >= (INT32)SelectionInit) {
        SelectionEnd = pos;
        SelectionBegin = SelectionInit;
    }
    else {
        SelectionBegin = pos;
        SelectionEnd = SelectionInit;
    }
    if (StartFromAscii){
        if (SelectionBegin == SelectionEnd) {
            SelectionEnd += 1;
        } else {
            SelectionEnd -= 1;
        }
    }
    if (SelectionEnd >= 2 * HexDataArray.size() - 1) {
        SelectionEnd = 2 * HexDataArray.size() - 1;
    }
}

void QHexView::setCursorPos(INT32 position)
{
    // Ensure that position is within the bounds of the data.
    if (position < 0)
        position = 0;

    INT32 maxPos = 0;
    if (HexDataArray.size() != 0) {
        maxPos = (HexDataArray.size() - 1) * 2 + 1;
    }

    if (position > maxPos)
        position = maxPos;

    // Set the cursor position.
    CursorPosition = position;
}

INT32 QHexView::getCursorPos() {
    return CursorPosition;
}

void QHexView::ensureVisible() {
    QSize areaSize = viewport()->size();

    INT32 firstVisibleLineIdx = verticalScrollBar()->value();
    INT32 lastVisibleLineIdx = firstVisibleLineIdx + areaSize.height() / CharHeight;

    INT32 cursorY = CursorPosition / (2 * BytesPerHexLine);

    if (cursorY < firstVisibleLineIdx)
        verticalScrollBar()->setValue(cursorY);
    else if (cursorY >= lastVisibleLineIdx)
        verticalScrollBar()->setValue(cursorY - areaSize.height() / CharHeight + 1);
}

void QHexView::confScrollBar() {
    QSize areaSize = viewport()->size();
    QSize widgetSize = fullSize();
    verticalScrollBar()->setPageStep(areaSize.height() / CharHeight);
    verticalScrollBar()->setRange(0, (widgetSize.height() - areaSize.height()) / CharHeight + BLANK_LINE_NUM);
}

std::size_t QHexView::sizeFile() {
    return HexDataArray.size();
}

void QHexView::setAddressLength() {
    INT32 size = (INT32) HexDataArray.size();
    for(INT32 i = 0; size >> i != 0; ++i) {
        AddressLength = i + 1;
    }
    INT32 remainder = AddressLength % 4;
    AddressLength = AddressLength / 4 + remainder;
}

bool QHexView::isSelected(INT32 index) {
    bool ret = false;
    if (index < sizeFile() * 2) {
        if (SelectionBegin != SelectionEnd) {
            if (SelectionBegin < SelectionEnd) {
                ret = (index >= SelectionBegin && index <= SelectionEnd);
            } else {
                ret = (index >= SelectionEnd && index <= SelectionBegin);
            }
        }
    }
    return ret;
}

bool QHexView::isEdited(INT32 index) {
    bool ret = false;
    if (std::count(EditedPos.begin(), EditedPos.end(), index)) {
        ret = true;
    }
    return ret;
}

void QHexView::restartTimer()
{
    timer->stop();
    ShowCursor = true;
    timer->start(500);
}

INT32 QHexView::getLineNum() {
    INT32 num = HexDataArray.size() / BytesPerHexLine;
    if (HexDataArray.size() % BytesPerHexLine)
        num += 1;
    return num;
}

void QHexView::actionGoto() {
    QDialog dialog(this);
    dialog.setWindowTitle("Goto ...");
    QFormLayout form(&dialog);
    // Offset
    QString Offset = QString("Offset: ");
    auto *OffsetSpinbox = new HexSpinBox(&dialog);
    OffsetSpinbox->setFocus();
    OffsetSpinbox->selectAll();
    form.addRow(Offset, OffsetSpinbox);
    // Add Cancel and OK button
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Process when OK button is clicked
    if (dialog.exec() == QDialog::Accepted) {
        INT64 SearchOffset = OffsetSpinbox->value();
        if (SearchOffset < 0 ||SearchOffset >= HexDataArray.size()) {
            QMessageBox::critical(this, tr("Goto ..."), "Invalid offset!");
            return;
        }
        showFromOffset(SearchOffset);
    }
}

void QHexView::actionSearch() {
    auto *searchDialog = new SearchDialog();
    searchDialog->SetBinaryData(&HexDataArray);
    searchDialog->setParentWidget(this);
    searchDialog->exec();
}

void QHexView::setParentWidget(QWidget *pWidget, bool fromMainWindow) {
    parentWidget = pWidget;
    startFromMainWindow = fromMainWindow;
}

void QHexView::setReadOnly(bool ReadOnlyFlag) {
    ReadOnly = ReadOnlyFlag;
}

bool QHexView::getSelectedBuffer(QByteArray &buffer, INT32*length) {
    INT32 beginIdx = SelectionBegin / 2;
    INT32 endIdx = SelectionEnd / 2;
    if (beginIdx > endIdx) {
        return false;
    }

    *length = endIdx - beginIdx + 1;
    buffer = HexDataArray.mid(beginIdx, *length);
    return true;
}
