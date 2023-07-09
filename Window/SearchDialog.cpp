#include <QDebug>
#include <QRegularExpression>
#include <QMessageBox>
#include <QKeyEvent>
#include <algorithm>
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
    initSetting();

    connect(ui->AsciiCheckbox,  SIGNAL(stateChanged(int)),    this, SLOT(AsciiCheckboxStateChanged(int)));
    connect(ui->CaseCheckbox,   SIGNAL(stateChanged(int)),    this, SLOT(CaseCheckboxStateChanged(int)));
    connect(ui->SearchContent,  SIGNAL(textChanged(QString)), this, SLOT(SearchContentTextChanged(QString)));
    connect(ui->NextButton,     SIGNAL(clicked()),            this, SLOT(NextButtonClicked()));
    connect(ui->PreviousButton, SIGNAL(clicked()),            this, SLOT(PreviousButtonClicked()));
    connect(ui->SearchContent,  SIGNAL(returnPressed()),      this, SLOT(SearchContentReturnPressed()));
    connect(ui->EndianBox,      SIGNAL(activated(int)),       this, SLOT(EndianBoxActivated(int)));

    ui->SearchContent->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->SearchContent->setText(SearchedString);
    ui->SearchContent->selectAll();
    setAttribute(Qt::WA_DeleteOnClose);
//    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}

SearchDialog::~SearchDialog()
{
    qDebug() << "SearchDialog::~SearchDialog";
    delete ui;
}

void SearchDialog::initSetting() {
    if (!setting.contains("Endian"))
        setting.setValue("Endian", "little");
    if (!setting.contains("SearchAscii"))
        setting.setValue("SearchAscii", "false");
    if (!setting.contains("CaseSensitive"))
        setting.setValue("CaseSensitive", "false");

    if (setting.value("Endian") == "little") {
        isLittleEndian = true;
        ui->EndianBox->setCurrentIndex(EndianMode::LittleEndian);
    } else if (setting.value("Endian") == "big") {
        isLittleEndian = false;
        ui->EndianBox->setCurrentIndex(EndianMode::BigEndian);
    }
    if (setting.value("SearchAscii") == "true") {
        SearchAscii = true;
        ui->AsciiCheckbox->setCheckState(Qt::Checked);
    } else if (setting.value("SearchAscii") == "false") {
        SearchAscii = false;
        ui->AsciiCheckbox->setCheckState(Qt::Unchecked);
    }
    if (setting.value("CaseSensitive") == "true") {
        CaseSensitive = true;
        ui->CaseCheckbox->setCheckState(Qt::Checked);
    } else if (setting.value("CaseSensitive") == "false") {
        CaseSensitive = false;
        ui->CaseCheckbox->setCheckState(Qt::Unchecked);
    }

    if (SearchAscii) {
        ui->EndianBox->setEnabled(false);
        ui->CaseCheckbox->setEnabled(true);
    } else {
        ui->EndianBox->setEnabled(true);
        ui->CaseCheckbox->setEnabled(false);
    }
}

QString SearchDialog::SearchedString = "";
QString SearchDialog::pSearchedString = "";

void SearchDialog::setParentWidget(QWidget *pWidget) {
    parentWidget = pWidget;
}

void SearchDialog::SetBinaryData(QByteArray *BinaryData) {
    BinaryBuffer = BinaryData;
}

bool SearchDialog::SearchBinary(int *begin, int *length) {
    if (SearchedString.size() == 0)
        return false;
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
    if (isLittleEndian)
        std::reverse(searchNum.begin(), searchNum.end());

    // Search little endian binary
    INT64 matchIdx = PreviousOffset;
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
    QMessageBox::about(this, tr("Search"), "Not Found!");
    return false;
}

bool SearchDialog::SearchBinaryAscii(int *begin, int *length) {
    if (SearchedString.size() == 0)
        return false;
    for (INT64 idx = PreviousOffset + 1; idx < BinaryBuffer->size(); ++idx) {
        bool Found = true;
        bool wFound = true;
        if (UpperToLower(BinaryBuffer->at(idx)) == SearchedString.at(0).toLatin1()) {
            for (INT64 strIdx = 1; strIdx < SearchedString.size(); ++strIdx) {
                if (UpperToLower(BinaryBuffer->at(idx + strIdx)) != SearchedString.at(strIdx).toLatin1()) {
                    Found = false;
                    break;
                }
            }
            for (INT64 strIdx = 1; strIdx < SearchedString.size(); ++strIdx) {
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
    QMessageBox::about(this, tr("Search"), "Not Found!");
    return false;
}

char SearchDialog::UpperToLower(char s) {
    if(!CaseSensitive && s >= 65 && s <= 90)
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
    emit closeSignal(false);
}

void SearchDialog::AsciiCheckboxStateChanged(int state) {
    PreviousOffset = -1;
    HistoryResult.clear();
    if (state == Qt::Checked) {
        SearchAscii = true;
        ui->EndianBox->setEnabled(false);
        ui->CaseCheckbox->setEnabled(true);
        setting.setValue("SearchAscii", "true");
    }
    else if (state == Qt::Unchecked) {
        SearchAscii = false;
        ui->EndianBox->setEnabled(true);
        ui->CaseCheckbox->setEnabled(false);
        setting.setValue("SearchAscii", "false");
    }
}

void SearchDialog::SearchContentTextChanged(const QString &text) {
    PreviousOffset = -1;
    HistoryResult.clear();
    SearchedString = text;
    if (!CaseSensitive)
        SearchedString = text.toLower();
}

void SearchDialog::NextButtonClicked() {
    int beginOffset = 0;
    int searchLength = 0;
    if (SearchAscii) {
        if (SearchBinaryAscii(&beginOffset, &searchLength)) {
            HistoryResult.push_back(QPair<int, int>(beginOffset, searchLength));
            ((QHexView*)parentWidget)->showFromOffset(beginOffset, searchLength);
        }
    } else {
        if (SearchBinary(&beginOffset, &searchLength)) {
            HistoryResult.push_back(QPair<int, int>(beginOffset, searchLength));
            ((QHexView*)parentWidget)->showFromOffset(beginOffset, searchLength);
        }
    }
}

void SearchDialog::PreviousButtonClicked() {
    if (HistoryResult.size() <= 1)
        return;
    HistoryResult.pop_back();
    QPair<int, int> lastResult = HistoryResult.back();
    INT32 beginOffset = lastResult.first;
    INT32 searchLength = lastResult.second;
    PreviousOffset = (INT64)beginOffset;
    ((QHexView*)parentWidget)->showFromOffset(beginOffset, searchLength);
}

void SearchDialog::SearchContentReturnPressed() {
    NextButtonClicked();
}

void SearchDialog::EndianBoxActivated(int index) {
    PreviousOffset = -1;
    HistoryResult.clear();
    if (ui->EndianBox->currentIndex() == EndianMode::LittleEndian) {
        isLittleEndian = true;
        setting.setValue("Endian", "little");
    } else if (ui->EndianBox->currentIndex() == EndianMode::BigEndian) {
        isLittleEndian = false;
        setting.setValue("Endian", "big");
    }
}

void SearchDialog::CaseCheckboxStateChanged(int state) {
    PreviousOffset = -1;
    HistoryResult.clear();
    if (state == Qt::Checked) {
        CaseSensitive = true;
        setting.setValue("CaseSensitive", "true");
    }
    else if (state == Qt::Unchecked) {
        CaseSensitive = false;
        setting.setValue("CaseSensitive", "false");
    }
}

