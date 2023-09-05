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

class SearchDialog;

class QHexView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit QHexView(QWidget *parent = nullptr, bool darkMode = false);
    ~QHexView() override;

    void refresh();
    void setFileOpened(bool state);
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

    INT64 AddressPosition{0};
    INT64 CharWidth{0};
    INT64 HexCharPosition;
    INT64 AsciiCharPosition;
    INT64 CharHeight{0};
    INT64 SelectionBegin{0};
    INT64 SelectionEnd{0};
    INT64 SelectionInit{0};
    INT64 CursorPosition{0};
    INT64 BytesPerHexLine{MIN_BYTES_PER_LINE};
    INT64 AddressLength{ADR_LENGTH};
    INT64 RelativeAdressBase{0};

    char inputKey{};
    bool BinaryEdited{false};
    bool ReadOnly{false};
    std::vector<INT64> EditedPos;

    bool isDarkMode{false};
    bool ShowCursor{true};
    bool StartFromAscii{false};
    bool FileOpened{false};
    bool startFromMainWindow{false};
    QTimer *timer{nullptr};
    QFont HexFontSetting{"Courier New", 12, QFont::Normal, false};
    QFont AsciiFontSetting{"Courier New", 12, QFont::Normal, false};
    QWidget *parentWidget{nullptr};
    QColor SelectionColor{COLOR_SELECTION};
    QColor EditedColor{255, 128, 128, 0xff};
    QColor SelectedEditedColor{143, 122, 46, 0xff};
    QColor WordColor{Qt::black};
    QColor WordColorOpposite{Qt::white};
    QColor CursorColor{COLOR_CURSOR};
    SearchDialog *HexSearchDialog{nullptr};
    bool         HexSearchDialogOpened{false};
    QSettings setting{QSettings("Intel", "BiosViewer")};
    QSettings SysSettings{R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)", QSettings::NativeFormat};

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
    void resetSelection(INT64 pos);
    void setSelection(INT64 pos);
    void ensureVisible();
    void setCursorPos(INT64 pos);
    INT64 cursorPos(const QPoint &position);
    INT64 getCursorPos() const;
    void confScrollBar();
    bool isSelected(INT64 index);
    bool isEdited(INT64 index);
    void restartTimer();
    INT64 getLineNum();
    void initRightMenu();
    void finiRightMenu();
    bool getSelectedBuffer(QByteArray &buffer, INT64*length);
    void binaryEdit(char inputChar);

public slots:
    void loadFile(const QString& p_file);
    void loadFromBuffer(QByteArray &buffer);
    void clear();
    void showFromOffset(INT64 offset, INT64 length = 1);
    std::size_t sizeFile();
    void setAddressLength();
    void setRelativeAddress(INT64 address = 0);
    void CopyFromSelectedContent();
    void PasteToContent();
    void PasteAndInsertToContent();
    void PasteAndOverlapToContent();
    void DiscardChangedContent();
    void SetEditingState(bool state);
    void SaveSelectedContent();
    void setSearchDialogState(bool opened);
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
