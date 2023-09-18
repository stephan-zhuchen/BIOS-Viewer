#include <QMessageBox>
#include <QKeyEvent>
#include <QPainter>
#include <QClipboard>
#include <QRegularExpressionValidator>
#include "BiosSearch.h"
#include "BiosView/BiosWindow.h"
#include <iostream>
#include "ui_BiosSearch.h"

BiosSearch::BiosSearch(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BiosSearch)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
    initSetting();
    ui->SearchContentBox->setCompleter(nullptr);
    ui->SearchContentBox->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->SearchContentBox->lineEdit()->setInputMethodHints(Qt::ImhPreferLatin);
    ui->SearchContentBox->lineEdit()->setClearButtonEnabled(true);
    ui->guidTab->installEventFilter(this);

    restoreGeometry(setting.value("BiosSearchDialog/geometry").toByteArray());

    connect(ui->FvCheckbox,         SIGNAL(stateChanged(int)),    this, SLOT(FvCheckboxStateChanged(int)));
    connect(ui->FfsCheckbox,        SIGNAL(stateChanged(int)),    this, SLOT(FfsCheckboxStateChanged(int)));
    connect(ui->SectionCheckbox,    SIGNAL(stateChanged(int)),    this, SLOT(SectionCheckboxStateChanged(int)));
    connect(ui->CaseCheckbox,       SIGNAL(stateChanged(int)),    this, SLOT(CaseCheckboxStateChanged(int)));
    connect(ui->PreviousButton,     SIGNAL(clicked()),            this, SLOT(PreviousButtonClicked()));
    connect(ui->NextButton,         SIGNAL(clicked()),            this, SLOT(NextButtonClicked()));
    connect(ui->tabWidget,          SIGNAL(currentChanged(int)),  this, SLOT(tabWidgetCurrentChanged(int)));
    connect(ui->clearButton,        SIGNAL(clicked()),            this, SLOT(ClearButtonClicked()));
    connect(ui->Data1Edit,          SIGNAL(GuidCopied(const QString&)), this, SLOT(SetGuidDataFromClipboard(const QString&)));
    connect(ui->GuidFvCheckbox,     SIGNAL(stateChanged(int)),    this, SLOT(GuidFvCheckboxStateChanged(int)));
    connect(ui->GuidFfsCheckbox,    SIGNAL(stateChanged(int)),    this, SLOT(GuidFfsCheckboxStateChanged(int)));
    connect(ui->GuidSectionCheckbox, SIGNAL(stateChanged(int)),   this, SLOT(GuidSectionCheckboxStateChanged(int)));
    connect(ui->SearchContentBox->lineEdit(),   SIGNAL(returnPressed()),      this, SLOT(SearchContentReturnPressed()));
    connect(ui->SearchContentBox->lineEdit(),   SIGNAL(textChanged(QString)), this, SLOT(SearchContentTextChanged(QString)));

    tabWidgetCurrentChanged(0);
}

BiosSearch::~BiosSearch() {
    delete ui;
}

void BiosSearch::initSetting() {
    if (!setting.contains("SearchFv"))
        setting.setValue("SearchFv", "true");
    if (!setting.contains("SearchFFS"))
        setting.setValue("SearchFFS", "true");
    if (!setting.contains("SearchSection"))
        setting.setValue("SearchSection", "true");
    if (!setting.contains("CaseSensitive"))
        setting.setValue("CaseSensitive", "false");
    if (!setting.contains("RecursiveSearch"))
        setting.setValue("RecursiveSearch", "false");

    if (!setting.contains("GuidSearchFv"))
        setting.setValue("GuidSearchFv", "true");
    if (!setting.contains("GuidSearchFFS"))
        setting.setValue("GuidSearchFFS", "true");
    if (!setting.contains("GuidSearchSection"))
        setting.setValue("GuidSearchSection", "true");
    if (!setting.contains("GuidRecursiveSearch"))
        setting.setValue("GuidRecursiveSearch", "false");

    if (setting.value("SearchFv") == "true") {
        SearchFv = true;
        ui->FvCheckbox->setCheckState(Qt::Checked);
    } else if (setting.value("SearchFv") == "false") {
        SearchFv = false;
        ui->FvCheckbox->setCheckState(Qt::Unchecked);
    }
    if (setting.value("SearchFFS") == "true") {
        SearchFFS = true;
        ui->FfsCheckbox->setCheckState(Qt::Checked);
    } else if (setting.value("SearchFFS") == "false") {
        SearchFFS = false;
        ui->FfsCheckbox->setCheckState(Qt::Unchecked);
    }
    if (setting.value("SearchSection") == "true") {
        SearchSection = true;
        ui->SectionCheckbox->setCheckState(Qt::Checked);
    } else if (setting.value("SearchSection") == "false") {
        SearchSection = false;
        ui->SectionCheckbox->setCheckState(Qt::Unchecked);
    }
    if (setting.value("CaseSensitive") == "true") {
        CaseSensitive = true;
        ui->CaseCheckbox->setCheckState(Qt::Checked);
    } else if (setting.value("CaseSensitive") == "false") {
        CaseSensitive = false;
        ui->CaseCheckbox->setCheckState(Qt::Unchecked);
    }

    if (setting.value("GuidSearchFv") == "true") {
        GuidSearchFv = true;
        ui->GuidFvCheckbox->setCheckState(Qt::Checked);
    } else if (setting.value("GuidSearchFv") == "false") {
        GuidSearchFv = false;
        ui->GuidFvCheckbox->setCheckState(Qt::Unchecked);
    }
    if (setting.value("GuidSearchFFS") == "true") {
        GuidSearchFFS = true;
        ui->GuidFfsCheckbox->setCheckState(Qt::Checked);
    } else if (setting.value("GuidSearchFFS") == "false") {
        GuidSearchFFS = false;
        ui->GuidFfsCheckbox->setCheckState(Qt::Unchecked);
    }
    if (setting.value("GuidSearchSection") == "true") {
        GuidSearchSection = true;
        ui->GuidSectionCheckbox->setCheckState(Qt::Checked);
    } else if (setting.value("GuidSearchSection") == "false") {
        GuidSearchSection = false;
        ui->GuidSectionCheckbox->setCheckState(Qt::Unchecked);
    }

    if (setting.contains("SearchTextHistory")) {
        setting.value("SearchTextHistory").toStringList().swap(searchHistory);
        ui->SearchContentBox->addItems(searchHistory);
    }

    QRegularExpressionValidator GuidData1Validator = QRegularExpressionValidator(QRegularExpression("([0-9a-fA-F]){8}"));
    QRegularExpressionValidator GuidData2Validator = QRegularExpressionValidator(QRegularExpression("([0-9a-fA-F]){4}"));
    QRegularExpressionValidator GuidData5Validator = QRegularExpressionValidator(QRegularExpression("([0-9a-fA-F]){12}"));
//    ui->Data1Edit->setValidator(new QRegularExpressionValidator(QRegularExpression("([0-9a-fA-F]){8}"), ui->Data1Edit));
    ui->Data2Edit->setValidator(new QRegularExpressionValidator(QRegularExpression("([0-9a-fA-F]){4}"), ui->Data2Edit));
    ui->Data3Edit->setValidator(new QRegularExpressionValidator(QRegularExpression("([0-9a-fA-F]){4}"), ui->Data3Edit));
    ui->Data4Edit->setValidator(new QRegularExpressionValidator(QRegularExpression("([0-9a-fA-F]){4}"), ui->Data4Edit));
    ui->Data5Edit->setValidator(new QRegularExpressionValidator(QRegularExpression("([0-9a-fA-F]){12}"), ui->Data5Edit));

    QString TipMsg = "You can paste the GUID data here. Supported GUID format:\n"
                     "8C8CE578\n"
                     "0x8C8CE578\n"
                     "8C8CE578-8A3D-4F1C-9935-896185C32DD3\n"
                     "{0x8c8ce578, 0x3dcb, 0x4dca, {0xbd, 0x6f, 0x1e, 0x96, 0x89, 0xe7, 0x34, 0x9a}}";
    ui->Data1Edit->setToolTip(TipMsg);
    ui->Data2Edit->setToolTip(TipMsg);
    ui->Data3Edit->setToolTip(TipMsg);
    ui->Data4Edit->setToolTip(TipMsg);
    ui->Data5Edit->setToolTip(TipMsg);
}

void BiosSearch::FvCheckboxStateChanged(int state) {
    SearchFv = state;
    ClearHighlightedItems();
    if (SearchFv)
        setting.setValue("SearchFv", "true");
    else
        setting.setValue("SearchFv", "false");
}


void BiosSearch::FfsCheckboxStateChanged(int state) {
    SearchFFS = state;
    ClearHighlightedItems();
    if (SearchFFS)
        setting.setValue("SearchFFS", "true");
    else
        setting.setValue("SearchFFS", "false");
}

void BiosSearch::SectionCheckboxStateChanged(int state) {
    SearchSection = state;
    ClearHighlightedItems();
    if (SearchSection)
        setting.setValue("SearchSection", "true");
    else
        setting.setValue("SearchSection", "false");
}

void BiosSearch::CaseCheckboxStateChanged(int state) {
    CaseSensitive = state;
    ClearHighlightedItems();
    if (CaseSensitive)
        setting.setValue("CaseSensitive", "true");
    else
        setting.setValue("CaseSensitive", "false");
}

void BiosSearch::GuidFvCheckboxStateChanged(int state) {
    GuidSearchFv = state;
    ClearHighlightedItems();
    if (GuidSearchFv)
        setting.setValue("GuidSearchFv", "true");
    else
        setting.setValue("GuidSearchFv", "false");
}


void BiosSearch::GuidFfsCheckboxStateChanged(int state) {
    GuidSearchFFS = state;
    ClearHighlightedItems();
    if (GuidSearchFFS)
        setting.setValue("GuidSearchFFS", "true");
    else
        setting.setValue("GuidSearchFFS", "false");
}


void BiosSearch::GuidSectionCheckboxStateChanged(int state) {
    GuidSearchSection = state;
    ClearHighlightedItems();
    if (GuidSearchSection)
        setting.setValue("GuidSearchSection", "true");
    else
        setting.setValue("GuidSearchSection", "false");
}

QString BiosSearch::SearchedString = "";
EFI_GUID BiosSearch::SearchedGuid = EFI_GUID {0};

bool BiosSearch::CompareGuid(const EFI_GUID &guid, const EFI_GUID &targetGuid) {
    enum CompareResult {Mismatch, Pass, Match};

    QVector<CompareResult> CompareResultList;
    if (guid.Data1 == 0) {
        CompareResultList.push_back(CompareResult::Pass);
    } else if (guid.Data1 == targetGuid.Data1) {
        CompareResultList.push_back(CompareResult::Match);
    } else {
        CompareResultList.push_back(CompareResult::Mismatch);
    }

    if (guid.Data2 == 0) {
        CompareResultList.push_back(CompareResult::Pass);
    } else if (guid.Data2 == targetGuid.Data2) {
        CompareResultList.push_back(CompareResult::Match);
    } else {
        CompareResultList.push_back(CompareResult::Mismatch);
    }

    if (guid.Data3 == 0) {
        CompareResultList.push_back(CompareResult::Pass);
    } else if (guid.Data3 == targetGuid.Data3) {
        CompareResultList.push_back(CompareResult::Match);
    } else {
        CompareResultList.push_back(CompareResult::Mismatch);
    }

    for (int idx = 0; idx < 8; ++idx) {
        if (guid.Data4[idx] == 0) {
            CompareResultList.push_back(CompareResult::Pass);
        } else if (guid.Data4[idx] == targetGuid.Data4[idx]) {
            CompareResultList.push_back(CompareResult::Match);
        } else {
            CompareResultList.push_back(CompareResult::Mismatch);
        }
    }

    if (CompareResultList.contains(CompareResult::Mismatch)) {
        return false;
    } else if (CompareResultList.contains(CompareResult::Match)) {
        return true;
    }
    return false;
}

void BiosSearch::NextButtonClicked() {
    if (ui->tabWidget->currentIndex() == SearchMode::Text) {
        if (SearchedString.isEmpty())
            return;
        if (matchedItems.empty())
            SearchTextFromTreeWidget(SearchedString);
    } else if (ui->tabWidget->currentIndex() == SearchMode::Guid) {
        CollectGuidData();
        if (matchedItems.empty())
            SearchGuidFromTreeWidget(SearchedGuid);
    }

    // Still Empty
    if (matchedItems.empty()) {
        QMessageBox::about(this, tr("Search"), "Not Found!");
        return;
    }
    currentHighlightedIndex = (currentHighlightedIndex + 1) % matchedItems.size();
    treeWidget->scrollToItem(matchedItems[currentHighlightedIndex]);
    treeWidget->setCurrentItem(matchedItems[currentHighlightedIndex]);
}

void BiosSearch::PreviousButtonClicked() {
    if (matchedItems.empty()) {
        QMessageBox::about(this, tr("Search"), "No Previous Results!");
        return;
    }
    currentHighlightedIndex = (currentHighlightedIndex - 1 + matchedItems.size()) % matchedItems.size();
    treeWidget->scrollToItem(matchedItems[currentHighlightedIndex]);
    treeWidget->setCurrentItem(matchedItems[currentHighlightedIndex]);

}

void BiosSearch::SearchContentTextChanged(const QString &txt) {
    SearchedString = txt;
    ClearHighlightedItems();
}

void BiosSearch::SearchContentReturnPressed() {
    NextButtonClicked();
}

void BiosSearch::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        this->close();
    }
}

void BiosSearch::closeEvent(QCloseEvent *event) {
    ClearHighlightedItems();
    setting.setValue("BiosSearchDialog/geometry", saveGeometry());
    setting.setValue("SearchTextHistory", searchHistory);
    emit closeSignal(false);
}

bool BiosSearch::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->guidTab && event->type() == QEvent::Paint) {
        paintWidget();
    }

    return QWidget::eventFilter(watched, event);
}

void BiosSearch::paintWidget() {
    QPainter painter(ui->guidTab);
    INT32 LineHeight = 53;
    INT32 LineX = 86;
    INT32 LengthX = 8;
    INT32 Interval = 60;
    if (darkMode)
        painter.setPen(QPen(Qt::white, 1));
    else
        painter.setPen(QPen(Qt::black, 1));
    for (int idx = 0; idx < 4; ++idx) {
        painter.drawLine(LineX + Interval * idx, LineHeight, LineX + LengthX + Interval * idx, LineHeight);
    }
}

void BiosSearch::CollectGuidData() {
    SearchedGuid.Data1 = ui->Data1Edit->text().toUInt(nullptr, 16);
    SearchedGuid.Data2 = ui->Data2Edit->text().toUInt(nullptr, 16);
    SearchedGuid.Data3 = ui->Data3Edit->text().toUInt(nullptr, 16);
    SearchedGuid.Data4[0] = ui->Data4Edit->text().rightJustified(4, '0').mid(0, 2).toUInt(nullptr, 16);
    SearchedGuid.Data4[1] = ui->Data4Edit->text().rightJustified(4, '0').mid(2, 2).toUInt(nullptr, 16);
    for (int idx = 0; idx < 6; ++idx) {
        SearchedGuid.Data4[idx + 2] = ui->Data5Edit->text().rightJustified(12, '0').mid(idx * 2, 2).toUInt(nullptr, 16);
    }
}

void BiosSearch::tabWidgetCurrentChanged(int index) {
    ClearHighlightedItems();
    if (ui->tabWidget->currentIndex() == SearchMode::Text) {
        ui->SearchContentBox->lineEdit()->setText(SearchedString);
        ui->SearchContentBox->lineEdit()->selectAll();
        ui->SearchContentBox->setFocus();
    } else if (ui->tabWidget->currentIndex() == SearchMode::Guid) {
        ui->Data1Edit->setFocus();
    }
}

void BiosSearch::SetGuidDataFromClipboard(const QString &ClipContent) {
    static QRegularExpression EmptySpace("\\s");
    static QRegularExpression GuidPattern1("^0[xX]([0-9a-fA-F]){8}$");
    static QRegularExpression GuidPattern2("^([0-9a-fA-F]){8}$");
    static QRegularExpression GuidPattern3("^[0-9a-fA-F]{8}(-[0-9a-fA-F]{4}){3}-[0-9a-fA-F]{12}$");
    static QRegularExpression GuidPattern4(R"(^\{0[xX][0-9a-fA-F]{8},(0[xX][0-9a-fA-F]{4},){2}\{(0[xX][0-9a-fA-F]{2},){7}0[xX][0-9a-fA-F]{2}\}\})");

    QString guidStr = ClipContent;
    guidStr.remove(EmptySpace);

    QRegularExpressionMatch GuidPattern1Match = GuidPattern1.match(guidStr);
    QRegularExpressionMatch GuidPattern2Match = GuidPattern2.match(guidStr);
    QRegularExpressionMatch GuidPattern3Match = GuidPattern3.match(guidStr);
    QRegularExpressionMatch GuidPattern4Match = GuidPattern4.match(guidStr);

    if (GuidPattern1Match.hasMatch()) {
        ui->Data1Edit->setText(guidStr.mid(2));
    }
    else if (GuidPattern2Match.hasMatch()) {
        ui->Data1Edit->setText(guidStr);
    }
    else if (GuidPattern3Match.hasMatch()) {
        ui->Data1Edit->setText(guidStr.mid(0, 8));
        ui->Data2Edit->setText(guidStr.mid(9, 4));
        ui->Data3Edit->setText(guidStr.mid(14, 4));
        ui->Data4Edit->setText(guidStr.mid(19, 4));
        ui->Data5Edit->setText(guidStr.mid(24, 12));
    }
    else if (GuidPattern4Match.hasMatch()) {
        ui->Data1Edit->setText(guidStr.mid(3, 8));
        ui->Data2Edit->setText(guidStr.mid(14, 4));
        ui->Data3Edit->setText(guidStr.mid(21, 4));
        ui->Data4Edit->setText(guidStr.mid(29, 2) + guidStr.mid(34, 2));
        ui->Data5Edit->setText(guidStr.mid(39, 2) + guidStr.mid(44, 2) + guidStr.mid(49, 2) + guidStr.mid(54, 2) + guidStr.mid(59, 2) + guidStr.mid(64, 2));
    } else {
        QString TipMsg = "\nSupported GUID Pattern:\n"
                         "8C8CE578\n"
                         "0x8C8CE578\n"
                         "8C8CE578-8A3D-4F1C-9935-896185C32DD3\n"
                         "{0x8c8ce578, 0x3dcb, 0x4dca, {0xbd, 0x6f, 0x1e, 0x96, 0x89, 0xe7, 0x34, 0x9a}}";
        QMessageBox::about(this, tr("Search"), "\"" + ClipContent + "\" Does Match GUID Pattern!" + TipMsg);
    }
}

void BiosSearch::setDarkMode(bool mode) {
    darkMode = mode;
    if (darkMode) {
        ItemColor = QColor(36, 36, 36);
        HightedColor = QColor("green");
    } else {
        ItemColor = QColor("white");
        HightedColor = QColor("yellow");
    }
}

void BiosSearch::ClearButtonClicked() {
    ClearHighlightedItems();
    ui->Data1Edit->clear();
    ui->Data2Edit->clear();
    ui->Data3Edit->clear();
    ui->Data4Edit->clear();
    ui->Data5Edit->clear();
}

void BiosSearch::SetTreeData(QTreeWidget *tree) {
    treeWidget = tree;
}

void BiosSearch::SearchTextFromTreeWidget(const QString &searchString) {
    INT32 Index = currentHighlightedIndex;
    searchHistory.removeAll(searchString);
    searchHistory.prepend(searchString);
    if (searchHistory.size() > 10)
        searchHistory.removeLast();
    ui->SearchContentBox->clear();
    ui->SearchContentBox->addItems(searchHistory);
    currentHighlightedIndex = Index;

    QFlags<Qt::MatchFlag> flags = Qt::MatchContains | Qt::MatchRecursive;
    if (CaseSensitive)
        flags = flags | Qt::MatchCaseSensitive;
    ClearHighlightedItems();
    matchedItems = treeWidget->findItems(searchString, flags, treeColNum::Name);
    for (QTreeWidgetItem *item : matchedItems) {
        item->setBackground(Name, QBrush(HightedColor));
    }
}

void BiosSearch::SearchGuidFromTreeWidget(const EFI_GUID &searchGuid) {
    ClearHighlightedItems();
    QList<QTreeWidgetItem *> allItems = treeWidget->findItems(QString("*"), Qt::MatchWildcard|Qt::MatchRecursive);
    for (QTreeWidgetItem *item : allItems) {
        auto *itemVolume = item->data(treeColNum::Name, Qt::UserRole).value<Volume*>();
        if (CompareGuid(searchGuid, itemVolume->getVolumeGuid())) {
            matchedItems.append(item);
            item->setBackground(Name, QBrush(HightedColor));
        }
    }
}

void BiosSearch::ClearHighlightedItems() {
    for (QTreeWidgetItem *item : matchedItems) {
        item->setBackground(Name, QBrush(ItemColor));
    }
    matchedItems.clear();
    currentHighlightedIndex = -1;
}
