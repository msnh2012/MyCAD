#-------------------------------------------------
#
# Project created by QtCreator 2020-01-6T20:00:00
#
#-------------------------------------------------
QT       += core gui opengl

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
    Common/src/ApplicationCommon.cxx \
    Common/src/DocumentCommon.cxx \
    Common/src/MDIWindow.cxx \
    Common/src/Material.cxx \
    Common/src/OcctWindow.cxx \
    Common/src/Transparency.cxx \
    Common/src/View.cxx \
    Interface/src/Application.cxx \
    Interface/src/Translate.cxx \
            filereader.cpp \
		project.cpp

HEADERS += filereader.h \
    Common/src/ApplicationCommon.h \
    Common/src/DocumentCommon.h \
    Common/src/MDIWindow.h \
    Common/src/Material.h \
    Common/src/OcctWindow.h \
    Common/src/Transparency.h \
    Common/src/View.h \
    Interface/src/Application.h \
    Interface/src/Translate.h \
    project.h

TRANSLATIONS += PWD/../../resources/trans/MyCAD_zh_CN.ts


CONFIG(debug, debug|release){
    DESTDIR = $$PWD/../../../build-Debug
}else {
    DESTDIR = $$PWD/../../../build-Release
}

#include($$PWD/../../MyCAD.pri)
INCLUDEPATH += $$PWD/../../resources/OCCT/inc
DEPENDPATH += $$PWD/../../resources/OCCT/inc

LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKBin.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKBinL.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKBinTObj.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKBinXCAF.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKBO.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKBool.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKBRep.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKCAF.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKCDF.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKDCAF.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKDraw.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKernel.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKFeat.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKernel.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKFeat.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKFillet.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKG2d.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKG3d.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKGeomAlgo.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKGeomBase.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKHLR.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKIGES.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKLCAF.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKMath.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKMesh.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKMeshVS.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKOffset.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKOpenGl.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKPrim.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKQADraw.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKRWMesh.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKService.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKShHealing.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKStd.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKStdL.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKSTEP.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKSTEP209.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKSTEPAttr.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKSTEPBase.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKSTL.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKTObj.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKTObjDRAW.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKTopAlgo.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKTopTest.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKV3d.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKVCAF.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKViewerTest.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKVRML.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKXCAF.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKXDEDRAW.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKXDEIGES.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKXDESTEP.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKXMesh.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKXml.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKXmlL.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKXmlTObj.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKXmlXCAF.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKXSBase.dll
LIBS += -L$$PWD/../../resources/OCCT/lib/ -llibTKXSDRAW.dll


INCLUDEPATH += $$PWD/Interface/src
INCLUDEPATH += $$PWD/Common/src

RESOURCES += \
    ../../icon.qrc



