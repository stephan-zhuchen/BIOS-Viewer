#include "inputdialog.h"
#include <QKeySequence>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>

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

GuidData1HexLineEdit::GuidData1HexLineEdit(QWidget * parent)
    :QLineEdit(parent), guidValidator(QRegularExpression("([0-9a-fA-F]){8}"))
{
    this->setValidator(&guidValidator);
}

GuidData1HexLineEdit::~GuidData1HexLineEdit() {}

QValidator::State GuidData1HexLineEdit::validate(QString &text, int &pos) const
{
    return guidValidator.validate(text, pos);
}

void GuidData1HexLineEdit::keyPressEvent(QKeyEvent *event) {
    if (event->matches(QKeySequence::Paste)) {
        qDebug() << "GuidData1HexLineEdit Paste";
        QClipboard *clipboard = QApplication::clipboard();
        QString text = clipboard->text();
        emit GuidCopied(text);
    } else {
        QLineEdit::keyPressEvent(event);
    }
}
