#include "HexView.h"
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
#include "Input/inputdialog.h"
#include "Search/HexSearch.h"

QHexView::QHexView(QWidget *parent)
    : QAbstractScrollArea(parent),
      timer(new QTimer(this))
{
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    connect(timer, &QTimer::timeout, this, [this](){ ShowCursor = !ShowCursor; });
    InitSetting();
    InitCustomMenu();
}

QHexView::~QHexView() {
    CleanupCustomMenu();
    if (HexSearchDialogOpened) {
        HexSearchDialog->close();
        HexSearchDialog = nullptr;
    }
}

/**
 * @brief Refreshes the QHexView after the settings are changed.
 *
 * This function refreshes the QHexView by repainting the visible area with the updated data.
 * It ensures that any changes in the data are reflected in the view.
 *
 * The refreshed view will reflect any modifications made to the data model since the last InitSetting.
 *
 * This function is typically called when there are updates to the underlying data that need to be
 * displayed in the view.
 *
 * @see QHexView, setData()
 */

void QHexView::InitSetting() {
    // default configs
    QString font = setting.value("HexFont").toString();
    INT32 fontsize = setting.value("HexFontSize").toInt();
    HexFontSetting = QFont(font, fontsize);
    font = setting.value("AsciiFont").toString();
    AsciiFontSetting = QFont(font, fontsize);
    setFont(HexFontSetting); // default font

    auto SetDarkMode = [this](){
        isDarkMode = true;
        WordColor = Qt::white;
        WordColorOpposite = Qt::black;
        SelectionColor = QColor(38, 79, 120);
        CursorColor = QColor(235, 235, 235);
    };

    auto SetLightMode = [this](){
        isDarkMode = false;
        WordColor = Qt::black;
        WordColorOpposite = Qt::white;
        SelectionColor = QColor(199, 199, 199, 0xff);
        CursorColor = QColor(38, 95, 153, 0xff);
    };

    if (setting.value("Theme").toString() == "System") {
        if (SysSettings.value("AppsUseLightTheme", 1).toInt() == 0) {
            SetDarkMode();
        } else {
            SetLightMode();
        }
    } else if (setting.value("Theme").toString() == "Light") {
        SetLightMode();
    } else if (setting.value("Theme").toString() == "Dark") {
        SetDarkMode();
    }

    if (setting.value("EnableHexEditing").toString() == "false") {
        setReadOnly(true);
    } else if (setting.value("EnableHexEditing").toString() == "true") {
        setReadOnly(false);
    }

    UpdateHexPosition();
}

void QHexView::setFileOpened(bool state) {
    FileOpened = state;
}

void QHexView::loadFile(const QString& p_file) {
    QFile qFile;
    qFile.setFileName(p_file);
    qFile.open(QFile::ReadOnly);

    if (qFile.isOpen()) {
        setCursorPos(0);
        ResetSelection(0);
        HexDataArray = qFile.readAll();
        qFile.close();
    }
    else
        throw std::runtime_error("Failed to open file " + p_file.toStdString());

    restartTimer();
    setFileOpened(true);
}

void QHexView::loadFromBuffer(QByteArray &buffer) {
    HexDataArray = buffer;
    setCursorPos(0);
    ResetSelection(0);
    EditedPos.clear();
    setAddressLength();
    setFileOpened(true);
    restartTimer();
}

/**
 * @brief Sets the starting offset to display in the Hex View.
 *
 * This method sets the starting offset to display in the Hex View. The starting
 * offset determines the first byte to be shown in the Hex View. The offset should
 * be a non-negative value where 0 indicates the beginning of the data and the
 * maximum offset is based on the length of the data provided.
 *
 * @param offset The starting offset in bytes.
 * @param length The length of the data.
 */

void QHexView::showFromOffset(INT64 offset, INT64 length) {
    if (offset + length <= HexDataArray.size())
    {
        StartFromAscii = true;
        UpdateLayout();

        setCursorPos(offset * 2);
        ResetSelection(offset * 2);
        setSelection(offset * 2 + length * 2);

        INT32 cursorY = (INT32)(CursorPosition / (2 * BytesPerHexLine));

        verticalScrollBar()->setValue(cursorY);
        viewport()->update();
    }
}

/**
 * @brief Clears the content of the QHexView.
 *
 * This function removes all the data and clears the display of the QHexView.
 * It resets the cursor position to the beginning and clears any selection.
 * After calling this function, the QHexView is empty.
 */

void QHexView::clear() {
    verticalScrollBar()->setValue(0);
    HexDataArray.clear();
    viewport()->update();
}

/**
 * @brief Getter for the full size of the QHexView window.
 *
 * This function returns the full size of the QHexView window.
 *
 * @return The full size of the QHexView window.
 */

QSize QHexView::fullSize() const {
    if (HexDataArray.size() == 0)
        return {0, 0};

    INT32 width = (AsciiCharPosition + (BytesPerHexLine * CharWidth));
    INT32 height = (INT32)(HexDataArray.size() / BytesPerHexLine);

    if (HexDataArray.size() % BytesPerHexLine)
        height++;

    height *= (INT32)CharHeight;
    return {width, height};
}

/**
 * @brief Updates the positions of the QHexView widget.
 *
 * This method recalculates the positions of the QHexView's elements
 * based on the current state of the widget. It should be called whenever
 * the widget is resized or its contents change.
 *
 * The positions that are updated include:
 * - Hexadecimal and ASCII text labels
 * - Line numbers
 * - Vertical and horizontal scrollbars
 *
 * Note that this method does not return any value. It modifies the internal
 * state of the QHexView widget directly.
 *
 * @see QHexView
 */

void QHexView::UpdateLayout() {
    CharWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
    CharHeight = fontMetrics().height();
    HexCharPosition = (AddressLength + 3) * CharWidth + AddressPosition;
    AsciiCharPosition = HexCharPosition + HexIndexToCursorPos(BytesPerHexLine * 2 - 1) * CharWidth + CharWidth + HexAsciiGap;
}

void QHexView::UpdateHexPosition() {
    HexToPos.clear();
    PosToHex.clear();

    if (!DoubleHexLine)
        BytesPerHexLine = 16;
    else
        BytesPerHexLine = 32;

    for (INT32 HexIndex = 0; HexIndex < BytesPerHexLine * 2; ++HexIndex) {
        INT32 BytesGap = HexIndex / 2;
        INT32 SectionGap = HexIndex / 16;
        INT32 CursorPos = HexIndex + BytesGap + SectionGap;
        HexToPos.insert(HexIndex, CursorPos);
        PosToHex.insert(CursorPos, HexIndex);
    }
}

INT32 QHexView::HexIndexToCursorPos(INT32 HexIndex) const {
    if (HexIndex < 0)
        return 0;
    else if (HexIndex >= BytesPerHexLine * 2)
        return HexToPos.value(BytesPerHexLine * 2 - 1);
    else {
        return HexToPos.value(HexIndex);
    }
}

INT32 QHexView::CursorPosToHexIndex(INT32 CursorPos) const {
    INT32 EndPos = (BytesPerHexLine * 2 - 1) + BytesPerHexLine + (BytesPerHexLine - 1) / 8;
    if (CursorPos < 0)
        return 0;
    else if (CursorPos >= EndPos)
        return PosToHex.value(EndPos - 1);
    else {
        if (!PosToHex.contains(CursorPos))
            return CursorPosToHexIndex(CursorPos + 1);
        return PosToHex.value(CursorPos);
    }
}

/**
 * @brief Handles the paint event of the QHexView widget.
 *
 * This function is called whenever the QHexView widget needs to be painted,
 * such as when it becomes visible or is resized. It draws the content of the
 * widget based on the current state and visual configuration.
 *
 * @param event A pointer to the QPaintEvent object containing information
 *              about the paint event, such as the rectangle area that needs
 *              to be updated.
 *
 * @see QWidget::paintEvent()
 */

void QHexView::paintEvent(QPaintEvent *event) {
    if (HexDataArray.size() == 0)
        return;

    QPainter painter(viewport());
    UpdateLayout();
    confScrollBar();

    INT32 firstLineIdx = verticalScrollBar()->value();
    INT32 lastLineIdx = firstLineIdx + viewport()->size().height() / CharHeight;
    INT32 lastDataIdx = getLineNum() - 1;
    INT32 linePos = AsciiCharPosition - (HexAsciiGap / 2);
    INT32 horizenLinePosY = CharHeight + 2;
    INT32 yPosStart = CharHeight + horizenLinePosY;

    painter.setPen(QPen(Qt::gray, 1));
    painter.drawLine(linePos, horizenLinePosY, linePos, height());
    painter.setPen(QPen(Qt::gray, 1));
    painter.drawLine(AddressPosition, horizenLinePosY, AsciiCharPosition + BytesPerHexLine * CharWidth , horizenLinePosY);
    painter.setPen(WordColor);

  // offset drawn
    for (INT32 i = 0; i < BytesPerHexLine; ++i) {
        INT32 offsetX = HexCharPosition + HexIndexToCursorPos(2 * i) * CharWidth;
        INT64 BeginVal = (i + (RelativeAdressBase & 0xF)) % 16;
        QString offsetVal = QString::number(BeginVal, 16);
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
            CHAR8 character = data[(lineIdx - firstLineIdx) * BytesPerHexLine + i];
            character = FilterAscii(character);

            INT64 pos = ((lineIdx * BytesPerHexLine + i) * 2) + 1;
            if (isSelected(pos)) {
                painter.fillRect(QRectF(xPosAscii, yPos - CharHeight + 4, CharWidth, CharHeight), SelectionColor);
            }

            painter.setFont(AsciiFontSetting);
            painter.drawText(xPosAscii, yPos, QString(character));

            painter.setBackground(painter.brush());
            painter.setBackgroundMode(Qt::OpaqueMode);
        }

        // binary position
        for (INT32 i = 0; i < BytesPerHexLine * 2 && ((lineIdx - firstLineIdx) * BytesPerHexLine + i / 2) < data.size(); i++) {
            INT32 xPos = HexCharPosition + HexIndexToCursorPos(i) * CharWidth;
            INT64 HexByteIndex = ((lineIdx * BytesPerHexLine) * 2) + i;
            if (isSelected(HexByteIndex)) {
                painter.fillRect(QRectF(xPos, yPos - CharHeight + 4, CharWidth, CharHeight), SelectionColor);
                if (((i + 1) % 16 == 0) && isSelected(HexByteIndex + 1) && (i != BytesPerHexLine * 2 - 1)) {
                    painter.fillRect(QRectF(xPos + CharWidth, yPos - CharHeight + 4, CharWidth * 2, CharHeight), SelectionColor);
                }
                else if ((i % 2 == 1) && isSelected(HexByteIndex + 1) && (i != BytesPerHexLine * 2 - 1)) {
                    painter.fillRect(QRectF(xPos + CharWidth, yPos - CharHeight + 4, CharWidth, CharHeight), SelectionColor);
                }
            }
            if (isEdited(HexByteIndex)) {
                painter.fillRect(QRectF(xPos, yPos - CharHeight + 4, CharWidth, CharHeight), EditedColor);
                if (((i + 1) % 16 == 0) && isEdited(HexByteIndex + 1) && (i != BytesPerHexLine * 2 - 1)) {
                    painter.fillRect(QRectF(xPos + CharWidth, yPos - CharHeight + 4, CharWidth * 2, CharHeight), EditedColor);
                }
                else if ((i % 2 == 1) && isEdited(HexByteIndex + 1) && (i != BytesPerHexLine * 2 - 1)) {
                    painter.fillRect(QRectF(xPos + CharWidth, yPos - CharHeight + 4, CharWidth, CharHeight), EditedColor);
                }
            }
            if (isEdited(HexByteIndex) && isSelected(HexByteIndex)) {
                painter.fillRect(QRectF(xPos, yPos - CharHeight + 4, CharWidth, CharHeight), SelectedEditedColor);
                if (((i + 1) % 16 == 0) && isEdited(HexByteIndex + 1) && (i != BytesPerHexLine * 2 - 1)) {
                    painter.fillRect(QRectF(xPos + CharWidth, yPos - CharHeight + 4, CharWidth * 2, CharHeight), SelectedEditedColor);
                }
                else if ((i % 2 == 1) && isEdited(HexByteIndex + 1) && (i != BytesPerHexLine * 2 - 1)) {
                    painter.fillRect(QRectF(xPos + CharWidth, yPos - CharHeight + 4, CharWidth, CharHeight), SelectedEditedColor);
                }
            }

            QString val;
            char binaryNum = data.at((lineIdx - firstLineIdx) * BytesPerHexLine + i / 2);
            if (i % 2 == 0) {
                val = QString::number((binaryNum & 0xF0) >> 4, 16);
            } else {
                val = QString::number(binaryNum & 0xF, 16);
            }

            painter.setFont(HexFontSetting);
            painter.drawText(xPos, yPos, val.toUpper());

            painter.setBackground(painter.brush());
            painter.setBackgroundMode(Qt::OpaqueMode);
        }

        // address drawn
        if (lineIdx <= lastDataIdx) {
            QString address = QString("%1h:").arg(lineIdx * BytesPerHexLine + RelativeAdressBase, AddressLength, 16, QChar('0'));
            painter.setFont(HexFontSetting);
            painter.drawText(AddressPosition, yPos, address);
        }

        viewport()->update();
    }

    //cursor drawn
    if (hasFocus() && ShowCursor && (CursorPosition >= 0))
    {
        INT32 x = (INT32)(CursorPosition % (2 * BytesPerHexLine));
        INT32 y = (INT32)(CursorPosition / (2 * BytesPerHexLine));
        if (y < firstLineIdx || y >= lastLineIdx)
            return;
        y -= firstLineIdx;
        INT32 cursorX;
        if (StartFromAscii)
            cursorX = (x / 2) * CharWidth + AsciiCharPosition;
        else {
            cursorX = HexIndexToCursorPos(x) * CharWidth + HexCharPosition;
        }

        INT32 cursorY = y * CharHeight + 4;
        painter.fillRect(cursorX, cursorY + horizenLinePosY, CharWidth, CharHeight, CursorColor);

        if (StartFromAscii) {
            CHAR8 character = data[y * BytesPerHexLine + x / 2];
            character = FilterAscii(character);
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

/**
 * @brief Gets the cursor position in the QHexView widget based on the given position.
 *
 * The cursor position is a QPoint object representing the x and y coordinates in the widget's coordinate system.
 *
 * @param position The desired cursor position in widget's coordinate system.
 *
 * @return the cursor position
 */

INT64 QHexView::cursorPos(const QPoint &position) {
    INT64 pos;
    INT32 x;
    INT32 y;

    if (!StartFromAscii) {
        if (position.x() < HexCharPosition)
            x = 0;
        else if (position.x() >= (HexCharPosition + HexIndexToCursorPos(BytesPerHexLine * 2 - 1) * CharWidth))
            x = HexIndexToCursorPos(BytesPerHexLine * 2 - 1);
        else
            x = (position.x() - HexCharPosition) / CharWidth;

        x = CursorPosToHexIndex(x);
    } else {
        if (position.x() < AsciiCharPosition)
            x = 0;
        else if (position.x() >= (AsciiCharPosition + BytesPerHexLine * CharWidth))
            x = BytesPerHexLine * 2 - 2;
        else {
            INT32 asciiNum = (position.x() - AsciiCharPosition) / CharWidth;
            x = 2 * asciiNum;
        }
    }

    INT32 firstLineIdx = verticalScrollBar()->value();
    INT32 lastLineIdx = viewport()->size().height() / CharHeight + 1;
    INT32 lastDataIdx = getLineNum() - firstLineIdx;
    INT64 lastPageFirstLineIdx = getLineNum() + BlankEndingLineNum - viewport()->size().height() / CharHeight;
    INT32 lastX_Offset = ((INT32)HexDataArray.size() % BytesPerHexLine) * 2 - 1;

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

void QHexView::ResetSelection(INT64 pos) {
    if (pos == std::numeric_limits<INT64>::max())
        pos = 0;

    SelectionInit = pos;
    SelectionBegin = pos;
    SelectionEnd = pos;
}

void QHexView::setSelection(INT64 pos) {
    if (pos == std::numeric_limits<INT64>::max())
        pos = 0;

    if (pos >= SelectionInit) {
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
    if (SelectionEnd >= HexDataArray.size() * 2 - 1) {
        SelectionEnd = (INT64)(HexDataArray.size() * 2 - 1);
    }
}

void QHexView::setCursorPos(INT64 position) {
    // Ensure that position is within the bounds of the data.
    if (position < 0)
        position = 0;

    INT64 maxPos = 0;
    if (HexDataArray.size() != 0) {
        maxPos = (INT64)((HexDataArray.size() - 1) * 2 + 1);
    }

    if (position > maxPos)
        position = maxPos;

    // Set the cursor position.
    CursorPosition = position;
}

void QHexView::ScrollToSelectedHexContent() {
    QSize areaSize = viewport()->size();

    INT32 firstVisibleLineIdx = verticalScrollBar()->value();
    INT32 lastVisibleLineIdx = firstVisibleLineIdx + areaSize.height() / CharHeight;

    INT32 cursorY = (INT32)(CursorPosition / (2 * BytesPerHexLine));

    if (cursorY < firstVisibleLineIdx)
        verticalScrollBar()->setValue(cursorY);
    else if (cursorY >= lastVisibleLineIdx)
        verticalScrollBar()->setValue(cursorY - areaSize.height() / CharHeight + 1);
}

CHAR8 QHexView::FilterAscii(CHAR8 character) {
    if ((character < 0x20) || (character > 0x7e)) {
        return '.';
    }
    return character;
}

void QHexView::confScrollBar() {
    QSize areaSize = viewport()->size();
    QSize widgetSize = fullSize();
    verticalScrollBar()->setPageStep(areaSize.height() / (INT32)CharHeight);
    verticalScrollBar()->setRange(0, (widgetSize.height() - areaSize.height()) / (INT32)CharHeight + BlankEndingLineNum);
}

void QHexView::setAddressLength() {
    INT64 size = (INT64) HexDataArray.size() + RelativeAdressBase;
    for(INT32 i = 0; size >> i != 0; ++i) {
        AddressLength = i + 1;
    }
    AddressLength = (AddressLength + 3) / 4;
}

void QHexView::setRelativeAddress(INT64 address) {
    RelativeAdressBase = address;
    setAddressLength();
}

bool QHexView::isSelected(INT64 index) {
    bool ret = false;
    if (index < HexDataArray.size() * 2) {
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

bool QHexView::isEdited(INT64 index) {
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
    INT32 num = (INT32)(HexDataArray.size() / BytesPerHexLine);
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
        INT64 SearchOffset = (INT64)(UINT32)OffsetSpinbox->value() - RelativeAdressBase;
        if (SearchOffset < 0 || SearchOffset >= HexDataArray.size()) {
            QMessageBox::critical(this, tr("Goto ..."), "Invalid offset!");
            return;
        }
        showFromOffset(SearchOffset);
    }
}

void QHexView::actionSearch() {
    if (!HexSearchDialogOpened) {
        HexSearchDialogOpened = true;
        HexSearchDialog = new HexSearch();
        HexSearchDialog->SetBinaryData(&HexDataArray);
        HexSearchDialog->setParentWidget(this);
        if (isDarkMode)
            HexSearchDialog->setWindowIcon(QIcon(":/search_light.svg"));
        HexSearchDialog->show();
        connect(HexSearchDialog, SIGNAL(closeSignal(bool)), this, SLOT(setSearchDialogState(bool)));
    } else {
        HexSearchDialog->activateWindow();
    }
}

void QHexView::setParentWidget(QWidget *pWidget, bool fromMainWindow) {
    parentWidget = pWidget;
    startFromMainWindow = fromMainWindow;
}

void QHexView::setReadOnly(bool ReadOnlyFlag) {
    ReadOnly = ReadOnlyFlag;
}

bool QHexView::getSelectedBuffer(QByteArray &buffer, INT64*length) {
    INT64 beginIdx = SelectionBegin / 2;
    INT64 endIdx = SelectionEnd / 2;
    if (beginIdx > endIdx) {
        return false;
    }

    *length = endIdx - beginIdx + 1;
    buffer = HexDataArray.mid(beginIdx, *length);
    return true;
}

void QHexView::setSearchDialogState(bool opened) {
    HexSearchDialogOpened = opened;
}
