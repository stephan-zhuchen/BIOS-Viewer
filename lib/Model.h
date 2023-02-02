#pragma once
#include <QMetaType>
#include <QStringList>
#include "UefiLib.h"

using namespace UefiSpace;

class FfsModel;
class FvModel;

class DataModel
{
protected:
//    QStringList rowData;
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
    void    setName(QString txt);
    void    setType(QString txt);
    void    setSubtype(QString txt);
    void    setText(QString txt);
    QString getName() const;
    QString getText() const;
    QString getType() const;
    QString getSubType() const;
    QStringList getData() const;
};

class SectionModel: public DataModel {
public:
    FfsModel *parentModel;
public:
    SectionModel(CommonSection *section, FfsModel *parent);
    ~SectionModel();
};


class FfsModel: public DataModel {
public:
    FvModel *parentModel;
public:
    FfsModel(FfsFile *ffs, FvModel *parent);
    ~FfsModel();
    QString getFmpDeviceName();
};

class FvModel: public DataModel {
public:
//    vector<FfsModel*> FfsModelData;
public:
    FvModel(FirmwareVolume *fv);
    ~FvModel();
};

Q_DECLARE_METATYPE(DataModel)
