######################################################################
# Automatically generated by qmake (2.01a) Mon Sep 1 16:01:54 2008
######################################################################

TEMPLATE = app
TARGET = watcher
DEPENDPATH += .
INCLUDEPATH += . ../../../include ../../util ../../../include/qwt ../../logger ../.. ../../watcherd
INCLUDEPATH += /usr/include/libxml2 /usr/local/include /usr/local/include/libxml2 /usr/X11R6/include /usr/include/qwt
QT += opengl 
CONFIG += qt x11
OBJECTS_DIR = ./objs
DEFINES += GRAPHICS MODULE_MOBILITY ggdb

# log4cxx issues warnings without this. 
QMAKE_CFLAGS_DEBUG += -fno-strict-aliasing -O0
QMAKE_CXXFLAGS_DEBUG += -fno-strict-aliasing -O0
QMAKE_CFLAGS_RELEASE += -fno-strict-aliasing
QMAKE_CXXFLAGS_RELEASE += -fno-strict-aliasing

LIBS += -L../../lib -L../logger -L../util
LIBS += -L/usr/X11R6/lib -lGL -lGLU -lglut
LIBS += -L/usr/local/lib -lidmef 
LIBS += -lidsCommunications 
LIBS += -lconfig++
LIBS += -llog4cxx
LIBS += -llogger
LIBS += -lwatcherutils
LIBS += -lwatcherd
LIBS += -lwatcher
LIBS += -lwatcherd
LIBS += -xml2
LIBS += -lqwt

win32 {
    error("No support for windows in watcher")
}

# Input
HEADERS += \
    backgroundImage.h \
    bitmap.h \
    config.h \
    graphPlot.h \
    manetglview.h \
    skybox.h \
    speedlabel.h \
    watcherGraphDialog.h \
    watcherMainWindow.h \
    watcherScrollingGraphControl.h

FORMS += watcher.ui

SOURCES += \
    backgroundImage.cpp \
    bitmap.cpp  \
    config.c \
    graphPlot.cpp   \
    main.cpp    \
    manetglview.cpp \
    skybox.cpp  \
    speedlabel.cpp  \
    watcherGraphDialog.cpp  \
    watcherMainWindow.cpp   \
    watcherScrollingGraphControl.cpp

