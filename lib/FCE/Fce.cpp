/** @file

 FCE is a tool which enables developers to retrieve and change HII configuration ("Setup")
 data in Firmware Device files (".fd" files).

 Copyright (c) 2011-2019, Intel Corporation. All rights reserved.<BR>
 SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "EDKTool/Fce.h"

#ifndef __GNUC__
#define COPY_STR      "copy \"%s\" \"%s\" > NUL"
#define RMDIR_STR     "rmdir /S /Q \"%s\" > NUL"
#define DEL_STR       "del \"%s\" > NUL"
#else
#define COPY_STR      "cp \"%s\" \"%s\" > /dev/null"
#define RMDIR_STR     "rm -r \"%s\" > /dev/null"
#define DEL_STR       "rm \"%s\" > /dev/null"
#endif

//
// Utility global variables
//
OPERATION_TYPE  Operations;

CHAR8  mInputFdName[MAX_FILENAME_LEN];
CHAR8  mOutputFdName[MAX_FILENAME_LEN];
CHAR8  mOutTxtName[MAX_FILENAME_LEN];
CHAR8  mSetupTxtName[MAX_FILENAME_LEN];

CHAR8* mUtilityFilename        = NULL;
UINT32 mEfiVariableAddr        = 0;

UQI_PARAM_LIST           *mUqiList = NULL;
UQI_PARAM_LIST           *mLastUqiList = NULL;
UEFI_LIST_ENTRY               mVarListEntry;
UEFI_LIST_ENTRY               mBfvVarListEntry;
UEFI_LIST_ENTRY               mAllVarListEntry;
UEFI_LIST_ENTRY               mFormSetListEntry;

//
// Store GUIDed Section guid->tool mapping
//
EFI_HANDLE mParsedGuidedSectionTools = NULL;

//
//gFfsArray is used to store all the FFS informations of Fd
//
G_EFI_FD_INFO               gEfiFdInfo;
//
//mMultiPlatformParam is used to store the structures about multi-platform support
//
MULTI_PLATFORM_PARAMETERS   mMultiPlatformParam;

UINT32                      mFormSetOrderRead;
UINT32                      mFormSetOrderParse;

CHAR8             mFullGuidToolDefinitionDir[_MAX_PATH];

CHAR8             *mFvNameGuidString = NULL;

