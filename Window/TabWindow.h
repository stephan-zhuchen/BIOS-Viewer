#ifndef TABWINDOW_H
#define TABWINDOW_H

#include <QWidget>
#include <QSettings>
#include <QVBoxLayout>
#include <QTabWidget>

namespace Ui {
class TabWindow;
}

class TabWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TabWindow(QWidget *parent = nullptr);
    ~TabWindow();

    void SetNewTabAndText(QString tabName, QString txt);
    void CollectTabAndShow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::TabWindow *ui;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QSettings setting{"Intel", "BiosViewer"};
};

#endif // TABWINDOW_H
