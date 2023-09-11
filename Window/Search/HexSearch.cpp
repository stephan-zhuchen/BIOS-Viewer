#include <QDebug>
#include <QRegularExpression>
#include <QMessageBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <algorithm>
#include "HexSearch.h"
#include "HexView/HexView.h"
#include "ui_HexSearch.h"

HexSearch::HexSearch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HexSearch),
    SearchLine(new QLineEdit)
{
    ui->setupUi(this);
    initSetting();

    connect(ui->AsciiCheckbox,  SIGNAL(stateChanged(int)),    this, SLOT(AsciiCheckboxStateChanged(int)));
    connect(ui->CaseCheckbox,   SIGNAL(stateChanged(int)),    this, SLOT(CaseCheckboxStateChanged(int)));
    connect(ui->WideCheckbox,   SIGNAL(stateChanged(int)),    this, SLOT(WideCheckboxStateChanged(int)));
    connect(ui->NextButton,     SIGNAL(clicked()),            this, SLOT(NextButtonClicked()));
    connect(ui->PreviousButton, SIGNAL(clicked()),            this, SLOT(PreviousButtonClicked()));
    connect(ui->EndianBox,      SIGNAL(activated(int)),       this, SLOT(EndianBoxActivated(int)));
    connect(SearchLine,         SIGNAL(textChanged(QString)), this, SLOT(SearchContentTextChanged(QString)));
    connect(SearchLine,         SIGNAL(returnPressed()),      this, SLOT(SearchContentReturnPressed()));

    ui->SearchContentBox->setCompleter(nullptr);
    ui->SearchContentBox->setLineEdit(SearchLine);
    SearchLine->setAttribute(Qt::WA_InputMethodEnabled, false);
//    ui->SearchContentBox->lineEdit()->setText(SearchedString);
    ui->SearchContentBox->lineEdit()->selectAll();
    ui->SearchContentBox->setFocus();
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);

    restoreGeometry(setting.value("HexSearch/geometry").toByteArray());
}

HexSearch::~HexSearch() {
    delete ui;
}

void HexSearch::initSetting() {
    if (!setting.contains("Endian"))
        setting.setValue("Endian", "little");
    if (!setting.contains("SearchAscii"))
        setting.setValue("SearchAscii", "false");
    if (!setting.contains("CaseSensitive"))
        setting.setValue("CaseSensitive", "false");
    if (!setting.contains("WideChar"))
        setting.setValue("WideChar", "true");

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
    if (setting.value("WideChar") == "true") {
        WideCharacter = true;
        ui->WideCheckbox->setCheckState(Qt::Checked);
    } else if (setting.value("WideChar") == "false") {
        WideCharacter = false;
        ui->WideCheckbox->setCheckState(Qt::Unchecked);
    }

    if (setting.contains("SearchBinaryHistory")) {
        setting.value("SearchBinaryHistory").toStringList().swap(searchHistory);
        ui->SearchContentBox->addItems(searchHistory);
    }

    if (SearchAscii) {
        ui->EndianBox->setEnabled(false);
        ui->CaseCheckbox->setEnabled(true);
        ui->WideCheckbox->setEnabled(true);
    } else {
        ui->EndianBox->setEnabled(true);
        ui->CaseCheckbox->setEnabled(false);
        ui->WideCheckbox->setEnabled(false);
    }
}

QString HexSearch::SearchedString = "";

void HexSearch::setParentWidget(QWidget *pWidget) {
    parentWidget = pWidget;
}

void HexSearch::SetBinaryData(QByteArray *BinaryData) {
    BinaryBuffer = BinaryData;
}

/**
 * @class HexSearch
 * @brief A class that represents a search dialog for binary search.
 */

void HexSearch::SearchBinary() {
    static QRegularExpression re("\\s");
    QString SearchContent;
    if (!CaseSensitive)
        SearchContent = SearchedString.toLower();
    else
        SearchContent = SearchedString;

    QString number = SearchContent.remove(re);
    if (number.size() % 2 == 1) {
        number = "0" + number;
    }
    QStringList littleEndianNum;
    QByteArray searchNum;
    for (int idx = 0; idx < number.size(); idx += 2) {
        littleEndianNum.append(number.mid(idx, 2));
    }
    for (const auto & var : littleEndianNum) {
        searchNum.append(var.toInt(nullptr, 16));
    }
    if (isLittleEndian)
        std::reverse(searchNum.begin(), searchNum.end());

    CurrentSearchLength = searchNum.size();

    INT64 searchIndex = 0;
    while ((searchIndex = BinaryBuffer->indexOf(searchNum, searchIndex)) != -1) {
        matchedSearchIndexes.append(searchIndex);
        searchIndex += searchNum.size();
    }

    return;
}

/**
 * @class HexSearch
 * @brief HexSearch class for performing binary search on ASCII values
 *
 * This class provides functionality to perform binary search on the ASCII values of an integer array.
 * It takes a pointer to the beginning of the array and the length of the array as input, and provides
 * methods to search for specific ASCII values within the array using the binary search algorithm.
 */

void HexSearch::SearchBinaryAscii() {
    QString SearchContent;
    if (!CaseSensitive)
        SearchContent = SearchedString.toLower();
    else
        SearchContent = SearchedString;
    for (INT64 idx = 0; idx < BinaryBuffer->size(); ++idx) {
        bool Found = true;
        bool wFound = WideCharacter;
        if (UpperToLower(BinaryBuffer->at(idx)) == SearchContent.at(0).toLatin1()) {
            for (INT64 strIdx = 1; strIdx < SearchContent.size(); ++strIdx) {
                if (UpperToLower(BinaryBuffer->at(idx + strIdx)) != SearchContent.at(strIdx).toLatin1()) {
                    Found = false;
                    break;
                }
            }
            if (WideCharacter) {
                for (INT64 strIdx = 1; strIdx < SearchContent.size(); ++strIdx) {
                    if (BinaryBuffer->at(idx + strIdx * 2 - 1) != 0 || UpperToLower(BinaryBuffer->at(idx + strIdx * 2)) != SearchContent.at(strIdx).toLatin1()) {
                        wFound = false;
                        break;
                    }
                }
            }

            if (Found || wFound) {
                matchedSearchIndexes.append(idx);
                continue;
            }
        }
    }

    return;
}

char HexSearch::UpperToLower(char s) const {
    if(!CaseSensitive && s >= 65 && s <= 90)
        s = s + 32;
    return s;
}

bool HexSearch::containsNonHexCharacters(const QString& str) {
    static QRegularExpression regex("[^0-9a-fA-F\\s]");
    QRegularExpressionMatch match = regex.match(str);
    return match.hasMatch();
}

void HexSearch::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        this->close();
    }
}

void HexSearch::closeEvent(QCloseEvent *event) {
    setting.setValue("HexSearch/geometry", saveGeometry());
    setting.setValue("SearchBinaryHistory", searchHistory);
    emit closeSignal(false);
}

void HexSearch::AsciiCheckboxStateChanged(int state) {
    CurrentIndex = -1;
    matchedSearchIndexes.clear();
    if (state == Qt::Checked) {
        SearchAscii = true;
        ui->EndianBox->setEnabled(false);
        ui->CaseCheckbox->setEnabled(true);
        ui->WideCheckbox->setEnabled(true);
        setting.setValue("SearchAscii", "true");
    }
    else if (state == Qt::Unchecked) {
        SearchAscii = false;
        ui->EndianBox->setEnabled(true);
        ui->CaseCheckbox->setEnabled(false);
        ui->WideCheckbox->setEnabled(false);
        setting.setValue("SearchAscii", "false");
    }
}

void HexSearch::SearchContentTextChanged(const QString &text) {
    CurrentIndex = -1;
    matchedSearchIndexes.clear();
    SearchedString = text;
    CurrentSearchLength = SearchedString.size();
}

void HexSearch::NextButtonClicked() {
    if (SearchedString.isEmpty())
        return;

    INT64 Index = CurrentIndex;
    searchHistory.removeAll(SearchedString);
    searchHistory.prepend(SearchedString);
    if (searchHistory.size() > 10)
        searchHistory.removeLast();
    ui->SearchContentBox->clear();
    ui->SearchContentBox->addItems(searchHistory);
    CurrentIndex = Index;

    if (matchedSearchIndexes.empty()) {
        if (SearchAscii) {
            SearchBinaryAscii();
        } else {
            if (containsNonHexCharacters(SearchedString)) {
                QMessageBox::about(this, tr("Search"), "Please type Hex number!");
                return;
            }
            SearchBinary();
        }
    }

    // Still Empty
    if (matchedSearchIndexes.empty()) {
        QMessageBox::about(this, tr("Search"), "Not Found!");
        return;
    }
    CurrentIndex = (CurrentIndex + 1) % matchedSearchIndexes.size();
    ((QHexView*)parentWidget)->showFromOffset(matchedSearchIndexes.at(CurrentIndex), CurrentSearchLength);
}

void HexSearch::PreviousButtonClicked() {
    if (matchedSearchIndexes.empty()) {
        QMessageBox::about(this, tr("Search"), "No Previous Results!");
        return;
    }

    CurrentIndex = (CurrentIndex - 1 + matchedSearchIndexes.size()) % matchedSearchIndexes.size();
    ((QHexView*)parentWidget)->showFromOffset(matchedSearchIndexes.at(CurrentIndex), CurrentSearchLength);
}

void HexSearch::SearchContentReturnPressed() {
    NextButtonClicked();
}

void HexSearch::EndianBoxActivated(int index) {
    CurrentIndex = -1;
    matchedSearchIndexes.clear();
    if (ui->EndianBox->currentIndex() == EndianMode::LittleEndian) {
        isLittleEndian = true;
        setting.setValue("Endian", "little");
    } else if (ui->EndianBox->currentIndex() == EndianMode::BigEndian) {
        isLittleEndian = false;
        setting.setValue("Endian", "big");
    }
}

void HexSearch::CaseCheckboxStateChanged(int state) {
    CurrentIndex = -1;
    matchedSearchIndexes.clear();
    if (state == Qt::Checked) {
        CaseSensitive = true;
        setting.setValue("CaseSensitive", "true");
    }
    else if (state == Qt::Unchecked) {
        CaseSensitive = false;
        setting.setValue("CaseSensitive", "false");
    }
}

void HexSearch::WideCheckboxStateChanged(int state) {
    CurrentIndex = -1;
    matchedSearchIndexes.clear();
    if (state == Qt::Checked) {
        WideCharacter = true;
        setting.setValue("WideChar", "true");
    }
    else if (state == Qt::Unchecked) {
        WideCharacter = false;
        setting.setValue("WideChar", "false");
    }
}
