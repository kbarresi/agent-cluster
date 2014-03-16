#-------------------------------------------------
#
# Project created by QtCreator 2014-03-02T17:19:46
#
#-------------------------------------------------

QT       += core gui widgets


TARGET = FASO
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    clustercanvas.cpp \
    agentcluster.cpp \
    gui/qgraphicsellipseitemobject.cpp \
    gui/qgraphicslineitemobject.cpp \
    faso.cpp

FORMS += \
    clustercanvas.ui

HEADERS += \
    clustercanvas.h \
    def.h \
    agentcluster.h \
    gui/qgraphicsellipseitemobject.h \
    gui/qgraphicslineitemobject.h \
    faso.h

RESOURCES += \
    gfx.qrc
