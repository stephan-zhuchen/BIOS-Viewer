#include <QDebug>
#include "SearchDialog.h"
#include "ui_SearchDialog.h"

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog),
    SearchModelData(nullptr)
{
    ui->setupUi(this);
    ui->SearchContent->setText(SearchedString);
    ui->SearchContent->selectAll();
    setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}

SearchDialog::~SearchDialog()
{
    SearchFound = false;
    PreviousItem.clear();
    delete ui;
}

QString SearchDialog::SearchedString = "";

void SearchDialog::setParentWidget(MainWindow *pWidget) {
    parentWidget = pWidget;
}

void SearchDialog::SetModelData(vector<FvModel*> *fvModel) {
    SearchModelData = fvModel;
}

bool SearchDialog::RecursiveSearch(DataModel *model, const QString &str, int depth, bool sameParent) {
    int row = 0;
    bool isSameParent = false;
    if (sameParent && PreviousItem.size() == depth + 1) {
        row = PreviousItem.at(depth) + 1;
    } else if (sameParent && PreviousItem.size() > depth + 1) {
        isSameParent = true;
        row = PreviousItem.at(depth);
    }

    if (model->getName().contains(str, Qt::CaseInsensitive) || model->getText().contains(str, Qt::CaseInsensitive)) {
        qDebug() << str << " Found!";
        SearchFound = true;
        return SearchFound;
    }
    while (row < model->volumeModelData.size() && SearchFound == false) {
        for (int var = 0; var < depth; ++var) {
            cout << "-";
        }
        cout << "Row: " << row << ", num= " << model->volumeModelData.size() << endl;

        DataModel *childModel = model->volumeModelData.at(row);
        if (PreviousItem.size() > depth && row != PreviousItem.at(depth))
            isSameParent = false;
        if (RecursiveSearch(childModel, str, depth + 1, isSameParent) == true) {
            SearchRows.emplace(SearchRows.begin(), row);
            break;
        }
        row += 1;
    }
    return SearchFound;
}

void SearchDialog::SearchFileText() {
    SearchRows.clear();
    if (SearchModelData->size() == 0 || SearchedString == "")
        return;
    int row = 0;
    bool isSameParent = false;
    if (PreviousItem.size() != 0) {
        isSameParent = true;
        row = PreviousItem.at(0) - 1;
    }

    while (row < SearchModelData->size()) {
        cout << "Row: " << row << endl;
        DataModel *model = SearchModelData->at(row);
        if (PreviousItem.size() != 0 && row != PreviousItem.at(0) - 1)
            isSameParent = false;

        if (RecursiveSearch(model, SearchedString, 1, isSameParent) == true) {
            SearchRows.emplace(SearchRows.begin(), row + 1);
            break;
        }
        row += 1;
    }
    for (auto row:SearchRows) {
        cout << row << " ";
    }
    cout << endl;
    PreviousItem = SearchRows;
    SearchFound = false;
    parentWidget->HighlightTreeItem(SearchRows);
}

void SearchDialog::on_AsciiCheckbox_stateChanged(int state)
{
    if (state == Qt::Checked)
    {
        SearchAscii = true;
        SearchText = false;
        ui->TextCheckbox->setCheckState(Qt::Unchecked);
    }
    else if (state == Qt::Unchecked)
    {
        SearchAscii = false;
        SearchText = true;
    }
}

void SearchDialog::on_TextCheckbox_stateChanged(int state)
{
    if (state == Qt::Checked)
    {
        SearchAscii = false;
        SearchText = true;
        ui->AsciiCheckbox->setCheckState(Qt::Unchecked);
    }
    else if (state == Qt::Unchecked)
    {
        SearchAscii = true;
        SearchText = false;
    }
}

void SearchDialog::on_SearchContent_textChanged(const QString &arg1)
{
    SearchedString = arg1.toLower();
}

void SearchDialog::on_NextButton_clicked()
{
    if (SearchText == true) {
        SearchFileText();
    }
}


void SearchDialog::on_PreviousButton_clicked()
{

}


void SearchDialog::on_SearchContent_returnPressed()
{
    on_NextButton_clicked();
}

