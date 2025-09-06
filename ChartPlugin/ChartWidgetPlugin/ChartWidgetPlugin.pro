CONFIG      += plugin debug_and_release
TARGET      = $$qtLibraryTarget(chartwidgetplugin)
TEMPLATE    = lib

HEADERS     = chartwidgetplugin.h
SOURCES     = chartwidgetplugin.cpp

INCLUDEPATH += $$PWD/../ChartWidget
LIBS += -L$$OUT_PWD/../ChartWidget -lChartWidget


RESOURCES   = icons.qrc
LIBS        += -L. 

QT += designer

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include(chartwidget.pri)
