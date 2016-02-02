#-------------------------------------------------
#
# Project created by QtCreator 2013-02-02T15:04:16
#
#-------------------------------------------------

QT       += core gui printsupport webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FlySightViewer
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    dataplot.cpp \
    dataview.cpp \
    waypoint.cpp \
    datapoint.cpp \
    configdialog.cpp \
    mapview.cpp \
    common.cpp \
    videoview.cpp \
    windplot.cpp \
    liftdragplot.cpp \
    scoringview.cpp \
    genome.cpp \
    orthoview.cpp \
    playbackview.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    datapoint.h \
    dataplot.h \
    dataview.h \
    waypoint.h \
    plotvalue.h \
    configdialog.h \
    mapview.h \
    common.h \
    videoview.h \
    windplot.h \
    liftdragplot.h \
    scoringview.h \
    genome.h \
    orthoview.h \
    playbackview.h

FORMS    += mainwindow.ui \
    configdialog.ui \
    videoview.ui \
    scoringview.ui \
    playbackview.ui

win32 {
    RC_ICONS = FlySightViewer.ico
}
else {
    ICON = FlySightViewer.icns
}

RESOURCES += \
    resource.qrc

INCLUDEPATH += ../include
INCLUDEPATH += ../include/wwWidgets

win32 {
    LIBS += -L../lib
    LIBS += -lvlc-qt -lvlc-qt-widgets
    LIBS += -lwwwidgets4
}
macx {
    QMAKE_LFLAGS += -F../frameworks
    LIBS         += -framework VLCQtCore
    LIBS         += -framework VLCQtQml
    LIBS         += -framework VLCQtWidgets
}
else {
    LIBS += -L/usr/local/lib
    LIBS += -lvlc-qt -lvlc-qt-widgets
}
