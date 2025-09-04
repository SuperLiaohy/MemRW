cmake_minimum_required(VERSION 3.20)

project(MemRW VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES MemRW_zh_CN.ts)

file(GLOB GET_ADDR_SRC
        "Src/GetAddr/*.cpp" "Inc/GetAddr/*.h")

file(GLOB CMSIS_DAP_SRC
        "Src/CMSIS_DAP/*.cpp" "Inc/CMSIS_DAP/*.h")

file(GLOB QTOVERRIDE_SRC
    "Src/QtOverride/*.ui" "Src/QtOverride/*.cpp" "Inc/QtOverride/*.h")

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}

        res.qrc
        RingBuffer.h

        ${GET_ADDR_SRC}
        ${QTOVERRIDE_SRC}
        ${CMSIS_DAP_SRC}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MemRW
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MemRW APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(MemRW SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MemRW
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(MemRW PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_include_directories(MemRW PRIVATE
        "Inc/GetAddr"
        "Inc/QtOverride"
        "Inc/CMSIS_DAP")

find_package(libdwarf CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE libdwarf::dwarf)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBUSB_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE ${LIBUSB_CFLAGS_OTHER})
message(STATUS libusb-- "${LIBUSB_INCLUDE_DIRS}")

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Charts)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Charts)

find_path(CHARTWIDGET_INCLUDE_DIR NAMES ChartWidget/chartwidget.h)
find_library(CHARTWIDGET_LIBRARY NAMES ChartWidget)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CHARTWIDGET_LIBRARY})
target_include_directories(MemRW PRIVATE ${CHARTWIDGET_INCLUDE_DIR})


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MemRW)
endif()
set_target_properties(MemRW PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS MemRW
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MemRW)
endif()
