#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QSettings>
#include <vector>
#include "Model.h"

namespace Ui {
class SearchDialog;
}

class SearchItemTree {
private:
    QString name;
    SearchItemTree *prev  {nullptr};
    SearchItemTree *next  {nullptr};
    SearchItemTree *parent{nullptr};
    SearchItemTree *child {nullptr};
    vector<int>    position;
public:
    SearchItemTree();
    ~SearchItemTree();
};

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr);
    ~SearchDialog();

    void initSetting();
    void setParentWidget(QWidget *pWidget);
    void SetBinaryData(QByteArray *BinaryData);
    bool RecursiveSearch(DataModel *model, const QString &str, vector<int> &SearchRow, vector<int> pItem, int depth, bool sameParent=false);
    bool SearchBinary(int *begin, int *length);
    bool SearchBinaryAscii(int *begin, int *length);
    char UpperToLower(char s);
    char LowerToUpper(char s);

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

private:
    Ui::SearchDialog *ui;
    QSettings        setting{"Intel", "BiosViewer"};
    QWidget          *parentWidget;
    vector<DataModel*> *SearchModelData;
    int              PreviousOffset;
    QByteArray       *BinaryBuffer;
    static QString   SearchedString;
    static QString   pSearchedString;
    enum             EndianMode{LittleEndian=0, BigEndian};
    bool             isLittleEndian{true};
    bool             SearchAscii{false};
    bool             CaseSensitive{false};
    QVector<QPair<int, int>>  HistoryResult;
};

#endif // SEARCHDIALOG_H
