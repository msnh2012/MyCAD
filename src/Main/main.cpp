#include "Application.h"

#include <OSD_Environment.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QApplication>
#include <QTranslator>
#include <QPixmap>
#include <QLocale>
#include <Standard_WarningsRestore.hxx>
#include<QDebug>

int main ( int argc, char* argv[] )
{
#if QT_VERSION > 0x050000
    TCollection_AsciiString aPlugindsDirName = OSD_Environment ("QTDIR").Value();
    if (!aPlugindsDirName.IsEmpty())
        QApplication::addLibraryPath (QString (aPlugindsDirName.ToCString()) + "/plugins");
#endif
    QApplication a( argc, argv );

    QString resDir = ApplicationCommonWindow::getResourceDir();
    QString resIEDir = ApplicationWindow::getIEResourceDir();

    QTranslator *strTrans = new QTranslator;
    Standard_Boolean isOK = strTrans->load(resDir+"/trans/Common-string");
    if( isOK )
        a.installTranslator( strTrans );

    QTranslator iconTrans( 0 );
    isOK = iconTrans.load(resDir+"/trans/Common-icon");
    if( isOK )
        a.installTranslator( &iconTrans );

    QTranslator strIETrans( 0 );
    isOK = strIETrans.load( resDir+"/trans/Interface-string");
    if( isOK )
        a.installTranslator( &strIETrans );

    QObject::connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    ApplicationWindow* mw = new ApplicationWindow();
    mw->setWindowTitle( QObject::tr( "MyCAD" ) );
    mw->show();

    return a.exec();
}

