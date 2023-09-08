#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QSettings>
#include <vector>
#include "BaseLib.h"

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr);
    ~SearchDialog() override;

    void initSetting();
    void setParentWidget(QWidget *pWidget);
    void SetBinaryData(QByteArray *BinaryData);
    void SearchBinary();
    void SearchBinaryAscii();
    char UpperToLower(char s) const;
    bool containsNonHexCharacters(const QString& str);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

signals:
    void closeSignal(bool State);

private slots:
    void AsciiCheckboxStateChanged(int state);
    void SearchContentTextChanged(const QString &text);
    void NextButtonClicked();
    void PreviousButtonClicked();
    void SearchContentReturnPressed();
    void EndianBoxActivated(int index);
    void CaseCheckboxStateChanged(int state);
    void WideCheckboxStateChanged(int state);

private:
    Ui::SearchDialog *ui;
    QSettings        setting{"Intel", "BiosViewer"};
    QWidget          *parentWidget{};
    INT64            CurrentIndex{-1};
    INT64            CurrentSearchLength{-1};
    QVector<INT64>   matchedSearchIndexes;
    QByteArray       *BinaryBuffer{};
    static QString   SearchedString;
    QStringList      searchHistory;
    enum             EndianMode{LittleEndian=0, BigEndian};
    bool             isLittleEndian{true};
    bool             SearchAscii{false};
    bool             CaseSensitive{false};
    bool             WideCharacter{true};
};

#endif // SEARCHDIALOG_H
