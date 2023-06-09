cmake_minimum_required(VERSION 3.5)

project(BinaryViewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include_directories(include lib lib/Openssl/include Window)
set(LINK_DIR  lib/Openssl/lib)
link_directories(${LINK_DIR})

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
        lib/LzmaDecompress/LzmaDecompressLibInternal.h
)

set(UEFI_DECOMPRESS
        lib/BaseUefiDecompress/UefiDecompressLib.h
        lib/BaseUefiDecompress/BaseUefiDecompressLibInternals.h
        lib/BaseUefiDecompress/BaseUefiDecompressLib.cpp
)

set(INCLUDE_FILE
        include/Base.h
        include/PeImage.h
        include/ELF.h
        include/FspHeader.h
        include/PiFirmwareFile.h
        include/PiFirmwareVolume.h
        include/PiDependency.h
        include/SymbolDefinition.h
        include/GuidDefinition.h
        include/BaseMemoryLib.h
        include/BaseMemoryLib.cpp
        include/Microcode.h
        include/VariableFormat.h
        include/BootGuard.h
        include/ME.h
        include/GbE.h
        include/descriptor.h
        include/Acpi.h
)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/logo/BIOS Viewer.rc")

#link_directories(D:/Code/BinaryViewer/lib/LzmaDecompress/Sdk/C)

set(UI_SOURCES
        Window/HexViewDialog.ui
        Window/SettingsDialog.ui
        Window/InfoWindow.ui
        Window/SearchDialog.ui
        Window/TabWindow.ui
        Window/StartWindow.ui
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
)

set(PROJECT_SOURCES
        main.cpp
        logo/Images.qrc
        ${LIBRARY_SOURCES}
        ${WINDOW_SOURCES}
        ${INCLUDE_FILE}
        ${LZMA_SDK}
        ${UEFI_DECOMPRESS}
        ${app_icon_resource_windows}
        ${UI_SOURCES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(BinaryViewer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET BinaryViewer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(BinaryViewer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(BinaryViewer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(BinaryViewer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets libcrypto)

set_target_properties(BinaryViewer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS BinaryViewer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BinaryViewer)
endif()
