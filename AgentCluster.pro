#-------------------------------------------------
#
# Project created by QtCreator 2014-03-02T17:19:46
#
#-------------------------------------------------

QT       += core gui widgets


TARGET = AgentCluster
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    clustercanvas.cpp \
    agentcluster.cpp

FORMS += \
    clustercanvas.ui

HEADERS += \
    clustercanvas.h \
    def.h \
    agentcluster.h