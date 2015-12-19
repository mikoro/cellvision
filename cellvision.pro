TEMPLATE = app
TARGET = cellvision
DESTDIR = bin
OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build
UI_DIR = build
CONFIG += release warn_off c++11 qt
CONFIG -= app_bundle
QT += core gui widgets
LIBS += -ltiff
QMAKE_CXXFLAGS += -mmacosx-version-min=10.9
QMAKE_POST_LINK += platform/mac/post-build.sh

INCLUDEPATH += include \
               src \
			   /opt/local/include

LIBPATH += /opt/local/lib

HEADERS += src/Common.h \
           src/ImageLoader.h \
           src/Log.h \
           src/MainWindow.h \
           src/RenderWidget.h \
           src/stdafx.h \
           src/SysUtils.h

FORMS += src/MainWindow.ui

SOURCES += src/ImageLoader.cpp \
           src/Log.cpp \
           src/Main.cpp \
           src/MainWindow.cpp \
           src/RenderWidget.cpp \
           src/SysUtils.cpp

RESOURCES += src/MainWindow.qrc
