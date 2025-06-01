//
// Created by stephan on 9/2/2023.
//

#pragma once
#include "Volume.h"

struct BinaryData
{
    string OpenedFileName;
    UINT8 *InputImage{nullptr};
    INT64 InputImageSize{};

    BinaryData() = default;
    ~BinaryData()
    {
        if (InputImage)
        {
            delete[] InputImage; // 释放动态分配的内存
            InputImage = nullptr;
        }
    }
};

class DataModel {
private:
    string name;
    string type;
    string subtype;
    Volume* modelData{};

    void setSectionModel(Volume *sec);
    void setFfsModel(Volume *file);
    void setFirmwareVolumeModel(Volume *vol);
    void setCompressedVolumeModel(Volume *vol);
    void setNvVariableHeaderModel();
    void setNvVariableEntryModel(Volume *entry);
public:
    DataModel()=default;
    DataModel(Volume* vol, string nm, string typ = "", string sbtyp = "");
    ~DataModel() = default;

    void InitFromVolume(Volume* vol);
    inline void setName(string txt) { name = std::move(txt); };
    inline void setType(string txt) { type = std::move(txt); };
    inline void setSubtype(string txt) { subtype = std::move(txt); };
    [[nodiscard]] inline string getName() const { return name; };
    [[nodiscard]] inline string getType() const { return type; };
    [[nodiscard]] inline string getSubType() const {return subtype; };
    [[nodiscard]] inline Volume* getVolume() const {return modelData; };
    [[nodiscard]] inline vector<string> getData() const { return {name, type, subtype}; };
};

