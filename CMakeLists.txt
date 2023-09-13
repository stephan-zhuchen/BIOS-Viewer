cmake_minimum_required(VERSION 3.5)

project(BiosViewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(LINK_DIR  lib/Openssl/lib/windows)
    link_directories(${LINK_DIR})
    include_directories(lib/VisualLeakDetector/include)
    set(VLD_LINK_DIR  lib/VisualLeakDetector/lib/Win64)
    link_directories(${VLD_LINK_DIR})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(LINK_DIR  lib/Openssl/lib/macos)
    link_directories(${LINK_DIR})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINK_DIR  lib/Openssl/lib/linux)
    link_directories(${LINK_DIR})
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
include_directories(include lib lib/Openssl/include lib/BrotliDecompress/brotli/include Window)

set(DECOMPRESS_LIB
        lib/BaseUefiDecompress/UefiDecompressLib.h
        lib/BaseUefiDecompress/BaseUefiDecompressLib.h
        lib/BaseUefiDecompress/BaseUefiDecompressLib.cpp
        lib/BrotliDecompress/BrotliDecompress.cpp
        lib/BrotliDecompress/BrotliDecompressLib.h
        lib/LzmaDecompress/LzmaDecompress.cpp
        lib/LzmaDecompress/LzmaDecompressLib.h
)

aux_source_directory(lib/LzmaDecompress/Sdk/C LZMA_SDK)
aux_source_directory(lib/BrotliDecompress/brotli/common BROTLI_COMMON_C)
aux_source_directory(lib/BrotliDecompress/brotli/dec BROTLI_DEC_C)
aux_source_directory(lib/BrotliDecompress/brotli/enc BROTLI_ENC_C)
file(GLOB_RECURSE INCLUDE_FILES "include/*")
set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/logo/BIOS Viewer.rc")
file(GLOB UI_SOURCES "Window/*.ui")
file(GLOB_RECURSE WINDOW_SOURCES "Window/*")
file(GLOB LIBRARY_H_SOURCES "lib/*.h")
file(GLOB FILE_SYSTEM_H_SOURCES "lib/UefiFileSystem/*.h")
file(GLOB IFWI_REGION_H_SOURCES "lib/IfwiRegion/*.h")
file(GLOB CAPSULE_H_SOURCES "lib/Capsule/*.h")
file(GLOB FEATURE_H_SOURCES "lib/Feature/*.h")
aux_source_directory(lib LIBRARY_SOURCES)
aux_source_directory(lib/UefiFileSystem FILE_SYSTEM_SOURCES)
aux_source_directory(lib/IfwiRegion IFWI_REGION_SOURCES)
aux_source_directory(lib/Capsule CAPSULE_SOURCES)
aux_source_directory(lib/Feature FEATURE_SOURCES)
file(GLOB_RECURSE COMPATIBILITY_SOURCES "lib/Compatibility/*")

set(PROJECT_SOURCES
        main.cpp
        logo/Images.qrc
        font/Fonts.qrc
        ${LIBRARY_SOURCES}
        ${LIBRARY_H_SOURCES}
        ${COMPATIBILITY_SOURCES}
        ${FILE_SYSTEM_H_SOURCES}
        ${FILE_SYSTEM_SOURCES}
        ${IFWI_REGION_H_SOURCES}
        ${IFWI_REGION_SOURCES}
        ${CAPSULE_H_SOURCES}
        ${CAPSULE_SOURCES}
        ${FEATURE_H_SOURCES}
        ${FEATURE_SOURCES}
        ${WINDOW_SOURCES}
        ${INCLUDE_FILES}
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

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(BiosViewer PRIVATE -Wno-deprecated-declarations)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(BiosViewer PRIVATE -Wno-deprecated)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("Using MSVC compiler")
endif()

add_library(Lzma STATIC ${LZMA_SDK})
add_library(Brotli STATIC ${BROTLI_COMMON_C} ${BROTLI_DEC_C} ${BROTLI_ENC_C})
add_library(Decompress STATIC ${DECOMPRESS_LIB})
target_link_libraries(Decompress PRIVATE Lzma Brotli)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(BiosViewer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets libcrypto vld Decompress)
else()
    target_link_libraries(BiosViewer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets crypto Decompress)
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.BiosViewer)
endif()
set_target_properties(BiosViewer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS BiosViewer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BiosViewer)
endif()
