#include "ApplicationCommon.h"

#include "DocumentCommon.h"
#include "View.h"

#include <Standard_WarningsDisable.hxx>
#include <QFrame>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QApplication>
#include <QSignalMapper>
#include <Standard_WarningsRestore.hxx>

#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <OSD_Environment.hxx>

#include <stdlib.h>
#include <QDebug>
#include <QSplitter>
#include <QTabWidget>
#include <QTimer>
#include <QDateTime>
#include "Slice/slice.h"
static ApplicationCommonWindow* stApp = 0;
static QMdiArea* stWs = 0;

ApplicationCommonWindow::ApplicationCommonWindow()
    : QMainWindow( 0 ),
      myNbDocuments( 0 ),
      myIsDocuments(false),
      myStdToolBar( 0 ),
      myCasCadeBar( 0 ),
      myFilePopup( 0 ),
      myWindowPopup( 0 ),
      myPrimitivePopup( 0 ),
      myFileSeparator(NULL)
{
    stApp = this;

    // create and define the central widget
    QSplitter *UpSplitter = new QSplitter(Qt::Vertical,this);
    QSplitter *LSplitter = new QSplitter(Qt::Horizontal,UpSplitter);

    QTabWidget *tabwidget = new QTabWidget(LSplitter);
    treewidget = new QTreeWidget();
    treewidget->setHeaderLabel("project");
    tabwidget->addTab(treewidget,"model");
    tabwidget->addTab(new QTextEdit("tab2"),"tab2");
    stWs = new QMdiArea(LSplitter);
    //设置分割上窗口左右的宽度比例为1:2
    UpSplitter->setStretchFactor(0,1);
    UpSplitter->setStretchFactor(1,2);
    BEdt = new QTextEdit(tr("bottom"),UpSplitter);

    //设置分割窗口上下的宽度比例为1:2
    UpSplitter->setStretchFactor(0,8);
    UpSplitter->setStretchFactor(1,2);

    setCentralWidget( UpSplitter );
    createStandardOperations();
    createCasCadeOperations();

    connect(treewidget,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(onshowSelectedItem(QTreeWidgetItem*,int)));

    statusBar()->showMessage( QObject::tr("INF_READY"), 5000 );
    initStatusBar();
    resize( 1000, 700 );
}

ApplicationCommonWindow::~ApplicationCommonWindow()
{
}

void ApplicationCommonWindow::createStandardOperations()
{
    QPixmap newIcon, helpIcon, closeIcon, saveIcon;

    QString dir = getResourceDir() + QString( "/icon/" );
    newIcon = QPixmap( dir + QObject::tr("ICON_NEW") );
    helpIcon = QPixmap( dir + QObject::tr("ICON_HELP") );
    closeIcon = QPixmap( dir + QObject::tr("ICON_CLOSE") );
    saveIcon = QPixmap( dir + QObject::tr("ICON_SAVE") );

    QAction * fileNewAction, * fileCloseAction, * filePrefUseVBOAction,
            * fileQuitAction, * viewToolAction, * viewStatusAction, * helpAboutAction,
            *fileSaveAction,*FilePreferencesAction;

    fileNewAction = new QAction( newIcon, QObject::tr("MNU_NEW"), this );
    fileNewAction->setToolTip( QObject::tr("TBR_NEW") );
    fileNewAction->setStatusTip( QObject::tr("TBR_NEW") );
    fileNewAction->setShortcut( QObject::tr("CTRL+N") );
    connect( fileNewAction, SIGNAL( triggered() ) , this, SLOT( onNewDoc() ) );
    myStdActions.insert( FileNewId, fileNewAction );

    fileSaveAction = new QAction( saveIcon, QObject::tr("MNU_SAVE"), this );
    fileSaveAction->setToolTip( QObject::tr("TBR_SAVE") );
    fileSaveAction->setStatusTip( QObject::tr("TBR_SAVE") );
    fileSaveAction->setShortcut( QObject::tr("CTRL+S") );
    connect( fileSaveAction, SIGNAL( triggered() ) , this, SLOT( onSave() ) );
    myStdActions.insert( FileSaveId, fileSaveAction );

    fileCloseAction = new QAction( closeIcon, QObject::tr("MNU_CLOSE"), this );
    fileCloseAction->setToolTip( QObject::tr("TBR_CLOSE") );
    fileCloseAction->setStatusTip( QObject::tr("TBR_CLOSE") );
    fileCloseAction->setShortcut( QObject::tr("CTRL+W") );
    connect( fileCloseAction, SIGNAL( triggered() ) , this, SLOT( onCloseWindow() ) );
    myStdActions.insert( FileCloseId, fileCloseAction );

    filePrefUseVBOAction = new QAction( QObject::tr("MNU_USE_VBO"), this );
    filePrefUseVBOAction->setToolTip( QObject::tr("TBR_USE_VBO") );
    filePrefUseVBOAction->setStatusTip( QObject::tr("TBR_USE_VBO") );
    filePrefUseVBOAction->setCheckable( true );
    filePrefUseVBOAction->setChecked( true );
    connect( filePrefUseVBOAction, SIGNAL( triggered() ) , this, SLOT( onUseVBO() ) );
    myStdActions.insert( FilePrefUseVBOId, filePrefUseVBOAction );

    fileQuitAction = new QAction( QObject::tr("MNU_QUIT"), this );
    fileQuitAction->setToolTip( QObject::tr("TBR_QUIT") );
    fileQuitAction->setStatusTip( QObject::tr("TBR_QUIT") );
    fileQuitAction->setShortcut( QObject::tr("CTRL+Q") );
    connect( fileQuitAction, SIGNAL( triggered() ) , qApp, SLOT( closeAllWindows() ) );
    myStdActions.insert( FileQuitId, fileQuitAction );

    viewToolAction = new QAction( QObject::tr("MNU_TOOL_BAR"), this );
    viewToolAction->setToolTip( QObject::tr("TBR_TOOL_BAR") );
    viewToolAction->setStatusTip( QObject::tr("TBR_TOOL_BAR") );
    connect( viewToolAction, SIGNAL( triggered() ) , this, SLOT( onViewToolBar() ));
    viewToolAction->setCheckable( true );
    viewToolAction->setChecked( true );
    myStdActions.insert( ViewToolId, viewToolAction );

    viewStatusAction = new QAction( QObject::tr("MNU_STATUS_BAR"), this );
    viewStatusAction->setToolTip( QObject::tr("TBR_STATUS_BAR") );
    viewStatusAction->setStatusTip( QObject::tr("TBR_STATUS_BAR") );
    connect( viewStatusAction, SIGNAL( triggered() ), this, SLOT( onViewStatusBar() ));
    viewStatusAction->setCheckable( true );
    viewStatusAction->setChecked( true );
    myStdActions.insert( ViewStatusId, viewStatusAction );

    helpAboutAction = new QAction( helpIcon, QObject::tr("MNU_ABOUT"), this );
    helpAboutAction->setToolTip( QObject::tr( "TBR_ABOUT" ) );
    helpAboutAction->setStatusTip( QObject::tr( "TBR_ABOUT" ) );
    helpAboutAction->setShortcut( QObject::tr( "F1" ) );
    connect( helpAboutAction, SIGNAL( triggered() ) , this, SLOT( onAbout() ) );
    myStdActions.insert( HelpAboutId, helpAboutAction );

    // create preferences menu
    QMenu* aPrefMenu = new QMenu( QObject::tr("MNU_PREFERENCES") );
    aPrefMenu->addAction( filePrefUseVBOAction );

    // populate a menu with all actions
    myFilePopup = new QMenu( this );
    myFilePopup = menuBar()->addMenu( QObject::tr("MNU_FILE") );
    myFilePopup->addAction( fileNewAction );
    myFilePopup->addAction( fileSaveAction );
    myFilePopup->addAction( fileCloseAction );
    myFileSeparator = myFilePopup->addSeparator();
    myFilePopup->addMenu( aPrefMenu );
    myFileSeparator = myFilePopup->addSeparator();
    myFilePopup->addAction( fileQuitAction );

    // add a view menu
    QMenu * view = new QMenu( this );

    view = menuBar()->addMenu( QObject::tr("MNU_VIEW") );
    view->addAction( viewToolAction );
    view->addAction( viewStatusAction );

    // add a help menu
    QMenu * help = new QMenu( this );
    menuBar()->addSeparator();
    help = menuBar()->addMenu( QObject::tr("MNU_HELP") );
    help->addAction( helpAboutAction );

    myStdActions.insert( FilePreferencesId, FilePreferencesAction );

    // populate a tool bar with some actions
    myStdToolBar = addToolBar( tr( "File Operations" ) );
    myStdToolBar->addAction( fileNewAction );
    myStdToolBar->addAction( fileSaveAction );
    QAction *sliceAction = new QAction(QPixmap(dir+"slice.png"),"slice");
    connect( sliceAction, SIGNAL( triggered() ) , this, SLOT( onSlice() ) );
    myStdActions.insert( FileSliceId, sliceAction );
    myStdToolBar->addAction( sliceAction );
    sliceAction->setVisible(false);

    myStdActions.at(FileCloseId)->setEnabled(myDocuments.count() > 0);

    myStdActions.at(FilePrefUseVBOId)->setEnabled( true );
}

void ApplicationCommonWindow::createCasCadeOperations()
{
    createWindowPopup();
    createPrimitivePopup();

    // populate a tool bar with some actions
    myCasCadeBar = addToolBar( tr( "Shape Operations" ) );

    QString dir = ApplicationCommonWindow::getResourceDir() + QString( "/icon/" );
    QAction* a;

    a = new QAction( QPixmap( dir+QObject::tr("ICON_TOOL_WIRE") ), QObject::tr("MNU_TOOL_WIRE"), this );
    a->setToolTip( QObject::tr("TBR_TOOL_WIRE") );
    a->setStatusTip( QObject::tr("TBR_TOOL_WIRE") );
    connect( a, SIGNAL( triggered() ) , this, SLOT( onToolAction() ) );
    myToolActions.insert( ToolWireframeId, a );

    a = new QAction( QPixmap( dir+QObject::tr("ICON_TOOL_SHAD") ), QObject::tr("MNU_TOOL_SHAD"), this );
    a->setToolTip( QObject::tr("TBR_TOOL_SHAD") );
    a->setStatusTip( QObject::tr("TBR_TOOL_SHAD") );
    connect( a, SIGNAL( triggered() ) , this, SLOT( onToolAction() ) );
    myToolActions.insert( ToolShadingId, a );

    a = new QAction( QPixmap( dir+QObject::tr("ICON_TOOL_COLOR") ), QObject::tr("MNU_TOOL_COLOR"), this );
    a->setToolTip( QObject::tr("TBR_TOOL_COLOR") );
    a->setStatusTip( QObject::tr("TBR_TOOL_COLOR") );
    connect( a, SIGNAL( triggered() ) , this, SLOT( onToolAction() ) );
    myToolActions.insert( ToolColorId, a );

    a = new QAction( QPixmap( dir+QObject::tr("ICON_TOOL_MATER") ), QObject::tr("MNU_TOOL_MATER"), this );
    a->setToolTip( QObject::tr("TBR_TOOL_MATER") );
    a->setStatusTip( QObject::tr("TBR_TOOL_MATER") );
    connect( a, SIGNAL( triggered() ) , this, SLOT( onToolAction() ) );
    myToolActions.insert( ToolMaterialId, a );

    a = new QAction( QPixmap( dir+QObject::tr("ICON_TOOL_TRANS") ), QObject::tr("MNU_TOOL_TRANS"), this );
    a->setToolTip( QObject::tr("TBR_TOOL_TRANS") );
    a->setStatusTip( QObject::tr("TBR_TOOL_TRANS") );
    connect( a, SIGNAL( triggered() ) , this, SLOT( onToolAction() ) );
    myToolActions.insert( ToolTransparencyId, a );

    a = new QAction( QPixmap( dir+QObject::tr("ICON_TOOL_DEL") ), QObject::tr("MNU_TOOL_DEL"), this );
    a->setToolTip( QObject::tr("TBR_TOOL_DEL") );
    a->setStatusTip( QObject::tr("TBR_TOOL_DEL") );
    connect( a, SIGNAL( triggered() ) , this, SLOT( onToolAction() ) );
    myToolActions.insert( ToolDeleteId, a );

    QSignalMapper* sm = new QSignalMapper( this );
    connect( sm, SIGNAL( mapped( int ) ), this, SLOT( onSetMaterial( int ) ) );

    a = new QAction( QObject::tr("MNU_BRASS"), this );
    a->setToolTip( QObject::tr("TBR_BRASS") );
    a->setStatusTip( QObject::tr("TBR_BRASS") );
    sm->setMapping( a,(int)Graphic3d_NOM_BRASS );
    connect( a, SIGNAL( triggered() ), sm, SLOT( map() ) );
    myMaterialActions.insert( Graphic3d_NOM_BRASS, a );

    a = new QAction( QObject::tr("MNU_BRONZE"), this );
    a->setToolTip( QObject::tr("TBR_BRONZE") );
    a->setStatusTip( QObject::tr("TBR_BRONZE") );
    sm->setMapping( a, ( int )Graphic3d_NOM_BRONZE );
    connect( a, SIGNAL( triggered() ), sm, SLOT( map() ) );
    myMaterialActions.insert( Graphic3d_NOM_BRONZE, a );

    a = new QAction( QObject::tr("MNU_COPPER"), this );
    a->setToolTip( QObject::tr("TBR_COPPER") );
    a->setStatusTip( QObject::tr("TBR_COPER") );
    sm->setMapping( a, ( int )Graphic3d_NOM_COPPER );
    connect( a, SIGNAL( triggered() ), sm, SLOT( map() ) );
    myMaterialActions.insert( Graphic3d_NOM_COPPER, a );

    a = new QAction( QObject::tr("MNU_GOLD"), this );
    a->setToolTip( QObject::tr("TBR_GOLD") );
    a->setStatusTip( QObject::tr("TBR_GOLD") );
    sm->setMapping( a, ( int )Graphic3d_NOM_GOLD );
    connect( a, SIGNAL( triggered() ), sm, SLOT( map() ) );
    myMaterialActions.insert( Graphic3d_NOM_GOLD, a );

    a = new QAction( QObject::tr("MNU_PEWTER"), this );
    a->setToolTip( QObject::tr("TBR_PEWTER") );
    a->setStatusTip( QObject::tr("TBR_PEWTER") );
    sm->setMapping( a, ( int )Graphic3d_NOM_PEWTER );
    connect( a, SIGNAL( triggered() ), sm, SLOT( map() ) );
    myMaterialActions.insert( Graphic3d_NOM_PEWTER, a );

    a = new QAction( QObject::tr("MNU_PLASTER"), this );
    a->setToolTip( QObject::tr("TBR_PLASTER") );
    a->setStatusTip( QObject::tr("TBR_PLASTER") );
    sm->setMapping( a, ( int )Graphic3d_NOM_PLASTER );
    connect( a, SIGNAL( triggered() ), sm, SLOT( map() ) );
    myMaterialActions.insert( Graphic3d_NOM_PLASTER, a );

    a = new QAction( QObject::tr("MNU_PLASTIC"), this );
    a->setToolTip( QObject::tr("TBR_PLASTIC") );
    a->setStatusTip( QObject::tr("TBR_PLASTIC") );
    sm->setMapping( a, ( int )Graphic3d_NOM_PLASTIC );
    connect( a, SIGNAL( triggered() ), sm, SLOT( map() ) );
    myMaterialActions.insert( Graphic3d_NOM_PLASTIC, a );

    a = new QAction( QObject::tr("MNU_SILVER"), this );
    a->setToolTip( QObject::tr("TBR_SILVER") );
    a->setStatusTip( QObject::tr("TBR_SILVER") );
    sm->setMapping( a, ( int )Graphic3d_NOM_SILVER );
    connect( a, SIGNAL( triggered() ), sm, SLOT( map() ) );
    myMaterialActions.insert( Graphic3d_NOM_SILVER, a );

    for ( int i = 0; i < myToolActions.size(); i++ )
        myCasCadeBar->addAction( myToolActions.at( i ) );
    myCasCadeBar->hide();
}

void ApplicationCommonWindow::updateTreeWidgetItem()
{
    treewidget->clear();
    for (int i = 0; i < myDocuments.count(); ++i)
    {
        QTreeWidgetItem* group=new QTreeWidgetItem(treewidget);
        group->setText(0,"project-"+QString::number(myDocuments.at(i)->getmyIndex()));
        group->setData(0,Qt::UserRole,i);
        group->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);

        QVector<Component*> myComponents = myDocuments.at(i)->myComponents;

        for(int j=0;j<myComponents.count();j++)
        {
            QTreeWidgetItem *item=new QTreeWidgetItem(group);
            item->setText(0, myComponents.at(j)->getName());
            item->setData(0,Qt::UserRole,j);
            item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsSelectable);
            item->setCheckState(0,Qt::Unchecked);
        }
    }
    treewidget->expandAll();
}

QList<QAction*>* ApplicationCommonWindow::getToolActions()
{
    return &myToolActions;
}

QList<QAction*>* ApplicationCommonWindow::getMaterialActions()
{
    return &myMaterialActions;
}

void ApplicationCommonWindow::createWindowPopup()
{
    if ( !myWindowPopup )
    {
        myWindowPopup = new QMenu( QObject::tr( "MNU_WINDOW" ), this );
        connect( myWindowPopup, SIGNAL( aboutToShow() ),
                 this, SLOT( windowsMenuAboutToShow() ) );
    }
}

void ApplicationCommonWindow::windowsMenuAboutToShow()
{
    myWindowPopup->clear();
    QAction* a;

    QString dir = getResourceDir() + QString( "/icon/" );

    a = new QAction( QPixmap( dir + QObject::tr( "ICON_WINDOW_NEW3D" ) ), QObject::tr( "MNU_WINDOW_NEW3D" ), this );
    a->setToolTip( QObject::tr( "TBR_WINDOW_NEW3D" ) );
    a->setStatusTip( QObject::tr( "TBR_WINDOW_NEW3D" ) );
    connect( a, SIGNAL( triggered() ), this, SLOT( onCreateNewView() ) );
    myWindowPopup->addAction( a );
    a->setVisible(false);

    a = new QAction( QPixmap( dir + QObject::tr( "ICON_WINDOW_CASCADE" ) ), QObject::tr( "MNU_WINDOW_CASCADE" ), this );
    a->setToolTip( QObject::tr( "TBR_WINDOW_CASCADE" ) );
    a->setStatusTip( QObject::tr( "TBR_WINDOW_CASCADE" ) );
    connect( a, SIGNAL( triggered() ), stWs, SLOT( cascade() ) );
    myWindowPopup->addAction( a );

    a = new QAction( QPixmap( dir + QObject::tr( "ICON_WINDOW_TILE" ) ), QObject::tr( "MNU_WINDOW_TILE" ), this );
    a->setToolTip( QObject::tr( "TBR_WINDOW_TILE" ) );
    a->setStatusTip( QObject::tr( "TBR_WINDOW_TILE" ) );
    connect( a, SIGNAL( triggered() ), stWs, SLOT( tile() ) );
    myWindowPopup->addAction( a );

    myWindowPopup->addSeparator();
    QList<QMdiSubWindow *> windows = stWs->subWindowList();
    for (int i = 0; i < windows.count(); ++i)
    {
        QAction* aAction = new QAction( windows.at(i)->windowTitle(), this );
        aAction->setCheckable( true );
        aAction->setData( i );
        myWindowPopup->addAction( aAction );
        connect( aAction, SIGNAL( toggled( bool ) ), this, SLOT( windowsMenuActivated( bool ) ) );
        aAction->setChecked( stWs->activeSubWindow() == windows.at(i) );
    }
}

void ApplicationCommonWindow::createPrimitivePopup()
{
    if ( !myPrimitivePopup )
    {
        myPrimitivePopup = new QMenu("Primitive", this );
    }
}

void ApplicationCommonWindow::windowsMenuActivated( bool checked )
{
    QAction* aSender = qobject_cast<QAction*>( sender() );
    if ( !aSender )
        return;
    QWidget * w = stWs->subWindowList().at( aSender->data().toInt() );
    if ( w && checked )
        w->setFocus();
}

QMdiArea * ApplicationCommonWindow::getWorkspace()
{
    return stWs;
}

ApplicationCommonWindow* ApplicationCommonWindow::getApplication()
{
    return stApp;
}

void ApplicationCommonWindow::updateFileActions()
{
    if (!myDocuments.isEmpty())
    {
        return;
    }

    if ( !myIsDocuments )
    {
        QAction* fileQuitAction = NULL;
        QAction* windowAction = NULL;
        QList<QAction *> aListActions = myFilePopup->actions();
        for ( int i = 0; i < aListActions.size(); i++ )
        {
            if( aListActions.at( i )->text() == QObject::tr("MNU_QUIT") )
            {
                fileQuitAction = aListActions.at( i );
                break;
            }
        }
        
        if( !fileQuitAction )
            return;

        myIsDocuments = true;
        myCasCadeBar->show();

        QList<QAction *> aListMenuActions = menuBar()->actions();
        for ( int i = 0; i < aListMenuActions.size(); i++ )
        {
            if( aListMenuActions.at( i )->text() == QObject::tr("MNU_HELP") )
            {
                windowAction= aListMenuActions.at( i );
                break;
            }
        }

        if( !windowAction )
            return;

        menuBar()->insertMenu( windowAction, myPrimitivePopup );
        menuBar()->insertMenu( windowAction, myWindowPopup );
        myStdActions[FileSliceId]->setVisible(true);
    }
    else
    {
        myIsDocuments = false;
        myCasCadeBar->hide();
        menuBar()->removeAction( myWindowPopup->menuAction() );
        menuBar()->removeAction( myPrimitivePopup->menuAction() );
        myStdActions[FileSliceId]->setVisible(false);
    }
}

DocumentCommon* ApplicationCommonWindow::createNewDocument()
{
    return new DocumentCommon( ++myNbDocuments, this );
}

int& ApplicationCommonWindow::getNbDocument()
{
    return myNbDocuments;
}

DocumentCommon* ApplicationCommonWindow::onNewDoc()
{
    updateFileActions();
    DocumentCommon* aDoc = createNewDocument();
    aDoc->onCreateNewView();
    onSelectionChanged();

    connect (aDoc, SIGNAL (sendCloseDocument (DocumentCommon*) ),
             this, SLOT (onCloseDocument (DocumentCommon*)));
    connect (stWs, SIGNAL (windowActivated (QWidget*)),
             this, SLOT (onWindowActivated (QWidget*)));
    connect (aDoc, SIGNAL (selectionChanged()),
             this, SLOT (onSelectionChanged()));
    mysubWindows.resize(aDoc->getmyIndex());
    QVector<int> v(1,0);
    v[0] = stWs->subWindowList().size()-1;
    mysubWindows[aDoc->getmyIndex()-1]=v;
    myDocuments.append (aDoc);
    myStdActions.at (FileCloseId)->setEnabled (myDocuments.count() > 0);
    updateTreeWidgetItem();
    return aDoc;
}

void ApplicationCommonWindow::onCloseWindow()
{
    stWs->closeAllSubWindows();
    myStdActions.at(FileNewId)->setDisabled(false);
    myStdActions.at(FileCloseId)->setDisabled(true);
    updateTreeWidgetItem();
    mysubWindows.clear();
}

void ApplicationCommonWindow::onUseVBO()
{
    MDIWindow* aWindow = qobject_cast<MDIWindow*> (stWs->activeSubWindow()->widget());
    
    if (NULL == aWindow)
        return;

    Handle(AIS_InteractiveContext) aContextAIS = aWindow->getDocument()->getContext();

    if (aContextAIS.IsNull())
        return;

    Handle(OpenGl_GraphicDriver) aDriver =
            Handle(OpenGl_GraphicDriver)::DownCast (aContextAIS->CurrentViewer()->Driver());

    if (!aDriver.IsNull())
    {
        aDriver->ChangeOptions().vboDisable = Standard_True;
    }
}

void ApplicationCommonWindow::onCloseDocument(DocumentCommon* theDoc)
{
    myDocuments.removeAll( theDoc );
    theDoc->removeViews();
    delete theDoc;
    updateFileActions();
    myStdActions.at(FileCloseId)->setEnabled(myDocuments.count() > 0);
    myStdActions.at(FileNewId)->setDisabled(myDocuments.count() > 0);
    updateTreeWidgetItem();
}

void ApplicationCommonWindow::onViewToolBar()
{
    if ( !myStdToolBar->isVisible() )
        myStdToolBar->show();
    else
        myStdToolBar->hide();
}

void ApplicationCommonWindow::onViewStatusBar()
{
    if ( !statusBar()->isVisible() )
        statusBar()->show();
    else
        statusBar()->hide();
}

void ApplicationCommonWindow::onAbout()
{
    QMessageBox::information( this, QObject::tr( "TIT_ABOUT" ), QObject::tr( "INF_ABOUT" ), QObject::tr("BTN_OK" ),
                              QString::null, QString::null, 0, 0 );
}

void ApplicationCommonWindow::onCreateNewView()
{
    MDIWindow* window = qobject_cast< MDIWindow* >( stWs->activeSubWindow()->widget() );
    window->getDocument()->onCreateNewView();
    mysubWindows[window->getDocument()->getmyIndex()-1].append(stWs->subWindowList().size()-1);
    updateTreeWidgetItem();
}

void ApplicationCommonWindow::onWindowActivated ( QWidget * w )
{
    if (w == NULL)
    {
        return;
    }

    MDIWindow* window = qobject_cast< MDIWindow* >(w);

    window->onWindowActivated();
}

void ApplicationCommonWindow::onToolAction()
{
    QAction* sentBy = (QAction*) sender();
    QMdiArea* ws = ApplicationCommonWindow::getWorkspace();
    DocumentCommon* doc = qobject_cast<MDIWindow*>( ws->activeSubWindow()->widget() )->getDocument();

    if( sentBy == myToolActions.at( ToolWireframeId ) )
        doc->onWireframe();

    if( sentBy == myToolActions.at( ToolShadingId ) )
        doc->onShading();

    if( sentBy == myToolActions.at( ToolColorId ) )
        doc->onColor();

    if( sentBy == myToolActions.at( ToolMaterialId ) )
        doc->onMaterial();

    if( sentBy == myToolActions.at( ToolTransparencyId ) )
        doc->onTransparency();

    if( sentBy == myToolActions.at( ToolDeleteId ) )
        doc->onDelete();
}

void ApplicationCommonWindow::onSelectionChanged()
{
    updateTreeWidgetItem();
    QMdiArea* ws = ApplicationCommonWindow::getWorkspace();
    DocumentCommon* doc;

    if( !qobject_cast<MDIWindow*>( ws->activeSubWindow()->widget() ) )
        return;

    doc = ( qobject_cast<MDIWindow*>( ws->activeSubWindow()->widget() ) )->getDocument();
    Handle(AIS_InteractiveContext) context = doc->getContext();

    //遍历treeWidget
    QTreeWidgetItemIterator it(treewidget);
    while (*it) {
        (*it)->setSelected(false);
        (*it)->setCheckState(0,Qt::Unchecked);
        ++it;
    }

    bool OneOrMoreInShading = false;
    bool OneOrMoreInWireframe = false;
    int numSel = context->NbSelected();
    if ( numSel )
    {
        for ( context->InitSelected(); context->MoreSelected(); context->NextSelected() )
        {
            if ( context->IsDisplayed( context->SelectedInteractive(), 1 ) )
                OneOrMoreInShading = true;
            if ( context->IsDisplayed( context->SelectedInteractive(), 0 ) )
                OneOrMoreInWireframe = true;

            Handle(AIS_InteractiveObject) obj = context->SelectedInteractive();
            TopoDS_Shape shape = Handle(AIS_Shape)::DownCast(obj)->Shape();
            //遍历treeWidget
            QTreeWidgetItemIterator it(treewidget);
            while (*it) {
                int j = (*it)->data(0,Qt::UserRole).toInt();
                if(doc->myComponents[j]->getShape() == shape)
                {
                    (*it)->setSelected(true);
                    (*it)->setCheckState(0,Qt::Checked);
                }
                ++it;
            }
        }
        myToolActions.at( ToolWireframeId )->setEnabled( OneOrMoreInShading );
        myToolActions.at( ToolShadingId )->setEnabled( OneOrMoreInWireframe );
        myToolActions.at( ToolColorId )->setEnabled( true );
        myToolActions.at( ToolMaterialId )->setEnabled( true );
        myToolActions.at( ToolTransparencyId )->setEnabled( OneOrMoreInShading );
        myToolActions.at( ToolDeleteId )->setEnabled( true );
    }
    else
    {
        myToolActions.at( ToolWireframeId )->setEnabled( false );
        myToolActions.at( ToolShadingId )->setEnabled( false );
        myToolActions.at( ToolColorId )->setEnabled( false );
        myToolActions.at( ToolMaterialId )->setEnabled( false );
        myToolActions.at( ToolTransparencyId )->setEnabled( false );
        myToolActions.at( ToolDeleteId )->setEnabled( false );
    }
}

void ApplicationCommonWindow::onSetMaterial( int theMaterial )
{
    QMdiArea* ws = getWorkspace();
    DocumentCommon* doc = qobject_cast<MDIWindow*>( ws->activeSubWindow()->widget() )->getDocument();
    doc->onMaterial( theMaterial );
}

QString ApplicationCommonWindow::getResourceDir()
{
    static QString aResourceDir =
            QString (OSD_Environment ("CSF_ResourcesDefaults").Value().ToCString());
    if (aResourceDir.isEmpty())
    {
        //aResourceDir = QString (OSD_Environment ("CSF_OCCTResourcePath").Value().ToCString()) + "/samples";
        aResourceDir = QCoreApplication::applicationDirPath()+"/resources";
    }
    return aResourceDir;
}

void ApplicationCommonWindow::resizeEvent( QResizeEvent* e )
{
    QMainWindow::resizeEvent( e );
    statusBar()->setSizeGripEnabled( !isMaximized() );
}

bool ApplicationCommonWindow::isDocument()
{
    return myIsDocuments;
}

QMenu* ApplicationCommonWindow::getFilePopup()
{
    return myFilePopup;
}
QMenu* ApplicationCommonWindow::getPrimitivePopup()
{
    return myPrimitivePopup;
}
QAction* ApplicationCommonWindow::getFileSeparator()
{
    return myFileSeparator;
}

QToolBar* ApplicationCommonWindow::getCasCadeBar()
{
    return myCasCadeBar;
}

void ApplicationCommonWindow::onshowSelectedItem(QTreeWidgetItem* item,int)
{
    int i=-1,j=-1;
    QTreeWidgetItem* parent=item->parent();
    if(parent==nullptr)
    {
        i = item->data(0,Qt::UserRole).toInt();
        j = 0;
    }
    else{
        i = parent->data(0,Qt::UserRole).toInt();
        j = item->data(0,Qt::UserRole).toInt();

    }
    int index = mysubWindows[i][j];
    //BEdt->append(QString::number(i)+":"+QString::number(j)+"-"+QString::number(index)+" is clicked.");
    if(index+1 > stWs->subWindowList().size()) return;
    QWidget * w = stWs->subWindowList().at(index);
    if ( w )w->setFocus();
}

void ApplicationCommonWindow::onSave()
{
    BEdt->append("save is clicked.");
}

void ApplicationCommonWindow::updatemysubWindows(int index)
{
    for(int i=0;i<mysubWindows.size();i++)
    {
        for(int j=0;j<mysubWindows[i].size();j++)
        {
            if(mysubWindows[i][j]<=index)continue;
            mysubWindows[i][j]--;
        }
    }
}

void ApplicationCommonWindow::initStatusBar()
{
    currentTimeLabel = new QLabel;

    //    statusLabel->setFrameShape(QFrame::WinPanel); //设置标签形状
//    statusLabel->setFrameShadow(QFrame::Sunken); //设置标签阴影

    QTimer *timer = new QTimer(this);
    timer->start(500); //每隔500ms发送timeout的信号
    connect(timer, SIGNAL(timeout()),this,SLOT(timeUpdate()));
    statusBar()->addPermanentWidget(currentTimeLabel); //现实永久信息
}

void ApplicationCommonWindow::timeUpdate()
{
    QDateTime current_time = QDateTime::currentDateTime();
    QString timestr = current_time.toString( "yyyy/MM/dd hh:mm:ss"); //设置显示的格式
    currentTimeLabel->setText(timestr); //设置label的文本内容为时间
}

void ApplicationCommonWindow::onSlice(){
    QMdiArea* ws = ApplicationCommonWindow::getWorkspace();
    DocumentCommon* doc;

    if(!qobject_cast<MDIWindow*>( ws->activeSubWindow()->widget() ) )
        return;

    doc = ( qobject_cast<MDIWindow*>( ws->activeSubWindow()->widget() ) )->getDocument();

    Handle(AIS_InteractiveContext) context = doc->getContext();
    int numSel = context->NbSelected();
    if ( numSel )
    {
        Handle(TopTools_HSequenceOfShape) aSequence=new TopTools_HSequenceOfShape();

        for ( context->InitSelected(); context->MoreSelected(); context->NextSelected() )
        {
            Handle(AIS_InteractiveObject) obj = context->SelectedInteractive();
            TopoDS_Shape shape = Handle(AIS_Shape)::DownCast(obj)->Shape();
            aSequence->Append( shape );
        }

        Slice *slice = new Slice(this);
        slice->setData(aSequence,doc);
        slice->show();
    }
    else {
        QMessageBox::warning(this, "warning", "please select a model", "OK",
                                  QString::null, QString::null, 0, 0 );
    }
     BEdt->append("slice is clicked.");
}
