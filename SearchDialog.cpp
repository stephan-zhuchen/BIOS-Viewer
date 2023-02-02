#include <QDebug>
#include <QRegularExpression>
#include "SearchDialog.h"
#include "lib/QHexView/qhexview.h"
#include "ui_SearchDialog.h"

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog),
    SearchModelData(nullptr),
    PreviousOffset(-1)
{
    ui->setupUi(this);
    ui->SearchContent->setAttribute(Qt::WA_InputMethodEnabled, false);
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
    parentWidget = pWidget;
}

void SearchDialog::SetModelData(vector<FvModel*> *fvModel) {
    if (!isBinary)
        SearchModelData = fvModel;
}

void SearchDialog::SetBinaryData(QByteArray *BinaryData) {
    if (isBinary) {
        BinaryBuffer = BinaryData;
    }
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
    ((MainWindow*)parentWidget)->HighlightTreeItem(SearchRows);
}

bool SearchDialog::SearchBinary(int *begin, int *length) {
    static QRegularExpression re("\\s");
    QString number = SearchedString.remove(re);
    if (number.size() % 2 == 1) {
        number = "0" + number;
    }
    QStringList littleEndianNum;
    QVector<char> searchNum;
    for (int idx = 0; idx < number.size(); idx += 2) {
        littleEndianNum.insert(0, number.mid(idx, 2));
    }
    for (int var = 0; var < littleEndianNum.size(); ++var) {
        searchNum.push_back(littleEndianNum.at(var).toInt(nullptr, 16));
        cout << hex << littleEndianNum.at(var).toInt(nullptr, 16) << endl;
    }

    // Search little endian binary
    int matchIdx = PreviousOffset;
    while (true) {
        bool Found = true;
        matchIdx = BinaryBuffer->indexOf(searchNum.at(0), matchIdx + 1);
        if (matchIdx == -1) {
            break;
        }
        for (int strIdx = 1; strIdx < searchNum.size(); ++strIdx) {
            if (BinaryBuffer->at(matchIdx + strIdx) != searchNum.at(strIdx)) {
                cout << (UINT16)BinaryBuffer->at(matchIdx + strIdx) << endl;
                Found = false;
                break;
            }
        }
        if (Found) {
            *begin = matchIdx;
            *length = searchNum.size();
            PreviousOffset = matchIdx;
            cout << "matchIdx = " << hex << matchIdx << endl;
            return true;
        }
    }
    return false;
}

bool SearchDialog::SearchBinaryAscii(int *begin, int *length) {
    for (int idx = PreviousOffset + 1; idx < BinaryBuffer->size(); ++idx) {
        bool Found = true;
        bool wFound = true;
        if (UpperToLower(BinaryBuffer->at(idx)) == SearchedString.at(0).toLatin1()) {
            for (int strIdx = 1; strIdx < SearchedString.size(); ++strIdx) {
                if (UpperToLower(BinaryBuffer->at(idx + strIdx)) != SearchedString.at(strIdx).toLatin1()) {
                    Found = false;
                    break;
                }
            }
            for (int strIdx = 1; strIdx < SearchedString.size(); ++strIdx) {
                if (BinaryBuffer->at(idx + strIdx * 2 - 1) != 0 || UpperToLower(BinaryBuffer->at(idx + strIdx * 2)) != SearchedString.at(strIdx).toLatin1()) {
                    wFound = false;
                    break;
                }
            }
            if (Found || wFound) {
                cout << "Found" << endl;
                cout << "matchIdx = " << hex << idx << endl;
                *begin = idx;
                *length = SearchedString.size();
                PreviousOffset = idx;
                return true;
            }
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
    PreviousOffset = -1;
    HistoryResult.clear();
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
    PreviousOffset = -1;
    HistoryResult.clear();
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
    PreviousOffset = -1;
    HistoryResult.clear();
    SearchedString = arg1.toLower();
}

void SearchDialog::on_NextButton_clicked()
{
    int beginOffset = 0;
    int searchLength = 0;
    if (!isBinary && SearchText) {
        SearchFileText();
    } else if (isBinary && SearchAscii) {
        if (SearchBinaryAscii(&beginOffset, &searchLength)) {
            HistoryResult.push_back(QPair<int, int>(beginOffset, searchLength));
            ((QHexView*)parentWidget)->showFromOffset(beginOffset, searchLength);
        }
    } else if (isBinary) {
        if (SearchBinary(&beginOffset, &searchLength)) {
            HistoryResult.push_back(QPair<int, int>(beginOffset, searchLength));
            ((QHexView*)parentWidget)->showFromOffset(beginOffset, searchLength);
        }
    }
}

void SearchDialog::on_PreviousButton_clicked()
{
    if (isBinary) {
        if (HistoryResult.size() <= 1)
            return;
        HistoryResult.pop_back();
        QPair<int, int> lastResult = HistoryResult.back();
        int beginOffset = lastResult.first;
        int searchLength = lastResult.second;
        PreviousOffset = beginOffset;
        ((QHexView*)parentWidget)->showFromOffset(beginOffset, searchLength);
    }
}


void SearchDialog::on_SearchContent_returnPressed()
{
    on_NextButton_clicked();
}

