#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QSpinBox>
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

#endif // INPUTDIALOG_H
