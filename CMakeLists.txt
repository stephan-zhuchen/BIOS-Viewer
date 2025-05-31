cmake_minimum_required(VERSION 3.15)

project(BiosViewerSuite
    VERSION 1.16
    LANGUAGES CXX
    DESCRIPTION "BIOS Viewer Suite with GUI and CLI versions"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    option(BUILD_GUI "Build GUI version (requires Qt)" OFF)
    option(BUILD_CLI "Build CLI version" ON)
else()
    option(BUILD_GUI "Build GUI version (requires Qt)" OFF)
    option(BUILD_CLI "Build CLI version" ON)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE INCLUDE_FILES "include/*")
include_directories(
        include
        lib
        lib/BrotliDecompress/brotli/include
        Volume
        Window
)

set(DECOMPRESS_LIB
        lib/BaseUefiDecompress/BaseUefiDecompressLib.h
        lib/BaseUefiDecompress/BaseUefiDecompressLib.cpp
        lib/BrotliDecompress/BrotliDecompress.cpp
        lib/BrotliDecompress/BrotliDecompressLib.h
        lib/LzmaDecompress/LzmaDecompress.cpp
        lib/LzmaDecompress/LzmaDecompressLib.h
        lib/Lz4Decompress/Lz4Decompress.h
        lib/Lz4Decompress/Lz4.h
        lib/Lz4Decompress/Lz4DecompressLib.cpp
        lib/Lz4Decompress/Lz4.cpp
)
aux_source_directory(lib/LzmaDecompress/Sdk/C LZMA_SDK)
aux_source_directory(lib/Lz4Decompress/Sdk LZ4_SDK)
aux_source_directory(lib/BrotliDecompress/brotli/common BROTLI_COMMON_C)
aux_source_directory(lib/BrotliDecompress/brotli/dec BROTLI_DEC_C)
aux_source_directory(lib/BrotliDecompress/brotli/enc BROTLI_ENC_C)
add_library(Lzma STATIC ${LZMA_SDK})
add_library(Lz4 STATIC ${LZ4_SDK})
add_library(Brotli STATIC ${BROTLI_COMMON_C} ${BROTLI_DEC_C} ${BROTLI_ENC_C})
add_library(Decompress STATIC ${DECOMPRESS_LIB})
target_link_libraries(Decompress PRIVATE Lzma Lz4 Brotli)


file(GLOB LIBRARY_H_SOURCES "lib/*.h")
file(GLOB VOLUME_H_SOURCES "Volume/*.h")
file(GLOB FILE_SYSTEM_H_SOURCES "Volume/UefiFileSystem/*.h")
file(GLOB PAYLOAD_H_SOURCES "Volume/Payload/*.h")
file(GLOB IFWI_REGION_H_SOURCES "Volume/IfwiRegion/*.h")
file(GLOB CAPSULE_H_SOURCES "Volume/Capsule/*.h")
file(GLOB FEATURE_H_SOURCES "Volume/Feature/*.h")
aux_source_directory(lib LIBRARY_SOURCES)
aux_source_directory(Volume VOLUME_SOURCES)
aux_source_directory(Volume/UefiFileSystem FILE_SYSTEM_SOURCES)
aux_source_directory(Volume/Payload PAYLOAD_SOURCES)
aux_source_directory(Volume/IfwiRegion IFWI_REGION_SOURCES)
aux_source_directory(Volume/Capsule CAPSULE_SOURCES)
aux_source_directory(Volume/Feature FEATURE_SOURCES)
file(GLOB_RECURSE COMPATIBILITY_SOURCES "lib/Compatibility/*")
set(COMMON_SOURCES
        ${LIBRARY_SOURCES}
        ${VOLUME_SOURCES}
        ${VOLUME_H_SOURCES}
        ${LIBRARY_H_SOURCES}
        ${COMPATIBILITY_SOURCES}
        ${FILE_SYSTEM_H_SOURCES}
        ${FILE_SYSTEM_SOURCES}
        ${PAYLOAD_SOURCES}
        ${PAYLOAD_H_SOURCES}
        ${IFWI_REGION_H_SOURCES}
        ${IFWI_REGION_SOURCES}
        ${CAPSULE_H_SOURCES}
        ${CAPSULE_SOURCES}
        ${FEATURE_H_SOURCES}
        ${FEATURE_SOURCES}
)

if(BUILD_CLI)
  set(CLI_SOURCES
        Cli/main.cpp
        Cli/cliview.cpp
        Cli/cliview.h
        Cli/BiosView/BiosCli.cpp
        Cli/BiosView/BiosCli.h
        Cli/HexView/HexCli.cpp
        Cli/HexView/HexCli.h
        ${COMMON_SOURCES}
  )
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # 使用系统ncurses库替代PDCurses
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIR})
    
    # 在Linux下使用系统OpenSSL
    find_package(OpenSSL REQUIRED)

    add_executable(binary-tool ${CLI_SOURCES})
    target_compile_definitions(binary-tool PRIVATE CLI_BIOS_VIEWER)
    target_link_libraries(binary-tool 
        Decompress 
        OpenSSL::Crypto 
        ${CURSES_LIBRARIES}
    )
  else()
    include_directories(lib/PDCurses/include)
    set(PDCURSES_LINK_DIR  lib/PDCurses)
    link_directories(${PDCURSES_LINK_DIR})
    set(LINK_DIR  lib/Openssl/lib/windows)
    link_directories(${LINK_DIR})
    add_executable(binary-tool ${CLI_SOURCES})
    target_compile_definitions(binary-tool PRIVATE CLI_BIOS_VIEWER)
    target_link_libraries(binary-tool libcrypto Decompress pdcurses)
  endif()

  install(TARGETS binary-tool
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      COMPONENT cli
  )
endif()


if(BUILD_GUI)
  set(CMAKE_AUTOUIC ON)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)

  find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
  find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
  get_filename_component(QT_INSTALL_PREFIX "${CMAKE_PREFIX_PATH}/../.." ABSOLUTE)
  set(OPENSSL_ROOT_DIR "${QT_INSTALL_PREFIX}/Tools/OpenSSLv3/Win_x64")
  find_package(OpenSSL REQUIRED)
  set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/logo/BIOS Viewer.rc")
  file(GLOB UI_SOURCES "Window/*.ui")
  file(GLOB_RECURSE WINDOW_SOURCES "Window/*")

  set(GUI_SOURCES
      logo/Images.qrc
      font/Fonts.qrc
      ${COMMON_SOURCES}
      ${WINDOW_SOURCES}
      ${INCLUDE_FILES}
      ${app_icon_resource_windows}
      ${UI_SOURCES}
  )

  if(WIN32)
    include_directories(lib/VisualLeakDetector/include)
    set(VLD_LINK_DIR  lib/VisualLeakDetector/lib/Win64)
    link_directories(${VLD_LINK_DIR})
  endif()

  if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
      qt_add_executable(BiosViewer MANUAL_FINALIZATION ${GUI_SOURCES})
  else()
      add_executable(BiosViewer ${GUI_SOURCES})
  endif()

  target_compile_definitions(BiosViewer PRIVATE GUI_BIOS_VIEWER)
  target_link_libraries(BiosViewer PRIVATE
      Qt${QT_VERSION_MAJOR}::Widgets
      Decompress
      OpenSSL::Crypto
  )
  if(WIN32)
    target_link_libraries(BiosViewer PRIVATE vld)
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_compile_options(BiosViewer PRIVATE -Wno-deprecated-declarations)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      target_compile_options(BiosViewer PRIVATE -Wno-deprecated)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      message("Using MSVC compiler")
  endif()

  if(QT_VERSION_MAJOR EQUAL 6)
      qt_finalize_executable(BiosViewer)
  endif()
endif()

