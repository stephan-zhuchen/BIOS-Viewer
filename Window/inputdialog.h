#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QSpinBox>
#include <QLineEdit>
#include <QRegularExpressionValidator>

class HexSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    HexSpinBox(QWidget *parent = 0);

protected:
    QValidator::State validate(QString &text, int &pos) const;
    int valueFromText(const QString &text) const;
    QString textFromValue(int value) const;

private:
    QRegularExpressionValidator validator;
};

class GuidData1HexLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    GuidData1HexLineEdit(QWidget * parent = 0);
    ~GuidData1HexLineEdit();

signals:
    void GuidCopied(const QString &text);

private:
    QRegularExpressionValidator guidValidator;

protected:
    QValidator::State validate(QString &text, int &pos) const;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // INPUTDIALOG_H
