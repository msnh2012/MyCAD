#include "Application.h"

#include "Translate.h"

#include <Standard_WarningsDisable.hxx>
#include <QMessageBox>
#include <QMdiSubWindow>
#include <Standard_WarningsRestore.hxx>

#include <OSD_Environment.hxx>

#include <stdlib.h>

ApplicationWindow::ApplicationWindow()
    : ApplicationCommonWindow( ),
      myImportPopup( 0 ),
      myExportPopup( 0 )
{
  createTranslatePopups();
}

ApplicationWindow::~ApplicationWindow()
{
}

void ApplicationWindow::createTranslatePopups()
{
    if ( !myImportPopup )
        myImportPopup = new QMenu( QObject::tr( "MNU_FILE_IMPORT" ), this );

    if ( !myExportPopup )
	    myExportPopup = new QMenu( QObject::tr( "MNU_FILE_EXPORT" ), this );

	QAction* a;
	a = new QAction( QObject::tr("MNU_IMPORT_BREP"), this );
	a->setStatusTip( QObject::tr("TBR_IMPORT_BREP") );
	connect( a, SIGNAL( triggered() ), this, SLOT( onImport() ) );
	myCasCadeTranslateActions.insert( FileImportBREPId, a );
	myImportPopup->addAction( a );

	a = new QAction( QObject::tr("MNU_EXPORT_BREP"), this );
	a->setStatusTip( QObject::tr("TBR_EXPORT_BREP") );
	connect( a, SIGNAL( triggered() ), this, SLOT( onExport() ) );
	myCasCadeTranslateActions.insert( FileExportBREPId, a );
	myExportPopup->addAction( a );

	a = new QAction( QObject::tr("MNU_IMPORT_IGES"), this );
	a->setStatusTip( QObject::tr("TBR_IMPORT_IGES") );
	connect( a, SIGNAL( triggered() ), this, SLOT( onImport() ) );
	myCasCadeTranslateActions.insert( FileImportIGESId, a );
	myImportPopup->addAction( a );

	a = new QAction( QObject::tr("MNU_EXPORT_IGES"), this );
	a->setStatusTip( QObject::tr("TBR_EXPORT_IGES") );
	connect( a, SIGNAL( triggered() ), this, SLOT( onExport() ) );
	myCasCadeTranslateActions.insert( FileExportIGESId, a );
	myExportPopup->addAction( a );

	a = new QAction( QObject::tr("MNU_IMPORT_STEP"), this );
	a->setStatusTip( QObject::tr("TBR_IMPORT_STEP") );
	connect( a, SIGNAL( triggered() ), this, SLOT( onImport() ) );
	myCasCadeTranslateActions.insert( FileImportSTEPId, a );
	myImportPopup->addAction( a );

	a = new QAction( QObject::tr("MNU_EXPORT_STEP"), this );
	a->setStatusTip( QObject::tr("TBR_EXPORT_STEP") );
	connect( a, SIGNAL( triggered() ), this, SLOT( onExport() ) );
	myCasCadeTranslateActions.insert( FileExportSTEPId, a );
	myExportPopup->addAction( a );

    a = new QAction("STL ...", this );
    a->setStatusTip("Import STL");
    connect( a, SIGNAL( triggered() ), this, SLOT( onImport() ) );
    myCasCadeTranslateActions.insert( FileImportSTLId, a );
    myImportPopup->addAction( a );


	a = new QAction( QObject::tr("MNU_EXPORT_STL"), this );
	a->setStatusTip( QObject::tr("TBR_EXPORT_STL") );
	connect( a, SIGNAL( triggered() ), this, SLOT( onExport() ) );
	myCasCadeTranslateActions.insert( FileExportSTLId, a );
	myExportPopup->addAction( a );

	a = new QAction( QObject::tr("MNU_EXPORT_VRML"), this );
	a->setStatusTip( QObject::tr("TBR_EXPORT_VRML") );
	connect( a, SIGNAL( triggered() ), this, SLOT( onExport() ) );
	myCasCadeTranslateActions.insert( FileExportVRMLId, a );
	myExportPopup->addAction( a );

	myExportPopup->addSeparator();

	a = new QAction( QObject::tr("MNU_EXPORT_IMAGE"), this );
	a->setStatusTip( QObject::tr("TBR_EXPORT_IMAGE") );
	connect( a, SIGNAL( triggered() ), this, SLOT( onExportImage() ) );
	myExportPopup->addAction( a );
}

void ApplicationWindow::updateFileActions()
{
    if ( myDocuments.isEmpty() )
    {
        if ( !isDocument() )
        {
            getFilePopup()->insertMenu( getFileSeparator(), myExportPopup );
            getFilePopup()->insertMenu( myExportPopup->menuAction(), myImportPopup );
            mySeparator = getFilePopup()->insertSeparator( myImportPopup->menuAction() );
            createprimitiveMenu(getPrimitivePopup());
        }
        else
        {
            getFilePopup()->removeAction( myImportPopup->menuAction() );
            getFilePopup()->removeAction( myExportPopup->menuAction() );
            getFilePopup()->removeAction( mySeparator );
        }
    }
    ApplicationCommonWindow::updateFileActions();
}

void ApplicationWindow::onImport()
{
    QAction* a = (QAction*)sender();
    int type = translationFormat( a );
    if ( type < 0 )
        return;

    bool stat = translate( type, true );
    if ( stat )
    {
        DocumentCommon* doc = qobject_cast<MDIWindow*>( getWorkspace()->activeSubWindow()->widget() )->getDocument();
        doc->fitAll();
    }
}

void ApplicationWindow::onExport()
{
    QAction* a = (QAction*)sender();
    int type = translationFormat( a );
    if ( type < 0 )
        return;

    translate( type, false );
}

int ApplicationWindow::translationFormat( const QAction* a )
{
    int type = -1;
    for ( int i = FileImportBREPId; i <= FileImportSTLId; i++ )
    {
        if ( myCasCadeTranslateActions.at( i ) == a )
        {
            type = i;
            break;
        }
    }
    switch ( type )
    {
    case FileImportBREPId:
    case FileExportBREPId:
        type = Translate::FormatBREP;
        break;
    case FileImportIGESId:
    case FileExportIGESId:
        type = Translate::FormatIGES;
        break;
    case FileImportSTEPId:
    case FileExportSTEPId:
        type =  Translate::FormatSTEP;
        break;
    case FileImportSTLId:
    case FileExportSTLId:
        type = Translate::FormatSTL;
        break;
    case FileExportVRMLId:
        type = Translate::FormatVRML;
        break;
    }
    return type;
}

bool ApplicationWindow::translate( const int format, const bool import )
{
    static Translate* anTrans = createTranslator();
    DocumentCommon* doc = qobject_cast<MDIWindow*>( getWorkspace()->activeSubWindow()->widget() )->getDocument();
    Handle(AIS_InteractiveContext) context = doc->getContext();
    bool status;
    if ( import )
        status = anTrans->importModel( format, context,doc->myComponents);
    else
        status = anTrans->exportModel( format, context );

    if ( !status )
    {
        QString msg = QObject::tr( "INF_TRANSLATE_ERROR" );
        if ( !anTrans->info().isEmpty() )
            msg += QString( "\n" ) + anTrans->info();
        QMessageBox::critical( this, QObject::tr( "TIT_ERROR" ), msg, QObject::tr( "BTN_OK" ), QString::null, QString::null, 0, 0 );
    }

    return status;
}

Translate* ApplicationWindow::createTranslator()
{
  Translate* anTrans = new Translate( this/*, "Translator"*/ );
  return anTrans;
}

void ApplicationWindow::onSelectionChanged()
{
  ApplicationCommonWindow::onSelectionChanged();

  QMdiArea* ws = getWorkspace();
  DocumentCommon* doc = qobject_cast<MDIWindow*>( ws->activeSubWindow()->widget() )->getDocument();
  Handle(AIS_InteractiveContext) context = doc->getContext();
  bool anEnabled = (context->NbSelected() > 0);

  myCasCadeTranslateActions.at( FileExportBREPId )->setEnabled( anEnabled );
  myCasCadeTranslateActions.at( FileExportIGESId )->setEnabled( anEnabled );
  myCasCadeTranslateActions.at( FileExportSTEPId )->setEnabled( anEnabled );
  myCasCadeTranslateActions.at( FileExportSTLId )->setEnabled( anEnabled );
  myCasCadeTranslateActions.at( FileExportVRMLId )->setEnabled( anEnabled );
}

QString ApplicationWindow::getIEResourceDir()
{
  static QString aResourceDir =
    QString (OSD_Environment ("CSF_IEResourcesDefaults").Value().ToCString());
  if (aResourceDir.isEmpty())
    aResourceDir = QString (OSD_Environment ("CSF_OCCTResourcePath").Value().ToCString()) + "/samples";

  return aResourceDir;
}

void ApplicationWindow::onExportImage()
{
    MDIWindow* w = qobject_cast<MDIWindow*>( getWorkspace()->activeSubWindow()->widget() );
    if ( w )
        w->dump();
}

void ApplicationWindow::createprimitiveMenu(QMenu* myPrimitivePopup)
{
    // add a primitive menu
    myPrimitivePopup->clear();
    QString dir = getResourceDir() + QString( "/icon/" );
    QAction *actionBox,*actionCone,*actionSphere,*actionCylinder,*actionTorus;

    actionBox = new QAction( QPixmap( dir+"box"), "Box", this );
    actionBox->setToolTip("Create Box");
    actionBox->setStatusTip("Create Box");

    actionCone = new QAction( QPixmap( dir+"cone"), "Cone", this );
    actionCone->setToolTip("Create Cone");
    actionCone->setStatusTip("Create Cone");


    actionSphere = new QAction( QPixmap( dir+"sphere"), "Sphere", this );
    actionSphere->setToolTip("Create Sphere");
    actionSphere->setStatusTip("Create Sphere");

    actionCylinder = new QAction( QPixmap( dir+"cylinder"), "Cylinder", this );
    actionCylinder->setToolTip("Create Cylinder");
    actionCylinder->setStatusTip("Create Cylinder");

    actionTorus = new QAction( QPixmap( dir+"torus"), "Torus", this );
    actionTorus->setToolTip("Create Torus");
    actionTorus->setStatusTip("Create Torus");

    // Primitive
    connect(actionBox, SIGNAL(triggered()), this, SLOT(onmakeBox()));
    connect(actionCone, SIGNAL(triggered()), this, SLOT(onmakeCone()));
    connect(actionSphere, SIGNAL(triggered()), this, SLOT(onmakeSphere()));
    connect(actionCylinder, SIGNAL(triggered()), this, SLOT(onmakeCylinder()));
    connect(actionTorus, SIGNAL(triggered()), this, SLOT(onmakeTorus()));

    myPrimitivePopup->addAction(actionBox);
    myPrimitivePopup->addAction(actionCone);
    myPrimitivePopup->addAction(actionSphere);
    myPrimitivePopup->addAction(actionCylinder);
    myPrimitivePopup->addAction(actionTorus);
}

void ApplicationWindow::onmakeBox()
{
    BEdt->append("make box is clicked.");
    static Translate* anTrans = createTranslator();
    DocumentCommon* doc = qobject_cast<MDIWindow*>( getWorkspace()->activeSubWindow()->widget() )->getDocument();
    Handle(AIS_InteractiveContext) context = doc->getContext();
    anTrans->makeBox(context,doc->myComponents);

    getApplication()->updateTreeWidgetItem();
}

void ApplicationWindow::onmakeCone()
{
    BEdt->append("make cone is clicked.");
    static Translate* anTrans = createTranslator();
    DocumentCommon* doc = qobject_cast<MDIWindow*>( getWorkspace()->activeSubWindow()->widget() )->getDocument();
    Handle(AIS_InteractiveContext) context = doc->getContext();
    anTrans->makeCone(context,doc->myComponents);
    getApplication()->updateTreeWidgetItem();
}

void ApplicationWindow::onmakeSphere()
{
    BEdt->append("make sphere is clicked.");
    static Translate* anTrans = createTranslator();
    DocumentCommon* doc = qobject_cast<MDIWindow*>( getWorkspace()->activeSubWindow()->widget() )->getDocument();
    Handle(AIS_InteractiveContext) context = doc->getContext();
    anTrans->makeSphere(context,doc->myComponents);
    getApplication()->updateTreeWidgetItem();
}

void ApplicationWindow::onmakeCylinder()
{
    BEdt->append("make cylinder is clicked.");
    static Translate* anTrans = createTranslator();
    DocumentCommon* doc = qobject_cast<MDIWindow*>( getWorkspace()->activeSubWindow()->widget() )->getDocument();
    Handle(AIS_InteractiveContext) context = doc->getContext();
    anTrans->makeCylinder(context,doc->myComponents);
    getApplication()->updateTreeWidgetItem();
}

void ApplicationWindow::onmakeTorus()
{
    BEdt->append("make torus is clicked.");
    static Translate* anTrans = createTranslator();
    DocumentCommon* doc = qobject_cast<MDIWindow*>( getWorkspace()->activeSubWindow()->widget() )->getDocument();
    Handle(AIS_InteractiveContext) context = doc->getContext();
    anTrans->makeTorus(context,doc->myComponents);
    getApplication()->updateTreeWidgetItem();
}
