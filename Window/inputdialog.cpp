#include "inputdialog.h"

HexSpinBox::HexSpinBox(QWidget *parent) :
    QSpinBox(parent), validator(QRegularExpression("0x([0-9a-fA-F]){1,8}"))
{
    this->setRange(INT_MIN, INT_MAX);
    this->setPrefix("0x");
}

QValidator::State HexSpinBox::validate(QString &text, int &pos) const
{
    return validator.validate(text, pos);
}

QString HexSpinBox::textFromValue(int val) const
{
    return QString::number((uint)val, 16).toUpper();
}

int HexSpinBox::valueFromText(const QString &text) const
{
    return (int)text.toUInt(NULL, 16);
}
