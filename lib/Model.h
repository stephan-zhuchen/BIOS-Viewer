#pragma once
#include <QMetaType>
#include <QStringList>
#include "UefiLib.h"

using namespace UefiSpace;

class DataModel
{
protected:
    QStringList rowData;
    QString name;
    QString type;
    QString subtype;
    QString text;
public:
    Volume* modelData;
    vector<DataModel*> volumeModelData;
    DataModel()=default;
    DataModel(Volume* model, QString nm, QString typ, QString sbtyp = "", QString txt = "");
    ~DataModel();
    void    setText(QString txt);
    QString getName() const;
    QString getText() const;
    QString getType() const;
    QString getSubType() const;
    QStringList getData() const;
};

class SectionModel: public DataModel {
public:
//    vector<DataModel*> volumeModelData;
public:
    SectionModel(CommonSection *section);
    ~SectionModel();
};


class FfsModel: public DataModel {
public:
//    vector<DataModel*> SectionModelData;
public:
    FfsModel(FfsFile *ffs);
    ~FfsModel();
};

class FvModel: public DataModel {
public:
//    vector<FfsModel*> FfsModelData;
public:
    FvModel(FirmwareVolume *fv);
    ~FvModel();
};

Q_DECLARE_METATYPE(DataModel)
