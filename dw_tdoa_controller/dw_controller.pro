#-------------------------------------------------
#
# Project created by QtCreator 2014-02-04T17:25:51
#
#-------------------------------------------------
cache()

QT       += core gui network widgets xml serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = RTLSController
QMAKE_INFO_PLIST = Info.plist
RC_ICONS = res/DWctrler.ico

INCLUDEPATH += graphics models network views util tools

QMAKE_LFLAGS+=-Wl,-Map=mapfile

SOURCES += \
    main.cpp \
    RTLSControllerApplication.cpp \
    views/mainwindow.cpp \
    views/anchorlistwidget.cpp \
    views/anchorpropertieswidget.cpp \
    views/GraphicsWidget.cpp \
    views/connectionwidget.cpp \
    views/GraphicsView.cpp \
    views/ModelInspectorWidget.cpp \
    views/ViewSettingsWidget.cpp \
    views/MinimapView.cpp \
    views/AnchorMenu.cpp \
    views/SceneMenu.cpp \
    network/RTLSControl.cpp \
    models/DataModel.cpp \
    models/DataAbstractItem.cpp \
    models/DataRoot.cpp \
    models/DataAnchor.cpp \
    models/DataLink.cpp \
    models/ViewSettings.cpp \
    graphics/GraphicsDataModel.cpp \
    graphics/GraphicsDataAnchor.cpp \
    graphics/GraphicsDataItem.cpp \
    graphics/GraphicsDataLink.cpp \
    util/QPropertyModel.cpp \
    tools/OriginTool.cpp \
    tools/ScaleTool.cpp \
    tools/RubberBandTool.cpp \
    network/RTLSCLEConnection.cpp \
    network/RTLSClient.cpp \
    views/ChannelSettingsWidget.cpp


HEADERS  += \
    RTLSControllerApplication.h \
    views/mainwindow.h \
    views/anchorlistwidget.h \
    views/anchorpropertieswidget.h \
    views/GraphicsWidget.h \
    views/connectionwidget.h \
    views/GraphicsView.h \
    views/ModelInspectorWidget.h \
    views/ViewSettingsWidget.h \
    views/MinimapView.h \
    views/AnchorMenu.h \
    views/SceneMenu.h \
    network/RTLSControl.h \
    models/DataAbstractItem.h \
    models/DataAnchor.h \
    models/DataModel.h \
    models/DataRoot.h \
    models/DataLink.h \
    models/ViewSettings.h \
    graphics/GraphicsDataModel.h \
    graphics/GraphicsDataAnchor.h \
    graphics/GraphicsDataItem.h \
    graphics/GraphicsDataLink.h \
    util/QPropertyModel.h \
    tools/AbstractTool.h \
    tools/OriginTool.h \
    tools/ScaleTool.h \
    tools/RubberBandTool.h \
    network/RTLSCLEConnection.h \
    network/RTLSClient.h \
    views/ChannelSettingsWidget.h \
    network/rtls_interface.h


FORMS    += \
    views/mainwindow.ui \
    views/anchorlistwidget.ui \
    views/anchorpropertieswidget.ui \
    views/GraphicsWidget.ui \
    views/connectionwidget.ui \
    views/ModelInspectorWidget.ui \
    views/ViewSettingsWidget.ui \
    views/ChannelSettingsWidget.ui

RESOURCES += \
    res/resources.qrc

