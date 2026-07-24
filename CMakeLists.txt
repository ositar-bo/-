cmake_minimum_required(VERSION 3.22)
project(BitP2PComm VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Network Multimedia MultimediaWidgets)

qt_add_executable(bit-p2p-comm
    MANUAL_FINALIZATION
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/PeerTransport.h
    src/PeerTransport.cpp
    src/MessageCodec.h
    src/MessageCodec.cpp
    assets/resources.qrc
)

target_link_libraries(bit-p2p-comm PRIVATE
    Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::MultimediaWidgets
)

target_compile_definitions(bit-p2p-comm PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

set_target_properties(bit-p2p-comm PROPERTIES
    OUTPUT_NAME "BIT点对点通信"
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "北京理工大学点对点通信"
    MACOSX_BUNDLE_GUI_IDENTIFIER "cn.edu.bit.p2pcomm"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

if(APPLE)
    set_source_files_properties(packaging/macos/AppIcon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(bit-p2p-comm PRIVATE packaging/macos/AppIcon.icns)
    set_target_properties(bit-p2p-comm PROPERTIES MACOSX_BUNDLE_ICON_FILE "AppIcon.icns")
endif()

include(GNUInstallDirs)
install(TARGETS bit-p2p-comm
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(UNIX AND NOT APPLE)
    install(FILES packaging/linux/cn.edu.bit.p2pcomm.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES assets/icons/app.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps RENAME cn.edu.bit.p2pcomm.svg)
endif()

qt_finalize_executable(bit-p2p-comm)
