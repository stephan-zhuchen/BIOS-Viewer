#pragma once

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QFile>
#include <QLabel>
#include <QTimer>
#include <QSettings>
#include "SymbolDefinition.h"

class HexSearch;

class QHexView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit QHexView(QWidget *parent = nullptr);
    ~QHexView() override;

    void InitSetting();
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
    void resizeEvent(QResizeEvent *event) override;

private:
    QByteArray HexDataArray;
    QByteArray CopiedData;

// Layout
    INT32 AddressPosition{16};
    INT32 HexCharPosition{0};
    INT32 AsciiCharPosition{0};
    INT32 HexAsciiGap{20};
    INT32 CharWidth{0};
    INT32 CharHeight{0};
    INT32 BlankEndingLineNum{10};
    INT64 SelectionBegin{0};
    INT64 SelectionEnd{0};
    INT64 SelectionInit{0};
    INT64 CursorPosition{0};
    INT32 BytesPerHexLine{16};
    INT32 AddressLength{8};
    INT64 RelativeAdressBase{0};

    char inputKey{};
    bool BinaryEdited{false};
    bool ReadOnly{false};
    QVector<INT64> EditedPos;
    QHash<INT32, INT32> HexToPos;
    QHash<INT32, INT32> PosToHex;

    bool isDarkMode{false};
    bool DoubleHexLine{false};
    bool ShowCursor{true};
    bool StartFromAscii{false};
    bool FileOpened{false};
    bool startFromMainWindow{false};
    bool HexSearchDialogOpened{false};
    QTimer *timer;

    // Color and Font
    QFont HexFontSetting{"Courier New", 12, QFont::Normal, false};
    QFont AsciiFontSetting{"Courier New", 12, QFont::Normal, false};

    QColor SelectionColor{199, 199, 199, 0xff};
    QColor EditedColor{255, 128, 128, 0xff};
    QColor SelectedEditedColor{143, 122, 46, 0xff};
    QColor WordColor{Qt::black};
    QColor WordColorOpposite{Qt::white};
    QColor CursorColor{38, 95, 153, 0xff};

    HexSearch *HexSearchDialog{nullptr};
    QWidget *parentWidget{nullptr};

    QSettings setting{QSettings("Intel", "BiosViewer")};
    QSettings SysSettings{R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)", QSettings::NativeFormat};

    QMenu  *CustomMenu{nullptr};
    QMenu  *DigestMenu{nullptr};
    QMenu  *ChecksumMenu{nullptr};
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
    void  UpdateLayout();
    void  UpdateHexPosition();
    INT32 HexIndexToCursorPos(INT32 HexIndex) const;
    INT32 CursorPosToHexIndex(INT32 CursorPos) const;
    void  ResetSelection(INT64 pos);
    void  setSelection(INT64 pos);
    void  ScrollToSelectedHexContent();
    CHAR8 FilterAscii(CHAR8 character);
    void  setCursorPos(INT64 pos);
    INT64 cursorPos(const QPoint &position);
    void  confScrollBar();
    bool  isSelected(INT64 index);
    bool  isEdited(INT64 index);
    void  restartTimer();
    INT32 getLineNum();
    void  InitCustomMenu();
    void  CleanupCustomMenu();
    bool  getSelectedBuffer(QByteArray &buffer, INT64*length);
    void  binaryEdit(char inputChar);

public slots:
    void loadFile(const QString& p_file);
    void loadFromBuffer(QByteArray &buffer);
    void clear();
    void showFromOffset(INT64 offset, INT64 length = 1);
    void setAddressLength();
    void setRelativeAddress(INT64 address = 0);
    void setSearchDialogState(bool opened);

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
