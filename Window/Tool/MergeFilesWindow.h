#ifndef MERGEFILESWINDOW_H
#define MERGEFILESWINDOW_H

#include <QMainWindow>
#include <QSettings>

namespace Ui {
class MergeFilesWindow;
}

class MergeFilesWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MergeFilesWindow(bool darkMode, QWidget *parent = nullptr);
    ~MergeFilesWindow();

    void OpenFile(const QString& path);

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Ui::MergeFilesWindow *ui;
    bool                 darkModeFlag;
    QSettings            setting{"Intel", "BiosViewer"};
    QList<QByteArray>    OpenedFilesList;

    // Menu
    QMenu*         RightMenu{nullptr};
    QAction*       MoveUp{nullptr};
    QAction*       MoveDown{nullptr};
    QAction*       DeleteItem{nullptr};

    void initRightMenu();
    void finiRightMenu();

private slots:
    void showListRightMenu(const QPoint &pos);
    void MoveFileUp();
    void MoveFileDown();
    void DeleteFile();
    void actionAddFileTriggered();
    void actionSaveTriggered();
};

#endif // MERGEFILESWINDOW_H
