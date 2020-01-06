#-------------------------------------------------
#
# Project created by QtCreator 2020-01-6T20:00:00
#
#-------------------------------------------------
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = MyCAD
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
		mainwindow.cpp

HEADERS += mainwindow.h

TRANSLATIONS += MyCAD_zh_CN.ts

RESOURCES += \
    saribbonresouce.qrc \
	
CONFIG(debug, debug|release){
    DESTDIR = $$PWD/../../../build-Debug
}else {
    DESTDIR = $$PWD/../../../build-Release
}

include($$PWD/../../MyCAD.pri)
