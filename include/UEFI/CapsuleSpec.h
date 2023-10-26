#pragma once
#include "C/Base.h"
#include "UEFI/GUID.h"

typedef UINT64                    EFI_PHYSICAL_ADDRESS;
typedef UINT64                    EFI_VIRTUAL_ADDRESS;

///
/// EFI Capsule Block Descriptor
///
typedef struct {
    ///
    /// Length in bytes of the data pointed to by DataBlock/ContinuationPointer.
    ///
    UINT64    Length;
    union {
        ///
        /// Physical address of the data block. This member of the union is
        /// used if Length is not equal to zero.
        ///
        EFI_PHYSICAL_ADDRESS    DataBlock;
        ///
        /// Physical address of another block of
        /// EFI_CAPSULE_BLOCK_DESCRIPTOR structures. This
        /// member of the union is used if Length is equal to zero. If
        /// ContinuationPointer is zero this entry represents the end of the list.
        ///
        EFI_PHYSICAL_ADDRESS    ContinuationPointer;
    } Union;
} EFI_CAPSULE_BLOCK_DESCRIPTOR;

///
/// EFI Capsule Header.
///
typedef struct {
    ///
    /// A GUID that defines the contents of a capsule.
    ///
    EFI_GUID    CapsuleGuid;
    ///
    /// The size of the capsule header. This may be larger than the size of
    /// the EFI_CAPSULE_HEADER since CapsuleGuid may imply
    /// extended header entries
    ///
    UINT32      HeaderSize;
    ///
    /// Bit-mapped list describing the capsule attributes. The Flag values
    /// of 0x0000 - 0xFFFF are defined by CapsuleGuid. Flag values
    /// of 0x10000 - 0xFFFFFFFF are defined by this specification
    ///
    UINT32      Flags;
    ///
    /// Size in bytes of the capsule.
    ///
    UINT32      CapsuleImageSize;
} EFI_CAPSULE_HEADER;

///
/// The EFI System Table entry must point to an array of capsules
/// that contain the same CapsuleGuid value. The array must be
/// prefixed by a UINT32 that represents the size of the array of capsules.
///
typedef struct {
    ///
    /// the size of the array of capsules.
    ///
    UINT32    CapsuleArrayNumber;
    ///
    /// Point to an array of capsules that contain the same CapsuleGuid value.
    ///
    VOID      *CapsulePtr[1];
} EFI_CAPSULE_TABLE;

#define CAPSULE_FLAGS_PERSIST_ACROSS_RESET   0x00010000
#define CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE  0x00020000
#define CAPSULE_FLAGS_INITIATE_RESET         0x00040000

#pragma pack(1)

typedef struct {
    UINT32    Version;

    ///
    /// The number of drivers included in the capsule and the number of corresponding
    /// offsets stored in ItemOffsetList array.
    ///
    UINT16    EmbeddedDriverCount;

    ///
    /// The number of payload items included in the capsule and the number of
    /// corresponding offsets stored in the ItemOffsetList array.
    ///
    UINT16    PayloadItemCount;

    ///
    /// Variable length array of dimension [EmbeddedDriverCount + PayloadItemCount]
    /// containing offsets of each of the drivers and payload items contained within the capsule
    ///
    // UINT64 ItemOffsetList[];
} EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER;

typedef struct {
    UINT32      Version;

    ///
    /// Used to identify device firmware targeted by this update. This guid is matched by
    /// system firmware against ImageTypeId field within a EFI_FIRMWARE_IMAGE_DESCRIPTOR
    ///
    EFI_GUID    UpdateImageTypeId;

    ///
    /// Passed as ImageIndex in call to EFI_FIRMWARE_MANAGEMENT_PROTOCOL.SetImage()
    ///
    UINT8       UpdateImageIndex;
    UINT8       reserved_bytes[3];

    ///
    /// Size of the binary update image which immediately follows this structure
    ///
    UINT32      UpdateImageSize;

    ///
    /// Size of the VendorCode bytes which optionally immediately follow binary update image in the capsule
    ///
    UINT32      UpdateVendorCodeSize;

    ///
    /// The HardwareInstance to target with this update. If value is zero it means match all
    /// HardwareInstances. This field allows update software to target only a single device in
    /// cases where there are more than one device with the same ImageTypeId GUID.
    /// This header is outside the signed data of the Authentication Info structure and
    /// therefore can be modified without changing the Auth data.
    ///
    UINT64    UpdateHardwareInstance;

    ///
    /// A 64-bit bitmask that determines what sections are added to the payload.
    /// #define CAPSULE_SUPPORT_AUTHENTICATION 0x0000000000000001
    /// #define CAPSULE_SUPPORT_DEPENDENCY 0x0000000000000002
    ///
    UINT64    ImageCapsuleSupport;
} EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER;

#pragma pack()

#define EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER_INIT_VERSION        0x00000001
#define EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER_INIT_VERSION  0x00000003
#define CAPSULE_SUPPORT_AUTHENTICATION                             0x0000000000000001
#define CAPSULE_SUPPORT_DEPENDENCY                                 0x0000000000000002

//
// _WIN_CERTIFICATE.wCertificateType
//
#define WIN_CERT_TYPE_PKCS_SIGNED_DATA  0x0002
#define WIN_CERT_TYPE_EFI_PKCS115       0x0EF0
#define WIN_CERT_TYPE_EFI_GUID          0x0EF1

///
/// The WIN_CERTIFICATE structure is part of the PE/COFF specification.
///
typedef struct {
    ///
    /// The length of the entire certificate,
    /// including the length of the header, in bytes.
    ///
    UINT32    dwLength;
    ///
    /// The revision level of the WIN_CERTIFICATE
    /// structure. The current revision level is 0x0200.
    ///
    UINT16    wRevision;
    ///
    /// The certificate type. See WIN_CERT_TYPE_xxx for the UEFI
    /// certificate types. The UEFI specification reserves the range of
    /// certificate type values from 0x0EF0 to 0x0EFF.
    ///
    UINT16    wCertificateType;
    ///
    /// The following is the actual certificate. The format of
    /// the certificate depends on wCertificateType.
    ///
    /// UINT8 bCertificate[ANYSIZE_ARRAY];
    ///
} WIN_CERTIFICATE;

///
/// WIN_CERTIFICATE_UEFI_GUID.CertType
///
#define EFI_CERT_TYPE_RSA2048_SHA256_GUID \
{0xa7717414, 0xc616, 0x4977, {0x94, 0x20, 0x84, 0x47, 0x12, 0xa7, 0x35, 0xbf } }

///
/// WIN_CERTIFICATE_UEFI_GUID.CertData
///
typedef struct {
    EFI_GUID    HashType;
    UINT8       PublicKey[256];
    UINT8       Signature[256];
} EFI_CERT_BLOCK_RSA_2048_SHA256;

///
/// Certificate which encapsulates a GUID-specific digital signature
///
typedef struct {
    ///
    /// This is the standard WIN_CERTIFICATE header, where
    /// wCertificateType is set to WIN_CERT_TYPE_EFI_GUID.
    ///
    WIN_CERTIFICATE    Hdr;
    ///
    /// This is the unique id which determines the
    /// format of the CertData. .
    ///
    EFI_GUID           CertType;
    ///
    /// The following is the certificate data. The format of
    /// the data is determined by the CertType.
    /// If CertType is EFI_CERT_TYPE_RSA2048_SHA256_GUID,
    /// the CertData will be EFI_CERT_BLOCK_RSA_2048_SHA256 structure.
    ///
//    UINT8              CertData[1];
} WIN_CERTIFICATE_UEFI_GUID;

///
/// Certificate which encapsulates the RSASSA_PKCS1-v1_5 digital signature.
///
/// The WIN_CERTIFICATE_UEFI_PKCS1_15 structure is derived from
/// WIN_CERTIFICATE and encapsulate the information needed to
/// implement the RSASSA-PKCS1-v1_5 digital signature algorithm as
/// specified in RFC2437.
///
typedef struct {
    ///
    /// This is the standard WIN_CERTIFICATE header, where
    /// wCertificateType is set to WIN_CERT_TYPE_UEFI_PKCS1_15.
    ///
    WIN_CERTIFICATE    Hdr;
    ///
    /// This is the hashing algorithm which was performed on the
    /// UEFI executable when creating the digital signature.
    ///
    EFI_GUID           HashAlgorithm;
    ///
    /// The following is the actual digital signature. The
    /// size of the signature is the same size as the key
    /// (1024-bit key is 128 bytes) and can be determined by
    /// subtracting the length of the other parts of this header
    /// from the total length of the certificate as found in
    /// Hdr.dwLength.
    ///
    /// UINT8 Signature[];
    ///
} WIN_CERTIFICATE_EFI_PKCS1_15;

///
/// Image Attribute - Authentication Required
///
typedef struct {
    ///
    /// It is included in the signature of AuthInfo. It is used to ensure freshness/no replay.
    /// It is incremented during each firmware image operation.
    ///
    UINT64                       MonotonicCount;
    ///
    /// Provides the authorization for the firmware image operations. It is a signature across
    /// the image data and the Monotonic Count value. Caller uses the private key that is
    /// associated with a public key that has been provisioned via the key exchange.
    /// Because this is defined as a signature, WIN_CERTIFICATE_UEFI_GUID.CertType must
    /// be EFI_CERT_TYPE_PKCS7_GUID.
    ///
    WIN_CERTIFICATE_UEFI_GUID    AuthInfo;
} EFI_FIRMWARE_IMAGE_AUTHENTICATION;

typedef struct {
    UINT32    Signature;
    UINT32    HeaderSize;
    UINT32    FwVersion;
    UINT32    LowestSupportedVersion;
} FMP_PAYLOAD_HEADER;
