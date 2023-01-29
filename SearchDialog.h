#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <vector>
#include "mainwindow.h"
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

    void setParentWidget(MainWindow *pWidget);
    void SetModelData(vector<FvModel*> *fvModel);
    bool RecursiveSearch(DataModel *model, const QString &str, int depth, bool sameParent=false);
    void SearchFileText();

private slots:

    void on_AsciiCheckbox_stateChanged(int state);
    void on_TextCheckbox_stateChanged(int state);
    void on_SearchContent_textChanged(const QString &arg1);
    void on_NextButton_clicked();
    void on_PreviousButton_clicked();
    void on_SearchContent_returnPressed();

private:
    Ui::SearchDialog *ui;
    MainWindow       *parentWidget;
    vector<FvModel*> *SearchModelData;
    vector<int>      SearchRows;
    vector<int>      PreviousItem;
    static QString   SearchedString;
    bool             SearchAscii{false};
    bool             SearchText{false};
    bool             SearchFound{false};
};

#endif // SEARCHDIALOG_H
