#include <QDebug>
#include <QRegularExpression>
#include <QMessageBox>
#include "mainwindow.h"
#include "SearchDialog.h"
#include "QHexView/qhexview.h"
#include "ui_SearchDialog.h"

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog),
    SearchModelData(nullptr),
    PreviousOffset(-1)
{
    ui->setupUi(this);

    connect(ui->AsciiCheckbox,  SIGNAL(stateChanged(int)),    this, SLOT(AsciiCheckboxStateChanged(int)));
    connect(ui->TextCheckbox,   SIGNAL(stateChanged(int)),    this, SLOT(TextCheckboxStateChanged(int)));
    connect(ui->SearchContent,  SIGNAL(textChanged(QString)), this, SLOT(SearchContentTextChanged(QString)));
    connect(ui->NextButton,     SIGNAL(clicked()),            this, SLOT(NextButtonClicked()));
    connect(ui->PreviousButton, SIGNAL(clicked()),            this, SLOT(PreviousButtonClicked()));
    connect(ui->SearchContent,  SIGNAL(returnPressed()),      this, SLOT(SearchContentReturnPressed()));

    ui->SearchContent->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->SearchContent->setText(SearchedString);
    ui->SearchContent->selectAll();
    setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}

SearchDialog::~SearchDialog()
{
    PreviousItems.clear();
    delete ui;
}

QString SearchDialog::SearchedString = "";
QString SearchDialog::pSearchedString = "";

void SearchDialog::setParentWidget(QWidget *pWidget) {
    parentWidget = pWidget;
}

void SearchDialog::SetModelData(vector<DataModel*> *fvModel) {
    if (!isBinary) {
        SearchModelData = fvModel;
        vector<int> pos;
        setTextList(SearchModelData, pos);

        for (int var = 0; var < SearchPositionList.size(); ++var) {
            SearchPositionList.at(var).at(0) += 1;
        }
    }
}

void SearchDialog::SetBinaryData(QByteArray *BinaryData) {
    if (isBinary) {
        BinaryBuffer = BinaryData;
    }
}

void SearchDialog::setTextList(vector<DataModel*> *itemsModel, vector<int> position) {
    for (int i = 0; i < itemsModel->size(); ++i) {
        vector<int> parentPos = position;
        DataModel* item = itemsModel->at(i);
        parentPos.push_back(i);
        QString name = item->getName().toLower();
        SearchTextList.push_back(name);
        SearchPositionList.push_back(parentPos);
        setTextList(&(item->volumeModelData), parentPos);
    }
}

void SearchDialog::SearchFileText() {
    int PreviousRow = -1;
    if (SearchedString != pSearchedString) {
        PreviousItems.clear();
    }
    if (PreviousItems.size() != 0) {
        PreviousRow = PreviousItems.at(PreviousItems.size() - 1);
    }
    if (SearchModelData->size() == 0 || SearchedString == "")
        return;

    int row = PreviousRow + 1;
    while (row < SearchTextList.size()) {
        if (SearchTextList.at(row).contains(SearchedString, Qt::CaseInsensitive)) {
            break;
        }
        row += 1;
    }

    if (row == SearchTextList.size()) {
        QMessageBox::about(this, tr("Search"), "Not Found!");
        return;
    }
    PreviousItems.push_back(row);
    ((MainWindow*)parentWidget)->HighlightTreeItem(SearchPositionList.at(row));
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
        littleEndianNum.append(number.mid(idx, 2));
    }
    for (int var = 0; var < littleEndianNum.size(); ++var) {
        searchNum.push_back(littleEndianNum.at(var).toInt(nullptr, 16));
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
                Found = false;
                break;
            }
        }
        if (Found) {
            *begin = matchIdx;
            *length = searchNum.size();
            PreviousOffset = matchIdx;
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

void SearchDialog::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        this->close();
    }
}

void SearchDialog::closeEvent(QCloseEvent *event) {
    ((MainWindow*)parentWidget)->setSearchDialogState(false);
}

void SearchDialog::AsciiCheckboxStateChanged(int state)
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

void SearchDialog::TextCheckboxStateChanged(int state)
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

void SearchDialog::SearchContentTextChanged(const QString &arg1)
{
    PreviousOffset = -1;
    HistoryResult.clear();
    SearchedString = arg1.toLower();
}

void SearchDialog::NextButtonClicked()
{
    qDebug("NextButtonClicked");
    int beginOffset = 0;
    int searchLength = 0;
    if (!isBinary && SearchText) {
        SearchFileText();
        pSearchedString = SearchedString;
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

void SearchDialog::PreviousButtonClicked()
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
    } else {
        if (SearchedString != pSearchedString) {
            PreviousItems.clear();
        }
        if (PreviousItems.size() > 1) {
            int PreviousRow = PreviousItems.at(PreviousItems.size() - 2);
            PreviousItems.pop_back();
            ((MainWindow*)parentWidget)->HighlightTreeItem(SearchPositionList.at(PreviousRow));
        }
    }
}


void SearchDialog::SearchContentReturnPressed()
{
    NextButtonClicked();
}

