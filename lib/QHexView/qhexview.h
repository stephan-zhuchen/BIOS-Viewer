#pragma once

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QFile>
#include <QLabel>
#include <QTimer>
#include <QSettings>

// config font and colors
#define FONT "Courier"
#define SIZE_FONT 11
#define COLOR_CHARACTERS Qt::black
#define COLOR_SELECTION 199, 199, 199, 0xff
#define COLOR_ADDRESS 199, 199, 199, 0xff
#define COLOR_CURSOR 38, 95, 153, 0xff

// config lines
#define MIN_HEXCHARS_IN_LINE 47
#define GAP_ADR_HEX 16
#define GAP_HEX_ASCII 16
#define MIN_BYTES_PER_LINE 16
#define ADR_LENGTH 8
#define BLANK_LINE_NUM 10

class QOffsetView : public QWidget
{
    Q_OBJECT
public:
    QOffsetView(QWidget *parent = nullptr);
    QLabel offset;
};

class QHexView : public QAbstractScrollArea

{
  Q_OBJECT
public:
  QHexView(QWidget *parent = nullptr);
  ~QHexView();

  void setfileOpened(bool state);
  void setParentWidget(QWidget *pWidget);

  void paintEvent(QPaintEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  QByteArray m_pdata;

  unsigned int m_posAddr,
      m_charWidth,
      m_posHex,
      m_posAscii,
      m_charHeight,
      m_selectBegin,
      m_selectEnd,
      m_selectInit,
      m_cursorPos,
      m_bytesPerLine,
      m_addressLength;

  bool      showCursor;
  bool      startFromAscii;
  bool      fileOpened;
  QTimer    *timer;
  QFont     fontSetting;
  QWidget   *parentWidget;
  QColor    selectionColor;
  QColor    wordColor;
  QColor    wordColorOpposite;
  QColor    cursorColor;
  QSettings setting{"./Setting.ini", QSettings::IniFormat};
  QSettings SysSettings{"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat};

  QSize fullSize() const;
  void updatePositions();
  void resetSelection();
  void resetSelection(int pos);
  void setSelection(int pos);
  void ensureVisible();
  void setCursorPos(int pos);
  int  cursorPos(const QPoint &position);
  int  getCursorPos();
  void paintMark(int xpos, int ypos);
  void confScrollBar();
  bool isSelected(int index);
  void restartTimer();
  int  getLineNum();
  void actionGoto();
  void actionSearch();

public slots:
  void loadFile(QString p_file);
  void loadFromBuffer(QByteArray buffer);
  void clear();
  void showFromOffset(int offset, int length=1);
  std::size_t sizeFile();
  void setAddressLength();
};
