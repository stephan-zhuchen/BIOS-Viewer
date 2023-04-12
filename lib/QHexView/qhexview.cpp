#include "qhexview.h"
#include <QClipboard>
#include <QFile>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QSize>
#include <stdexcept>
#include <QtGlobal>
#include <QInputDialog>
#include "../../SearchDialog.h"

// valid character table ascii
#define CHAR_VALID(caracter) \
  ((caracter < 0x20) || (caracter > 0x7e)) ? caracter = '.' : caracter;

QOffsetView::QOffsetView(QWidget *parent):QWidget(parent) {
//    QLabel *label = new QLabel(this);
}

QHexView::QHexView(QString &applicationDir, QWidget *parent)
    : QAbstractScrollArea(parent),
      m_pdata(nullptr),
      m_posAddr(0),
      m_charWidth(0),
      m_posHex(ADR_LENGTH * m_charWidth + GAP_ADR_HEX),
      m_posAscii(m_posHex + MIN_HEXCHARS_IN_LINE * m_charWidth + GAP_HEX_ASCII),
      m_charHeight(0),
      m_selectBegin(0),
      m_selectEnd(0),
      m_selectInit(0),
      m_cursorPos(0),
      m_bytesPerLine(MIN_BYTES_PER_LINE),
      m_addressLength(ADR_LENGTH),
      showCursor(true),
      startFromAscii(false),
      fileOpened(false),
      fontSetting("Courier New", 12, QFont::Normal, false),
      selectionColor(COLOR_SELECTION),
      EditedColor(255, 128, 128, 0xff),
      wordColor(Qt::black),
      wordColorOpposite(Qt::white),
      cursorColor(COLOR_CURSOR),
      appDir(applicationDir),
      setting(QSettings(applicationDir + "/Setting.ini", QSettings::IniFormat))
{
  // default configs
  if (setting.contains("HexFont") && setting.contains("HexFontSize")){
      QString font = setting.value("HexFont").toString();
      int fontsize = setting.value("HexFontSize").toInt();
      fontSetting = QFont(font, fontsize, QFont::Normal, false);
  }
  setFont(fontSetting); // default font

  if (setting.value("Theme").toString() == "System") {
      if (SysSettings.value("AppsUseLightTheme", 1).toInt() == 0) {
          wordColor = Qt::white;
          wordColorOpposite = Qt::black;
          selectionColor = QColor(38, 79, 120);
          cursorColor = QColor(235, 235, 235);
      } else {
          wordColor = Qt::black;
          wordColorOpposite = Qt::white;
          selectionColor = QColor(COLOR_SELECTION);
          cursorColor = QColor(COLOR_CURSOR);
      }
  }

  this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  timer = new QTimer(this);
  connect(timer,&QTimer::timeout,[this](){
          showCursor = !showCursor;
      });
}

QHexView::~QHexView() {}

void QHexView::setfileOpened(bool state) {
    fileOpened = state;
}

void QHexView::loadFile(QString p_file)
{
  QFile qFile;

  qFile.setFileName(p_file);

  qFile.open(QFile::ReadOnly);

  if (qFile.isOpen())
  {
    setCursorPos(0);
    resetSelection(0);

    m_pdata = qFile.readAll();

    qFile.close();
  }
  else
    throw std::runtime_error("Falied to open file " + p_file.toStdString());

  restartTimer();
  setfileOpened(true);

//  resetSelection(0);
}

void QHexView::loadFromBuffer(QByteArray &buffer) {
    m_pdata = buffer;
    setCursorPos(0);
    resetSelection(0);
    EditedPos.clear();
    setAddressLength();
    setfileOpened(true);
    restartTimer();
}

// search and set offset
void QHexView::showFromOffset(int offset, int length)
{
    if (offset + length < m_pdata.size())
    {
        startFromAscii = true;
        updatePositions();

        setCursorPos(offset * 2);
        resetSelection(offset * 2);
        setSelection(offset * 2 + length * 2);

        int cursorY = m_cursorPos / (2 * m_bytesPerLine);

        verticalScrollBar()->setValue(cursorY);
        viewport()->update();
    }
}

// clean all
void QHexView::clear()
{
  verticalScrollBar()->setValue(0);
  m_pdata.clear();
  viewport()->update();
}

QSize QHexView::fullSize() const
{
  if (m_pdata.size() == 0)
    return QSize(0, 0);

  int width = m_posAscii + (m_bytesPerLine * m_charWidth);
  int height = m_pdata.size() / m_bytesPerLine;

  if (m_pdata.size() % m_bytesPerLine)
    height++;

  height *= m_charHeight;

  return QSize(width, height);
}

void QHexView::updatePositions()
{
    m_charWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
    m_charHeight = fontMetrics().height();

//  int serviceSymbolsWidth = ADR_LENGTH * m_charWidth + GAP_ADR_HEX + GAP_HEX_ASCII;
  //m_bytesPerLine = (width() - serviceSymbolsWidth) / (4 * m_charWidth) - 1; // 4 symbols per byte

    m_posAddr = 0;
    m_posHex = (m_addressLength + 3) * m_charWidth;
    m_posAscii = m_posHex + (m_bytesPerLine * 3 - 1) * m_charWidth + GAP_HEX_ASCII;
}

/*****************************************************************************/
/* Paint Hex View  */
/*****************************************************************************/
void QHexView::paintEvent(QPaintEvent *event)
{
  if (m_pdata.size() == 0)
    return;

  QPainter painter(viewport());

  updatePositions();
  confScrollBar();

  int firstLineIdx = verticalScrollBar()->value();
  int lastLineIdx = firstLineIdx + viewport()->size().height() / m_charHeight;
  int lastDataIdx = getLineNum() - 1;
  int linePos = m_posAscii - (GAP_HEX_ASCII / 2);
  int horizenLinePosY = m_charHeight + 2;
  int yPosStart = m_charHeight + horizenLinePosY;
//  QBrush def = painter.brush();

//  painter.fillRect(event->rect(), this->palette().color(QPalette::Base));
//  painter.fillRect(QRect(m_posAddr, event->rect().top(), m_posHex - GAP_ADR_HEX + 10, height()), QColor(COLOR_ADDRESS));

  painter.setPen(QPen(Qt::gray, 1));
  painter.drawLine(linePos, horizenLinePosY, linePos, height());
  painter.setPen(QPen(Qt::gray, 1));
  painter.drawLine(0, horizenLinePosY, m_posAscii + m_bytesPerLine * m_charWidth , horizenLinePosY);
  painter.setPen(wordColor);

  // offset drawn
  for (int offsetX = m_posHex, i = 0; i < m_bytesPerLine; ++i, offsetX += m_charWidth * 3) {
      QString offsetVal = QString::number(i, 16);
      painter.drawText(offsetX + m_charWidth / 2, m_charHeight - 2, offsetVal);
  }

  QByteArray data = m_pdata.mid(firstLineIdx * m_bytesPerLine, (lastLineIdx - firstLineIdx) * m_bytesPerLine);
  painter.setPen(wordColor); // paint white characters and binary

  for (int lineIdx = firstLineIdx, yPos = yPosStart; lineIdx < lastLineIdx; lineIdx += 1, yPos += m_charHeight)
  {
    // ascii position
    for (int xPosAscii = m_posAscii, i = 0;
         ((lineIdx - firstLineIdx) * m_bytesPerLine + i) < data.size() &&
         (i < m_bytesPerLine);
         i++, xPosAscii += m_charWidth)
    {
      char character = data[(lineIdx - firstLineIdx) * (uint)m_bytesPerLine + i];
      CHAR_VALID(character);

      int pos = ((lineIdx * m_bytesPerLine + i) * 2) + 1;
      if (isSelected(pos)) {
          painter.fillRect(QRectF(xPosAscii, yPos - m_charHeight + 4, m_charWidth, m_charHeight), selectionColor);
      }

      painter.drawText(xPosAscii, yPos, QString(character));

      painter.setBackground(painter.brush());
      painter.setBackgroundMode(Qt::OpaqueMode);
    }

    // binary position
    for (int xPos = m_posHex, i = 0; i < m_bytesPerLine * 2 &&
        ((lineIdx - firstLineIdx) * m_bytesPerLine + i / 2) < data.size();
         i++, xPos += m_charWidth)
    {
      int pos = ((lineIdx * m_bytesPerLine) * 2) + i;
      if (isSelected(pos)) {
          painter.fillRect(QRectF(xPos, yPos - m_charHeight + 4, m_charWidth, m_charHeight), selectionColor);
          if ((i % 2 == 1) && isSelected(pos + 1) && (i != m_bytesPerLine * 2 - 1)){
              painter.fillRect(QRectF(xPos + m_charWidth, yPos - m_charHeight + 4, m_charWidth, m_charHeight), selectionColor);
          }
      }
      if (isEdited(pos)) {
          painter.fillRect(QRectF(xPos, yPos - m_charHeight + 4, m_charWidth, m_charHeight), EditedColor);
          if ((i % 2 == 1) && isEdited(pos + 1) && (i != m_bytesPerLine * 2 - 1)){
              painter.fillRect(QRectF(xPos + m_charWidth, yPos - m_charHeight + 4, m_charWidth, m_charHeight), EditedColor);
          }
      }

      QString val;
      char binaryNum = data.at((lineIdx - firstLineIdx) * m_bytesPerLine + i / 2);
      if (i % 2 == 0) {
          val = QString::number((binaryNum & 0xF0) >> 4, 16);
      } else {
          val = QString::number(binaryNum & 0xF, 16);
      }

      painter.drawText(xPos, yPos, val.toUpper());

      if (i % 2 == 1){
          xPos += m_charWidth;
      }

      painter.setBackground(painter.brush());
      painter.setBackgroundMode(Qt::OpaqueMode);
    }

    // address drawn
    if (lineIdx <= lastDataIdx) {
        QString address = QString("%1h:").arg(lineIdx * m_bytesPerLine, m_addressLength, 16, QChar('0'));
        painter.drawText(m_posAddr, yPos, address);
    }

    viewport()->update();
  }

  //cursor drawn
  if (hasFocus() && showCursor && (m_cursorPos >= 0))
  {
      int x = (m_cursorPos % (2 * m_bytesPerLine));
      int y = m_cursorPos / (2 * m_bytesPerLine);
      if (y < firstLineIdx || y >= lastLineIdx)
          return;
      y -= firstLineIdx;
      int cursorX;
      if (startFromAscii)
          cursorX = (x / 2) * m_charWidth + m_posAscii;
      else
          cursorX = (((x / 2) * 3) + (x % 2)) * m_charWidth + m_posHex;
      int cursorY = y * m_charHeight + 4;
      painter.fillRect(cursorX, cursorY + horizenLinePosY, m_charWidth, m_charHeight, cursorColor);

      if (startFromAscii) {
          char character = data[y * m_bytesPerLine + x / 2];
          CHAR_VALID(character);
          painter.setPen(wordColorOpposite);
          painter.drawText(cursorX, (y + 1) * m_charHeight + horizenLinePosY, QString(character));
          painter.setPen(wordColor);
      } else {
          QString val;
          char binaryNum = data.at(y * m_bytesPerLine + x / 2);
          if (x % 2 == 0) {
              val = QString::number((binaryNum & 0xF0) >> 4, 16);
          } else {
              val = QString::number(binaryNum & 0xF, 16);
          }
          painter.setPen(wordColorOpposite);
          painter.drawText(cursorX, (y + 1) * m_charHeight + horizenLinePosY, val.toUpper());
          painter.setPen(wordColor);
      }
  }
}

int QHexView::cursorPos(const QPoint &position)
{
    int pos = -1;
    int x = 0;
    int y = 0;

    if (!startFromAscii) {
        if (position.x() < (int)m_posHex)
            x = 0;
        else if (position.x() >= (int)(m_posHex + (m_bytesPerLine * 3 - 1) * m_charWidth))
            x = m_bytesPerLine * 3 - 2;
        else
            x = (position.x() - m_posHex) / m_charWidth;

        if ((x % 3) == 0)
            x = (x / 3) * 2;
        else if((x % 3) == 1)
            x = ((x / 3) * 2) + 1;
        else
            x = ((x / 3) * 2) + 2;
    } else {
        if (position.x() < (int)m_posAscii)
            x = 0;
        else if (position.x() >= (int)(m_posAscii + m_bytesPerLine * m_charWidth))
            x = m_bytesPerLine * 2 - 2;
        else {
            int asciiNum = (position.x() - m_posAscii) / m_charWidth;
            x = 2 * (int)(asciiNum);
        }
    }

    int firstLineIdx = verticalScrollBar()->value();
    int lastLineIdx = viewport()->size().height() / m_charHeight + 1;
    int lastDataIdx = getLineNum() - firstLineIdx;
    int lastPageFirstLineIdx = getLineNum() + BLANK_LINE_NUM - viewport()->size().height() / m_charHeight;
    int lastX_Offset = (m_pdata.size() % m_bytesPerLine) * 2 - 1;

    y = (position.y() / (int)m_charHeight);
    if (y < 0) {
        verticalScrollBar()->setValue(firstLineIdx - 1);
        firstLineIdx = (firstLineIdx > 0) ? firstLineIdx - 1 : 0;
        y = 0;
    }
    else if (y > (viewport()->size().height() / m_charHeight)) {
        verticalScrollBar()->setValue(firstLineIdx + 1);
        if (firstLineIdx < lastPageFirstLineIdx)
            firstLineIdx += 1;
    }
    if (y > lastLineIdx)
        y = lastLineIdx;
    if (y > lastDataIdx)
        y = lastDataIdx;
    if (((m_pdata.size() % m_bytesPerLine) != 0) && (y == lastDataIdx) && (x > lastX_Offset))
        x = lastX_Offset;
    y = (y == 0) ? y : y - 1;
    pos = x + y * 2 * m_bytesPerLine + firstLineIdx * m_bytesPerLine * 2;
  return pos;
}

void QHexView::resetSelection()
{
  m_selectBegin = m_selectInit;
  m_selectEnd = m_selectInit;
}

void QHexView::resetSelection(int pos)
{
  if (pos == std::numeric_limits<int>::max())
    pos = 0;

  m_selectInit = pos;
  m_selectBegin = pos;
  m_selectEnd = pos;
}

void QHexView::setSelection(int pos)
{
  if (pos == std::numeric_limits<int>::max())
    pos = 0;

  if (pos >= (int)m_selectInit)
  {
    m_selectEnd = pos;
    m_selectBegin = m_selectInit;
  }
  else
  {
    m_selectBegin = pos;
    m_selectEnd = m_selectInit;
  }
  if (startFromAscii){
      if (m_selectBegin == m_selectEnd) {
          m_selectEnd += 1;
      } else {
          m_selectEnd -= 1;
      }
  }
}

void QHexView::setCursorPos(int position)
{
  if (position < 0)
    position = 0;

  int maxPos = 0;

  if (m_pdata.size() != 0)
  {
    maxPos = (m_pdata.size() - 1) * 2 + 1;
  }

  if (position > maxPos)
    position = maxPos;

  m_cursorPos = position;
}

int QHexView::getCursorPos()
{
  return m_cursorPos;
}

void QHexView::ensureVisible()
{
  QSize areaSize = viewport()->size();

  int firstLineIdx = verticalScrollBar()->value();
  int lastLineIdx = firstLineIdx + areaSize.height() / m_charHeight;

  int cursorY = m_cursorPos / (2 * m_bytesPerLine);

  if (cursorY < firstLineIdx)
    verticalScrollBar()->setValue(cursorY);
  else if (cursorY >= lastLineIdx)
    verticalScrollBar()->setValue(cursorY - areaSize.height() / m_charHeight + 1);
}

void QHexView::confScrollBar()
{
  QSize areaSize = viewport()->size();
  QSize widgetSize = fullSize();
  verticalScrollBar()->setPageStep(areaSize.height() / m_charHeight);
  verticalScrollBar()->setRange(0, (widgetSize.height() - areaSize.height()) / m_charHeight + BLANK_LINE_NUM);
}

std::size_t QHexView::sizeFile()
{
  return m_pdata.size();
}

void QHexView::setAddressLength()
{
  unsigned int size = (unsigned int) m_pdata.size();
  for(int i = 0; size >> i != 0; ++i) {
      m_addressLength = i + 1;
  }
  int remainder = m_addressLength % 4;
  m_addressLength = m_addressLength / 4 + remainder;
}

bool QHexView::isSelected(int index)
{
    bool ret = false;
    if (index < sizeFile() * 2) {
        if (m_selectBegin != m_selectEnd) {
            if (m_selectBegin < m_selectEnd) {
                ret = (index >= m_selectBegin && index <= m_selectEnd);
            } else {
                ret = (index >= m_selectEnd && index <= m_selectBegin);
            }
        }
    }
    return ret;
}

bool QHexView::isEdited(unsigned int index) {
    bool ret = false;
    if (std::count(EditedPos.begin(), EditedPos.end(), index)) {
        ret = true;
    }
    return ret;
}

void QHexView::restartTimer()
{
    timer->stop();
    showCursor = true;
    timer->start(500);
}

int QHexView::getLineNum()
{
    int num = m_pdata.size() / m_bytesPerLine;
    if (m_pdata.size() % m_bytesPerLine)
        num += 1;
    return num;
}

void QHexView::actionGoto() {
    bool done;
    QString offset = QInputDialog::getText ( this, tr ( "Goto..." ),
                     tr ( "Offset (0x for hexadecimal):" ), QLineEdit::Normal,
                     nullptr, &done );

    if ( done && offset[0] == '0' && offset[1] == 'x' )
      showFromOffset ( offset.toInt ( nullptr, 16 ) );
    else
      showFromOffset ( offset.toInt ( nullptr ) );
}

void QHexView::actionSearch() {
    SearchDialog *settingDialog = new SearchDialog(appDir);
    settingDialog->setSearchMode(true);
//    settingDialog->SetModelData(&FvModelData);
    settingDialog->SetBinaryData(&m_pdata);
    settingDialog->setParentWidget(this);
    settingDialog->exec();
}

void QHexView::setParentWidget(QWidget *pWidget) {
    parentWidget = pWidget;
}

void QHexView::setReadOnly(bool ReadOnlyFlag) {
    ReadOnly = ReadOnlyFlag;
}
