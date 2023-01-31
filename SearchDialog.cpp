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

    if (setting.value("Theme").toString() == "Dark") {
        QFile styleFile(":/qdarkstyle/dark/darkstyle.qss");
        if(styleFile.open(QIODevice::ReadOnly)) {
            QString setStyleSheet(styleFile.readAll());
            this->setStyleSheet(setStyleSheet);
            styleFile.close();
        }
    } else if (setting.value("Theme").toString() == "Light") {
        QFile styleFile(":/qdarkstyle/light/lightstyle.qss");
        if(styleFile.open(QIODevice::ReadOnly)) {
            QString setStyleSheet(styleFile.readAll());
            this->setStyleSheet(setStyleSheet);
            styleFile.close();
        }
    }
}

SearchDialog::~SearchDialog()
{
    SearchFound = false;
    PreviousItem.clear();
    delete ui;
}

QString SearchDialog::SearchedString = "";

void SearchDialog::setParentWidget(QWidget *pWidget) {
    parentWidget = (MainWindow*)pWidget;
}

void SearchDialog::SetModelData(vector<FvModel*> *fvModel) {
    if (!isBinary)
        SearchModelData = fvModel;
}

void SearchDialog::SetBinaryData(QByteArray *BinaryData) {
    if (isBinary)
        BinaryBuffer = BinaryData;
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

void SearchDialog::SearchBinary() {
    int idx = BinaryBuffer->indexOf('x', 0x10);
    cout << "idx = " << hex << idx << endl;
}

bool SearchDialog::SearchBinaryAscii(int *begin, int *length) {
    bool Found = true;
    bool wFound = true;
    int matchIdx = -1;

    // Search Lower case
    while (true) {
        matchIdx = BinaryBuffer->indexOf(SearchedString.at(0).toLatin1(), matchIdx + 1);
        if (matchIdx == -1) {
            break;
        }
        for (int strIdx = 1; strIdx < SearchedString.size(); ++strIdx) {
            if (UpperToLower(BinaryBuffer->at(matchIdx + strIdx)) != SearchedString.at(strIdx).toLatin1()) {
                Found = false;
                break;
            }
        }
        for (int strIdx = 1; strIdx < SearchedString.size(); ++strIdx) {
            if (UpperToLower(BinaryBuffer->at(matchIdx + strIdx * 2)) != SearchedString.at(strIdx).toLatin1()) {
                wFound = false;
                break;
            }
        }
        if (Found || wFound) {
            cout << "Found" << endl;
            *begin = matchIdx;
            *length = SearchedString.size();
            return true;
        }
    }

    // Search Upper case
    matchIdx = -1;
    Found = true;
    wFound = true;
    while (true) {
        matchIdx = BinaryBuffer->indexOf(SearchedString.at(0).toUpper().toLatin1(), matchIdx + 1);
        if (matchIdx == -1) {
            break;
        }
        for (int strIdx = 1; strIdx < SearchedString.size(); ++strIdx) {
            if (LowerToUpper(BinaryBuffer->at(matchIdx + strIdx)) != SearchedString.at(strIdx).toUpper().toLatin1()) {
                Found = false;
                break;
            }
        }
        for (int strIdx = 1; strIdx < SearchedString.size(); ++strIdx) {
            if (LowerToUpper(BinaryBuffer->at(matchIdx + strIdx * 2)) != SearchedString.at(strIdx).toUpper().toLatin1()) {
                wFound = false;
                break;
            }
        }
        if (Found || wFound) {
            cout << "Found" << endl;
            *begin = matchIdx;
            *length = SearchedString.size();
            return true;
        }
    }
    return false;
}

void SearchDialog::setSearchMode(bool searchBinary) {
    isBinary = searchBinary;
    if (isBinary) {
        ui->TextCheckbox->setCheckState(Qt::Unchecked);
        ui->TextCheckbox->setDisabled(true);
    } else {
        ui->TextCheckbox->setCheckState(Qt::Checked);
        ui->AsciiCheckbox->setCheckState(Qt::Unchecked);
        ui->AsciiCheckbox->setDisabled(true);
    }
}

char SearchDialog::UpperToLower(char s) {
    if(s >= 65 && s <= 90)
        s = s + 32;
    return s;
}

char SearchDialog::LowerToUpper(char s) {
    if(s >= 97 && s <= 122)
        s = s - 32;
    return s;
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
    int beginOffset = 0;
    int searchLength = 0;
    if (!isBinary && SearchText) {
        SearchFileText();
    } else if (isBinary && SearchAscii) {
        SearchBinaryAscii(&beginOffset, &searchLength);
    } else if (isBinary) {
        SearchBinary();
    }
}


void SearchDialog::on_PreviousButton_clicked()
{

}


void SearchDialog::on_SearchContent_returnPressed()
{
    on_NextButton_clicked();
}

