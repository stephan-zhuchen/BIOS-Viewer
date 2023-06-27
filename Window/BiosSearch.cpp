#include <QMessageBox>
#include <QKeyEvent>
#include <QPainter>
#include <QClipboard>
#include <QRegularExpressionValidator>
#include "BiosSearch.h"
#include "ui_BiosSearch.h"

BiosSearch::BiosSearch(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BiosSearch)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    initSetting();
    ui->SearchContent->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->guidTab->installEventFilter(this);

    connect(ui->SearchContent,      SIGNAL(textChanged(QString)), this, SLOT(SearchContentTextChanged(QString)));
    connect(ui->FvCheckbox,         SIGNAL(stateChanged(int)),    this, SLOT(FvCheckboxStateChanged(int)));
    connect(ui->FfsCheckbox,        SIGNAL(stateChanged(int)),    this, SLOT(FfsCheckboxStateChanged(int)));
    connect(ui->SectionCheckbox,    SIGNAL(stateChanged(int)),    this, SLOT(SectionCheckboxStateChanged(int)));
    connect(ui->CaseCheckbox,       SIGNAL(stateChanged(int)),    this, SLOT(CaseCheckboxStateChanged(int)));
    connect(ui->RecursiveCheckbox,  SIGNAL(stateChanged(int)),    this, SLOT(RecursiveCheckboxStateChanged(int)));
    connect(ui->PreviousButton,     SIGNAL(clicked()),            this, SLOT(PreviousButtonClicked()));
    connect(ui->NextButton,         SIGNAL(clicked()),            this, SLOT(NextButtonClicked()));
    connect(ui->SearchContent,      SIGNAL(returnPressed()),      this, SLOT(SearchContentReturnPressed()));
    connect(ui->tabWidget,          SIGNAL(currentChanged(int)),  this, SLOT(tabWidgetCurrentChanged(int)));
    connect(ui->clearButton,        SIGNAL(clicked()),            this, SLOT(ClearButtonClicked()));
    connect(ui->Data1Edit,          SIGNAL(GuidCopied(const QString&)), this, SLOT(SetGuidDataFromClipboard(const QString&)));
    connect(ui->GuidFvCheckbox,     SIGNAL(stateChanged(int)),    this, SLOT(GuidFvCheckboxStateChanged(int)));
    connect(ui->GuidFfsCheckbox,    SIGNAL(stateChanged(int)),    this, SLOT(GuidFfsCheckboxStateChanged(int)));
    connect(ui->GuidSectionCheckbox, SIGNAL(stateChanged(int)),   this, SLOT(GuidSectionCheckboxStateChanged(int)));
    connect(ui->GuidRecursiveCheckbox, SIGNAL(stateChanged(int)), this, SLOT(GuidRecursiveCheckboxStateChanged(int)));

    tabWidgetCurrentChanged(0);
}

BiosSearch::~BiosSearch()
{
    PreviousItems.clear();
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
    if (setting.value("RecursiveSearch") == "true") {
        RecursiveSearch = true;
        ui->RecursiveCheckbox->setCheckState(Qt::Checked);
    } else if (setting.value("RecursiveSearch") == "false") {
        RecursiveSearch = false;
        ui->RecursiveCheckbox->setCheckState(Qt::Unchecked);
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
    if (setting.value("GuidRecursiveSearch") == "true") {
        GuidRecursiveSearch = true;
        ui->GuidRecursiveCheckbox->setCheckState(Qt::Checked);
    } else if (setting.value("GuidRecursiveSearch") == "false") {
        GuidRecursiveSearch = false;
        ui->GuidRecursiveCheckbox->setCheckState(Qt::Unchecked);
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

void BiosSearch::FvCheckboxStateChanged(int state)
{
    SearchFv = state;
    PreviousItems.clear();
    if (SearchFv)
        setting.setValue("SearchFv", "true");
    else
        setting.setValue("SearchFv", "false");
}


void BiosSearch::FfsCheckboxStateChanged(int state)
{
    SearchFFS = state;
    PreviousItems.clear();
    if (SearchFFS)
        setting.setValue("SearchFFS", "true");
    else
        setting.setValue("SearchFFS", "false");
}

void BiosSearch::SectionCheckboxStateChanged(int state)
{
    SearchSection = state;
    PreviousItems.clear();
    if (SearchSection)
        setting.setValue("SearchSection", "true");
    else
        setting.setValue("SearchSection", "false");
}

void BiosSearch::CaseCheckboxStateChanged(int state)
{
    CaseSensitive = state;
    PreviousItems.clear();
    if (CaseSensitive)
        setting.setValue("CaseSensitive", "true");
    else
        setting.setValue("CaseSensitive", "false");
}

void BiosSearch::RecursiveCheckboxStateChanged(int state)
{
    RecursiveSearch = state;
    PreviousItems.clear();
    if (RecursiveSearch)
        setting.setValue("RecursiveSearch", "true");
    else
        setting.setValue("RecursiveSearch", "false");
}

void BiosSearch::GuidFvCheckboxStateChanged(int state) {
    GuidSearchFv = state;
    PreviousItems.clear();
    if (GuidSearchFv)
        setting.setValue("GuidSearchFv", "true");
    else
        setting.setValue("GuidSearchFv", "false");
}


void BiosSearch::GuidFfsCheckboxStateChanged(int state) {
    GuidSearchFFS = state;
    PreviousItems.clear();
    if (GuidSearchFFS)
        setting.setValue("GuidSearchFFS", "true");
    else
        setting.setValue("GuidSearchFFS", "false");
}


void BiosSearch::GuidSectionCheckboxStateChanged(int state) {
    GuidSearchSection = state;
    PreviousItems.clear();
    if (GuidSearchSection)
        setting.setValue("GuidSearchSection", "true");
    else
        setting.setValue("GuidSearchSection", "false");
}

void BiosSearch::GuidRecursiveCheckboxStateChanged(int state) {
    GuidRecursiveSearch = state;
    PreviousItems.clear();
    if (GuidRecursiveSearch)
        setting.setValue("GuidRecursiveSearch", "true");
    else
        setting.setValue("GuidRecursiveSearch", "false");
}

QString BiosSearch::SearchedString = "";
QString BiosSearch::pSearchedString = "";
EFI_GUID BiosSearch::SearchedGuid = EFI_GUID {0};
EFI_GUID BiosSearch::pSearchedGuid = EFI_GUID {0};

void BiosSearch::SetModelData(vector<DataModel*> *fvModel) {
    SearchModelData = fvModel;
    vector<int> pos;
    setTextList(SearchModelData, pos);

    for (int var = 0; var < SearchPositionList.size(); ++var) {
        SearchPositionList.at(var).at(0) += 1;
    }
}

void BiosSearch::setTextList(vector<DataModel*> *itemsModel, vector<int> position) {
    for (int i = 0; i < itemsModel->size(); ++i) {
        vector<int> parentPos = position;
        DataModel* item = itemsModel->at(i);
        parentPos.push_back(i);
        QString name = item->getName();
        SearchTextList.push_back(name);
        SearchPositionList.push_back(parentPos);
        SearchTextType.push_back(item->modelData->Type);
        SearchGuidList.push_back(item->modelData->getVolumeGuid());
        setTextList(&(item->volumeModelData), parentPos);
    }
}

void BiosSearch::SearchFileText(bool recursive) {
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
        if (!this->SearchFv && SearchTextType.at(row) == VolumeType::FirmwareVolume) {
            row += 1;
            continue;
        }
        if (!this->SearchFFS && SearchTextType.at(row) == VolumeType::FfsFile) {
            row += 1;
            continue;
        }
        if (!this->SearchSection && SearchTextType.at(row) == VolumeType::CommonSection) {
            row += 1;
            continue;
        }
        if (SearchTextList.at(row).contains(SearchedString, this->CaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)) {
            break;
        }
        row += 1;
    }

    if (row == SearchTextList.size()) {
        if (recursive) {
            PreviousItems.clear();
            SearchFileText(false);
        } else {
            QMessageBox::about(this, tr("Search"), "Not Found!");
        }
        return;
    }
    PreviousItems.push_back(row);
    emit Highlight(SearchPositionList.at(row));
}

void BiosSearch::SearchFileGuid(bool recursive) {
    int PreviousRow = -1;
    if (SearchedGuid != pSearchedGuid) {
        PreviousItems.clear();
    }
    if (PreviousItems.size() != 0) {
        PreviousRow = PreviousItems.at(PreviousItems.size() - 1);
    }
    if (SearchModelData->size() == 0)
        return;

    int row = PreviousRow + 1;
    enum CompareResult {Mismatch, Pass, Match};
    while (row < SearchGuidList.size()) {
        if (!this->GuidSearchFv && SearchTextType.at(row) == VolumeType::FirmwareVolume) {
            row += 1;
            continue;
        }
        if (!this->GuidSearchFFS && SearchTextType.at(row) == VolumeType::FfsFile) {
            row += 1;
            continue;
        }
        if (!this->GuidSearchSection && SearchTextType.at(row) == VolumeType::CommonSection) {
            row += 1;
            continue;
        }
        EFI_GUID guid = SearchGuidList.at(row);
        QVector<CompareResult> CompareResultList;
        if (SearchedGuid.Data1 == 0) {
            CompareResultList.push_back(CompareResult::Pass);
        } else if (SearchedGuid.Data1 == guid.Data1) {
            CompareResultList.push_back(CompareResult::Match);
        } else {
            CompareResultList.push_back(CompareResult::Mismatch);
        }

        if (SearchedGuid.Data2 == 0) {
            CompareResultList.push_back(CompareResult::Pass);
        } else if (SearchedGuid.Data2 == guid.Data2) {
            CompareResultList.push_back(CompareResult::Match);
        } else {
            CompareResultList.push_back(CompareResult::Mismatch);
        }

        if (SearchedGuid.Data3 == 0) {
            CompareResultList.push_back(CompareResult::Pass);
        } else if (SearchedGuid.Data3 == guid.Data3) {
            CompareResultList.push_back(CompareResult::Match);
        } else {
            CompareResultList.push_back(CompareResult::Mismatch);
        }

        for (int idx = 0; idx < 8; ++idx) {
            if (SearchedGuid.Data4[idx] == 0) {
                CompareResultList.push_back(CompareResult::Pass);
            } else if (SearchedGuid.Data4[idx] == guid.Data4[idx]) {
                CompareResultList.push_back(CompareResult::Match);
            } else {
                CompareResultList.push_back(CompareResult::Mismatch);
            }
        }

        if (CompareResultList.contains(CompareResult::Mismatch)) {
            row += 1;
            continue;
        } else if (CompareResultList.contains(CompareResult::Match)) {
            break;
        }
        row += 1;
    }

    if (row == SearchGuidList.size()) {
        if (recursive) {
            PreviousItems.clear();
            SearchFileGuid(false);
        } else {
            QMessageBox::about(this, tr("Search"), "Not Found!");
        }
        return;
    }

    PreviousItems.push_back(row);
    emit Highlight(SearchPositionList.at(row));
}

void BiosSearch::NextButtonClicked() {
    if (ui->tabWidget->currentIndex() == SearchMode::Text) {
        SearchFileText(RecursiveSearch);
        pSearchedString = SearchedString;
    } else if (ui->tabWidget->currentIndex() == SearchMode::Guid) {
        CollectGuidData();
        SearchFileGuid(GuidRecursiveSearch);
        pSearchedGuid = SearchedGuid;
    }
}

void BiosSearch::PreviousButtonClicked() {
    if (ui->tabWidget->currentIndex() == SearchMode::Text && SearchedString != pSearchedString) {
        PreviousItems.clear();
    } else if (ui->tabWidget->currentIndex() == SearchMode::Guid && SearchedGuid != pSearchedGuid) {
        PreviousItems.clear();
    }

    if (PreviousItems.size() > 1) {
        int PreviousRow = PreviousItems.at(PreviousItems.size() - 2);
        PreviousItems.pop_back();
        emit Highlight(SearchPositionList.at(PreviousRow));
    }
}

void BiosSearch::SearchContentTextChanged(const QString &txt) {
    SearchedString = txt;
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
    INT32 LineHeight = 50;
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
    PreviousItems.clear();
    if (ui->tabWidget->currentIndex() == SearchMode::Text) {
        ui->SearchContent->setText(SearchedString);
        ui->SearchContent->selectAll();
        ui->SearchContent->setFocus();
    } else if (ui->tabWidget->currentIndex() == SearchMode::Guid) {
        ui->Data1Edit->setFocus();
    }
}

void BiosSearch::SetGuidDataFromClipboard(const QString &ClipContent) {
    static QRegularExpression EmptySpace("\\s");
    static QRegularExpression GuidPattern1("^0[xX]([0-9a-fA-F]){8}$");
    static QRegularExpression GuidPattern2("^([0-9a-fA-F]){8}$");
    static QRegularExpression GuidPattern3("^[A-F0-9]{8}(-[A-F0-9]{4}){3}-[A-F0-9]{12}$");
    static QRegularExpression GuidPattern4("^\\{0[xX][0-9a-fA-F]{8},(0[xX][0-9a-fA-F]{4},){2}\\{(0[xX][0-9a-fA-F]{2},){7}0[xX][0-9a-fA-F]{2}\\}\\}");

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
    }
}

void BiosSearch::setDarkMode(bool mode) {
    darkMode = mode;
}

void BiosSearch::ClearButtonClicked() {
    PreviousItems.clear();
    ui->Data1Edit->clear();
    ui->Data2Edit->clear();
    ui->Data3Edit->clear();
    ui->Data4Edit->clear();
    ui->Data5Edit->clear();
}
