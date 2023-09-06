//
// Created by stephan on 9/2/2023.
//

#pragma once
#include <QMetaType>
#include <QStringList>
#include "Volume.h"

class DataModel {
private:
    QString name;
    QString type;
    QString subtype;
    Volume* modelData{};

    void setSectionModel(Volume *sec);
    void setFfsModel(Volume *file);
    void setFirmwareVolumeModel(Volume *vol);
    void setNvVariableHeaderModel(Volume *var);
    void setNvVariableEntryModel(Volume *entry);
public:
    DataModel()=default;
    DataModel(Volume* vol, QString nm, QString typ = "", QString sbtyp = "");
    ~DataModel() = default;

    void InitFromVolume(Volume* vol);
    inline void setName(QString txt) { name = std::move(txt); };
    inline void setType(QString txt) { type = std::move(txt); };
    inline void setSubtype(QString txt) { subtype = std::move(txt); };
    [[nodiscard]] inline QString getName() const { return name; };
    [[nodiscard]] inline QString getType() const { return type; };
    [[nodiscard]] inline QString getSubType() const {return subtype; };
    [[nodiscard]] inline Volume* getVolume() const {return modelData; };
    [[nodiscard]] inline QStringList getData() const { return QStringList() << name << type << subtype; };
};

