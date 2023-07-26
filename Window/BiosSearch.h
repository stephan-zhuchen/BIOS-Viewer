#ifndef BIOSSEARCH_H
#define BIOSSEARCH_H

#include <QWidget>
#include <QSettings>
#include "Model.h"

using UefiSpace::VolumeType;

namespace Ui {
class BiosSearch;
}

class BiosSearch : public QWidget
{
    Q_OBJECT

public:
    explicit BiosSearch(QWidget *parent = nullptr);
    ~BiosSearch();

    void initSetting();
    void SetModelData(vector<DataModel*> *fvModel);
    void setTextList(vector<DataModel*> *itemsModel, const vector<int>& position);
    void SearchFileText(bool recursive);
    void SearchFileGuid(bool recursive);
    void paintWidget();
    void CollectGuidData();
    void setDarkMode(bool mode);

signals:
    void Highlight(vector<INT32>);
    void closeSignal(bool State);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void FvCheckboxStateChanged(int state);
    void FfsCheckboxStateChanged(int state);
    void SectionCheckboxStateChanged(int state);
    void CaseCheckboxStateChanged(int state);
    void RecursiveCheckboxStateChanged(int state);
    void NextButtonClicked();
    void PreviousButtonClicked();
    void SearchContentTextChanged(const QString &txt);
    void SearchContentReturnPressed();
    void tabWidgetCurrentChanged(int index);
    void SetGuidDataFromClipboard(const QString &ClipContent);
    void ClearButtonClicked();
    void GuidFvCheckboxStateChanged(int state);
    void GuidFfsCheckboxStateChanged(int state);
    void GuidSectionCheckboxStateChanged(int state);
    void GuidRecursiveCheckboxStateChanged(int state);

private:
    Ui::BiosSearch *ui;
    QSettings setting{"Intel", "BiosViewer"};
    bool darkMode{false};

    enum  SearchMode{Text=0, Guid};
    bool  SearchFv{true};
    bool  SearchFFS{true};
    bool  SearchSection{true};
    bool  CaseSensitive{false};
    bool  RecursiveSearch{false};
    bool  GuidSearchFv{true};
    bool  GuidSearchFFS{true};
    bool  GuidSearchSection{true};
    bool  GuidRecursiveSearch{false};

    vector<DataModel*> *SearchModelData{};
    static QString   SearchedString;
    static QString   pSearchedString;
    static EFI_GUID  SearchedGuid;
    static EFI_GUID  pSearchedGuid;
    QVector<QString> SearchTextList;
    QVector<VolumeType> SearchTextType;
    QVector<EFI_GUID>   SearchGuidList;
    vector<vector<int>>       SearchPositionList;
    vector<int>      PreviousItems;
};

#endif // BIOSSEARCH_H
