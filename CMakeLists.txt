cmake_minimum_required(VERSION 3.5)

project(BiosViewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "D:/Qt/6.3.1/msvc2019_64/lib/cmake")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

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
        include/PiFirmwareFile.h
        include/PiFirmwareVolume.h
        include/PiDependency.h
        include/SymbolDefinition.h
        include/GuidDefinition.h
        include/BaseMemoryLib.h
        include/BaseMemoryLib.cpp
        include/Microcode.h
        include/VariableFormat.h
)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/logo/BIOS Viewer.rc")

link_directories(D:/Code/BiosViewer/lib/LzmaDecompress/Sdk/C)

set(PROJECT_SOURCES
        main.cpp
        lib/GuidDefinition.cpp
        lib/BaseLib.h
        lib/BaseLib.cpp
        lib/UefiLib.h
        lib/UefiLib.cpp
        lib/iwfi.h
        lib/iwfi.cpp
        lib/Model.h
        lib/Model.cpp
        lib/Section.cpp
        lib/QHexView/qhexview.cpp
        lib/QHexView/qhexview.h
        UefiDecode.cpp
        mainwindow.cpp
        BinaryTool.cpp
        mainwindow.h
        mainwindow.ui
        HexViewDialog.h HexViewDialog.cpp HexViewDialog.ui
        SettingsDialog.h SettingsDialog.cpp SettingsDialog.ui
        InfoWindow.h InfoWindow.cpp InfoWindow.ui
        SearchDialog.h SearchDialog.cpp SearchDialog.ui
        logo/Images.qrc
        theme/light/lightstyle.qrc
        theme/dark/darkstyle.qrc
        ${INCLUDE_FILE}
        ${LZMA_SDK}
        ${UEFI_DECOMPRESS}
        ${app_icon_resource_windows}
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

target_link_libraries(BiosViewer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

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
