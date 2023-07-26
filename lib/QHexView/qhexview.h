#pragma once

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QFile>
#include <QLabel>
#include <QTimer>
#include <QSettings>
#include "SymbolDefinition.h"

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

class QHexView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit QHexView(QWidget *parent = nullptr);
    ~QHexView() override;

    void refresh();
    void setfileOpened(bool state);
    void setParentWidget(QWidget *pWidget, bool fromMainWindow);
    void setReadOnly(bool ReadOnlyFlag);
    void actionGoto();
    void actionSearch();

    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QByteArray HexDataArray;
    QByteArray CopiedData;

    INT32 AddressPosition{0};
    INT32 CharWidth{0};
    INT32 HexCharPosition;
    INT32 AsciiCharPosition;
    INT32 CharHeight{0};
    INT32 SelectionBegin{0};
    INT32 SelectionEnd{0};
    INT32 SelectionInit{0};
    INT32 CursorPosition{0};
    INT32 BytesPerHexLine{MIN_BYTES_PER_LINE};
    INT32 AddressLength{ADR_LENGTH};

    char inputKey{};
    bool BinaryEdited{false};
    bool ReadOnly{false};
    std::vector<INT32> EditedPos;

    bool isDarkMode{false};
    bool ShowCursor{true};
    bool StartFromAscii{false};
    bool FileOpened{false};
    bool startFromMainWindow{false};
    QTimer *timer{nullptr};
    QFont FontSetting{"Courier New", 12, QFont::Normal, false};
    QWidget *parentWidget{nullptr};
    QColor SelectionColor{COLOR_SELECTION};
    QColor EditedColor{255, 128, 128, 0xff};
    QColor SlectedEditedColor{143, 122, 46, 0xff};
    QColor WordColor{Qt::black};
    QColor WordColorOpposite{Qt::white};
    QColor CursorColor{COLOR_CURSOR};
    QSettings setting{QSettings("Intel", "BiosViewer")};
    QSettings SysSettings{"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat};

    QMenu *RightMenu{nullptr};
    QMenu *DigestMenu{nullptr};
    QMenu *ChecksumMenu{nullptr};
    QAction *CopyContent{nullptr};
    QAction *PasteInsertContent{nullptr};
    QAction *PasteOverlapContent{nullptr};
    QAction *EnableEditing{nullptr};
    QAction *DiscardChange{nullptr};
    QAction *SaveContent{nullptr};
    QAction *CheckSum8{nullptr};
    QAction *CheckSum16{nullptr};
    QAction *CheckSum32{nullptr};
    QAction *md5_Menu{nullptr};
    QAction *sha1_Menu{nullptr};
    QAction *sha224_Menu{nullptr};
    QAction *sha256_Menu{nullptr};
    QAction *sha384_Menu{nullptr};
    QAction *sha512_Menu{nullptr};

    QSize fullSize() const;
    void updatePositions();
    void resetSelection();
    void resetSelection(INT32 pos);
    void setSelection(INT32 pos);
    void ensureVisible();
    void setCursorPos(INT32 pos);
    INT32 cursorPos(const QPoint &position);
    INT32 getCursorPos();
    void confScrollBar();
    bool isSelected(INT32 index);
    bool isEdited(INT32 index);
    void restartTimer();
    INT32 getLineNum();
    void initRightMenu();
    void finiRightMenu();
    bool getSelectedBuffer(QByteArray &buffer, INT32*length);
    void binaryEdit(char inputChar);

public slots:
    void loadFile(const QString& p_file);
    void loadFromBuffer(QByteArray &buffer);
    void clear();
    void showFromOffset(INT32 offset, INT32 length = 1);
    std::size_t sizeFile();
    void setAddressLength();
    void CopyFromSelectedContent();
    void PasteToContent();
    void PasteAndInsertToContent();
    void PasteAndOverlapToContent();
    void DiscardChangedContent();
    void SetEditingState(bool state);
    void SaveSelectedContent();
    void getChecksum8();
    void getChecksum16();
    void getChecksum32();
    void getMD5();
    void getSHA1();
    void getSHA224();
    void getSHA256();
    void getSHA384();
    void getSHA512();
};
