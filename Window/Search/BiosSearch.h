#ifndef BIOSSEARCH_H
#define BIOSSEARCH_H

#include <QWidget>
#include <QSettings>
#include <QTreeWidget>
#include "Volume.h"

namespace Ui {
class BiosSearch;
}

class BiosSearch : public QWidget
{
    Q_OBJECT

public:
    explicit BiosSearch(QWidget *parent = nullptr);
    ~BiosSearch() override;

    void initSetting();
    void SetTreeData(QTreeWidget* tree);
    void SearchTextFromTreeWidget(const QString& searchString);
    void SearchGuidFromTreeWidget(const EFI_GUID& searchGuid);
    bool CompareGuid(const EFI_GUID &guid, const EFI_GUID &targetGuid);
    void paintWidget();
    void CollectGuidData();
    void setDarkMode(bool mode);
    void ClearHighlightedItems();

signals:
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

private:
    Ui::BiosSearch *ui;
    QTreeWidget*   treeWidget;
    QList<QTreeWidgetItem *> matchedItems;
    QSettings setting{"Intel", "BiosViewer"};

    QColor HightedColor{"yellow"};
    QColor ItemColor{"white"};

    bool  darkMode{false};
    enum  SearchMode{Text=0, Guid};
    bool  SearchFv{true};
    bool  SearchFFS{true};
    bool  SearchSection{true};
    bool  CaseSensitive{false};
    bool  GuidSearchFv{true};
    bool  GuidSearchFFS{true};
    bool  GuidSearchSection{true};

    static QString   SearchedString;
    static EFI_GUID  SearchedGuid;
    QStringList      searchHistory;


    INT32 currentHighlightedIndex = -1;
};

#endif // BIOSSEARCH_H
