cmake_minimum_required(VERSION 3.5)

project(BiosViewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:\\Qt\\6.5.1\\msvc2019_64\\")


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include_directories(include lib lib/Openssl/include lib/BrotliDecompress/brotli/include Window)
set(LINK_DIR  lib/Openssl/lib)
link_directories(${LINK_DIR})

include_directories(lib/VisualLeakDetector/include)
set(VLD_LINK_DIR  lib/VisualLeakDetector/lib/Win64)
link_directories(${VLD_LINK_DIR})

set(LZMA_SDK
        lib/LzmaDecompress/Sdk/C/7zTypes.h
        lib/LzmaDecompress/Sdk/C/7zVersion.h
        lib/LzmaDecompress/Sdk/C/Bra.h
        lib/LzmaDecompress/Sdk/C/Compiler.h
        lib/LzmaDecompress/Sdk/C/CpuArch.h
        lib/LzmaDecompress/Sdk/C/LzFind.h
        lib/LzmaDecompress/Sdk/C/LzHash.h
        lib/LzmaDecompress/Sdk/C/LzmaDec.h
        lib/LzmaDecompress/Sdk/C/Precomp.h
        lib/LzmaDecompress/Sdk/C/LzmaDec.cpp
        lib/LzmaDecompress/LzmaDecompress.cpp
        lib/LzmaDecompress/LzmaDecompressLib.h
)

set(UEFI_DECOMPRESS
        lib/BaseUefiDecompress/UefiDecompressLib.h
        lib/BaseUefiDecompress/BaseUefiDecompressLib.h
        lib/BaseUefiDecompress/BaseUefiDecompressLib.cpp
)

set(BROTLI_COMMON_C
    lib/BrotliDecompress/brotli/common/constants.cpp
    lib/BrotliDecompress/brotli/common/context.cpp
    lib/BrotliDecompress/brotli/common/dictionary.cpp
    lib/BrotliDecompress/brotli/common/platform.cpp
    lib/BrotliDecompress/brotli/common/shared_dictionary.cpp
    lib/BrotliDecompress/brotli/common/transform.cpp
)

set(BROTLI_DEC_C
    lib/BrotliDecompress/brotli/dec/bit_reader.cpp
    lib/BrotliDecompress/brotli/dec/decode.cpp
    lib/BrotliDecompress/brotli/dec/huffman.cpp
    lib/BrotliDecompress/brotli/dec/state.cpp
)

set(BROTLI_ENC_C
    lib/BrotliDecompress/brotli/enc/backward_references.cpp
    lib/BrotliDecompress/brotli/enc/backward_references_hq.cpp
    lib/BrotliDecompress/brotli/enc/bit_cost.cpp
    lib/BrotliDecompress/brotli/enc/block_splitter.cpp
    lib/BrotliDecompress/brotli/enc/brotli_bit_stream.cpp
    lib/BrotliDecompress/brotli/enc/cluster.cpp
    lib/BrotliDecompress/brotli/enc/command.cpp
    lib/BrotliDecompress/brotli/enc/compound_dictionary.cpp
    lib/BrotliDecompress/brotli/enc/compress_fragment.cpp
    lib/BrotliDecompress/brotli/enc/compress_fragment_two_pass.cpp
    lib/BrotliDecompress/brotli/enc/dictionary_hash.cpp
    lib/BrotliDecompress/brotli/enc/encode.cpp
    lib/BrotliDecompress/brotli/enc/encoder_dict.cpp
    lib/BrotliDecompress/brotli/enc/entropy_encode.cpp
    lib/BrotliDecompress/brotli/enc/fast_log.cpp
    lib/BrotliDecompress/brotli/enc/histogram.cpp
    lib/BrotliDecompress/brotli/enc/literal_cost.cpp
    lib/BrotliDecompress/brotli/enc/memory.cpp
    lib/BrotliDecompress/brotli/enc/metablock.cpp
    lib/BrotliDecompress/brotli/enc/static_dict.cpp
    lib/BrotliDecompress/brotli/enc/utf8_util.cpp
)

set(BROTI_DECOMPRESS
        lib/BrotliDecompress/BrotliDecompress.cpp
        lib/BrotliDecompress/BrotliDecompressLib.h
)

set(INCLUDE_FILE
        include/SymbolDefinition.h
        include/C/Base.h
        include/C/BaseMemoryLib.h
        include/C/Common.h
        include/UEFI/PeImage.h
        include/UEFI/ELF.h
        include/UEFI/FspHeader.h
        include/UEFI/PiFirmwareFile.h
        include/UEFI/PiFirmwareVolume.h
        include/UEFI/PiDependency.h
        include/UEFI/GuidDefinition.h
        include/UEFI/Microcode.h
        include/UEFI/VariableFormat.h
        include/UEFI/BootGuard.h
        include/UEFI/ME.h
        include/UEFI/GbE.h
        include/UEFI/descriptor.h
        include/UEFI/Acpi.h
        include/UEFI/UefiStatus.h
        include/EDKTool/MdeModuleHii.h
        include/EDKTool/UefiInternalFormRepresentation.h
        include/EDKTool/IfrParse.h
        include/EDKTool/Fce.h
        include/EDKTool/BinaryParse.h
)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/logo/BIOS Viewer.rc")

set(UI_SOURCES
        Window/HexViewDialog.ui
        Window/SettingsDialog.ui
        Window/InfoWindow.ui
        Window/SearchDialog.ui
        Window/TabWindow.ui
        Window/StartWindow.ui
        Window/BiosWindow.ui
        Window/BiosSearch.ui
)

set(WINDOW_SOURCES
        Window/BinaryTool.cpp
        Window/HexViewDialog.h
        Window/HexViewDialog.cpp
        Window/SettingsDialog.h
        Window/SettingsDialog.cpp
        Window/InfoWindow.h
        Window/InfoWindow.cpp
        Window/SearchDialog.h
        Window/SearchDialog.cpp
        Window/UefiDecode.cpp
        Window/RightMenu.cpp
        Window/inputdialog.h
        Window/inputdialog.cpp
        Window/TabWindow.h
        Window/TabWindow.cpp
        Window/StartWindow.h
        Window/StartWindow.cpp
        Window/BiosWindow.h
        Window/BiosWindow.cpp
        Window/HexWindow.h
        Window/HexWindow.cpp
        Window/BiosSearch.h
        Window/BiosSearch.cpp
    )

set(LIBRARY_SOURCES
        lib/GuidDefinition.cpp
        lib/BaseLib.h
        lib/BaseLib.cpp
        lib/UefiLib.h
        lib/UefiLib.cpp
        lib/iwfi.h
        lib/iwfi.cpp
        lib/BootGuard.cpp
        lib/Model.h
        lib/Model.cpp
        lib/Section.cpp
        lib/ElfLib.cpp
        lib/ElfLib.h
        lib/QHexView/qhexview.cpp
        lib/QHexView/HexEdit.cpp
        lib/QHexView/qhexview.h
        lib/Compatibility/BaseMemoryLib.cpp
        lib/Compatibility/Common.cpp
        lib/FCE/Expression.cpp
        lib/FCE/IfrParse.cpp
        lib/FCE/Fce.cpp
)

set(PROJECT_SOURCES
        main.cpp
        logo/Images.qrc
        ${LIBRARY_SOURCES}
        ${WINDOW_SOURCES}
        ${INCLUDE_FILE}
        ${app_icon_resource_windows}
        ${UI_SOURCES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(BiosViewer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET BiosViewer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(BiosViewer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(BiosViewer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

add_library(Lzma STATIC ${LZMA_SDK})
add_library(UefiDecompress STATIC ${UEFI_DECOMPRESS})
add_library(Brotli STATIC ${BROTI_DECOMPRESS})
add_library(BrotliCommon STATIC ${BROTLI_COMMON_C})
add_library(BrotliDec STATIC ${BROTLI_DEC_C})
add_library(BrotliEnc STATIC ${BROTLI_ENC_C})
target_link_libraries(Brotli PRIVATE BrotliCommon BrotliDec BrotliEnc)

target_link_libraries(BiosViewer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets libcrypto vld Brotli Lzma UefiDecompress)

set_target_properties(BiosViewer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS BiosViewer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BiosViewer)
endif()
