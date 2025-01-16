//
// Created by stephan on 9/2/2023.
//
#pragma once
#include "Volume.h"
#include "Feature/FitClass.h"
#include "Feature/AcpiClass.h"

class BiosRegion: public Volume {
private:
    string          BiosID;
    bool            foundBiosID{false};
    bool            isResiliency{false};
    bool            DebugFlag{false};
    bool            FitValid{false};
public:
    FitTableClass           *FitTable{nullptr};
    vector<AcpiClass*>       AcpiTables;

    BiosRegion()=delete;
    BiosRegion(UINT8* buffer, INT64 length, INT64 offset=0);
    ~BiosRegion() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;

    void setBiosID();
    void setDebugFlag();
    void collectAcpiTable(Volume *parent = nullptr);
    [[nodiscard]] inline string getBiosID() const  { return BiosID; }
    [[nodiscard]] inline bool isFitValid() const { return FitValid; }
};
