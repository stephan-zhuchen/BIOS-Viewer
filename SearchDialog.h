#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QSettings>
#include <vector>
#include "lib/Model.h"

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
    QVector<QString> SearchTextList;
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

    void setParentWidget(QWidget *pWidget);
    void SetModelData(vector<DataModel*> *fvModel);
    void SetBinaryData(QByteArray *BinaryData);
    bool RecursiveSearch(DataModel *model, const QString &str, vector<int> &SearchRow, vector<int> pItem, int depth, bool sameParent=false);
    void SearchFileText();
    bool SearchBinary(int *begin, int *length);
    bool SearchBinaryAscii(int *begin, int *length);
    void setSearchMode(bool searchBinary);
    void setTextList(vector<DataModel*> *itemsModel, vector<int> position);
    static char UpperToLower(char s);
    static char LowerToUpper(char s);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:

    void AsciiCheckboxStateChanged(int state);
    void TextCheckboxStateChanged(int state);
    void SearchContentTextChanged(const QString &arg1);
    void NextButtonClicked();
    void PreviousButtonClicked();
    void SearchContentReturnPressed();

private:
    Ui::SearchDialog *ui;
    QSettings        setting{"Intel", "BiosViewer"};
    QWidget          *parentWidget;
    vector<DataModel*> *SearchModelData;
    int              PreviousOffset;
    QByteArray       *BinaryBuffer;
    static QString   SearchedString;
    static QString   pSearchedString;
    bool             isBinary{false};
    bool             SearchAscii{false};
    bool             SearchText{false};
    QVector<QString> SearchTextList;
    vector<vector<int>>       SearchPositionList;
    QVector<QPair<int, int>>  HistoryResult;
    vector<int>      PreviousItems;
};

#endif // SEARCHDIALOG_H
