#ifndef FBMDEF_H
#define FBMDEF_H

#include "BootGuard.h"

//
// Data structure definitions
//
#pragma pack (1)

#define FSP_REGION_TYPE_FSPOT       0
#define FSP_REGION_TYPE_FSPM        1
#define FSP_REGION_TYPE_FSPS        2

#define COMPONENT_DIGEST0_PTR(Fbm)  \
(FSP_REGION_DIGEST *)((UINT8 *)(Fbm) + sizeof (FSP_BOOT_MANIFEST_STRUCTURE) - \
                                                                                   3 * sizeof (FSP_REGION_DIGEST) - sizeof (UINT8))

#define FSP_REGION0_PTR(Fbm)  \
    (FSP_REGION_STRUCTURE *)((UINT8 *)(Fbm) + sizeof (FSP_BOOT_MANIFEST_STRUCTURE))

#define IBB_SEGMENTS_PTR(FspRegion)  \
    (IBB_SEGMENT *) ((UINT8 *) FspRegion + sizeof (FSP_REGION_STRUCTURE))

    //
    // In FBM, all TPM required type digest should be there.
    //
    typedef struct {
    UINT8             ComponentID;  //0: FSP-O/T  1: FSP-M  2: FSP-S
    MAX_HASH_LIST     ComponentDigests;
} FSP_REGION_DIGEST;

typedef struct {
    UINT8             ComponentID;  //0: FSP-O/T  1: FSP-M  2: FSP-S
    UINT8             SegmentCnt;
    //IBB_SEGMENT     SegmentArray[];
} FSP_REGION_STRUCTURE;

#define FSP_BOOT_MANIFEST_STRUCTURE_ID  (*(UINT64 *)"__FBMS__")
#define FSP_BOOT_MANIFEST_STRUCTURE_VERSION_0_1          0x01
typedef struct {
    UINT8                         StructureId[8];
    UINT8                         StructVersion;         // 0x01
    UINT8                         Reserved1[3];          // UINT32 Alignment

    UINT16                        KeySignatureOffset;
    UINT16                        FspVersion;
    UINT8                         FspSvn;
    UINT8                         Reserved2;
    UINT32                        Flags;                 // UINT32 Alignment

    UINT8                         CompDigestCnt;
    FSP_REGION_DIGEST             ComponentDigests[3];   // digest for FSP/T, FSP-M, FSP-S

    UINT8                         FspRgnCnt;
    //FSP_REGION_STRUCTURE        FspRegions_0;          // FSP-O/T
    //FSP_REGION_STRUCTURE        FspRegions[2];         // FSP-M, FSP-S

    //KEY_AND_SIGNATURE_STRUCT    Signature;

} FSP_BOOT_MANIFEST_STRUCTURE;

#pragma pack ()

#endif // FBMDEF_H
