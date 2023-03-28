#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <vector>
#include "lib/Model.h"

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr);
    ~SearchDialog();

    void setParentWidget(QWidget *pWidget);
    void SetModelData(vector<FvModel*> *fvModel);
    void SetBinaryData(QByteArray *BinaryData);
    bool RecursiveSearch(DataModel *model, const QString &str, int depth, bool sameParent=false);
    void SearchFileText();
    bool SearchBinary(int *begin, int *length);
    bool SearchBinaryAscii(int *begin, int *length);
    void setSearchMode(bool searchBinary);
    static char UpperToLower(char s);
    static char LowerToUpper(char s);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:

    void on_AsciiCheckbox_stateChanged(int state);
    void on_TextCheckbox_stateChanged(int state);
    void on_SearchContent_textChanged(const QString &arg1);
    void on_NextButton_clicked();
    void on_PreviousButton_clicked();
    void on_SearchContent_returnPressed();

private:
    Ui::SearchDialog *ui;
    QWidget          *parentWidget;
    vector<FvModel*> *SearchModelData;
    vector<int>      SearchRows;
    vector<int>      PreviousItem;
    int              PreviousOffset;
    QByteArray       *BinaryBuffer;
    static QString   SearchedString;
    bool             isBinary{false};
    bool             SearchAscii{false};
    bool             SearchText{false};
    bool             SearchFound{false};
    QVector<QPair<int, int>>  HistoryResult;
};

#endif // SEARCHDIALOG_H
