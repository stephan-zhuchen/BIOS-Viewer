#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QSpinBox>
#include <QLineEdit>
#include <QRegularExpressionValidator>

class HexSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    explicit HexSpinBox(QWidget *parent = nullptr);

protected:
    QValidator::State validate(QString &text, int &pos) const override;
    int valueFromText(const QString &text) const override;
    QString textFromValue(int value) const override;

private:
    QRegularExpressionValidator validator;
};

class GuidData1HexLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit GuidData1HexLineEdit(QWidget * parent = nullptr);
    ~GuidData1HexLineEdit() override;

signals:
    void GuidCopied(const QString &text);

private:
    QRegularExpressionValidator guidValidator;

protected:
    QValidator::State validate(QString &text, int &pos) const;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // INPUTDIALOG_H
