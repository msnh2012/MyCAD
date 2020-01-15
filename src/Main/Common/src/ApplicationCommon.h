#ifndef APPLICATIONCOMMON_H
#define APPLICATIONCOMMON_H

#include "DocumentCommon.h"

#include <Standard_WarningsDisable.hxx>
#include <QMainWindow>
#include <QAction>
#include <QToolBar>
#include <QMenu>
#include <QMdiArea>
#include <QList>
#include <QTreeWidget>
#include <Standard_WarningsRestore.hxx>
#include <QTextEdit>
#include <QLabel>

class ApplicationCommonWindow: public QMainWindow
{
    Q_OBJECT

public:
    enum { FileNewId, FilePrefUseVBOId, FileCloseId, FilePreferencesId, FileQuitId, ViewToolId, ViewStatusId, HelpAboutId,
           FileSaveId,FileSliceId};
    enum { ToolWireframeId, ToolShadingId, ToolColorId, ToolMaterialId, ToolTransparencyId, ToolDeleteId };

    ApplicationCommonWindow();
    ~ApplicationCommonWindow();

    static QMdiArea*              getWorkspace();
    static ApplicationCommonWindow* getApplication();
    static QString                  getResourceDir();

    virtual void                     updateFileActions();
    QList<QAction*>*                 getToolActions();
    QList<QAction*>*                 getMaterialActions();
    void    updateTreeWidgetItem();
    void    updatemysubWindows(int i);
    QTextEdit *BEdt;
protected:
    virtual DocumentCommon*          createNewDocument();
    int&                             getNbDocument();

public slots:

    DocumentCommon*                 onNewDoc();
    void                            onCloseWindow();
    void                            onUseVBO();
    virtual void                    onCloseDocument( DocumentCommon* theDoc );
    virtual void                    onSelectionChanged();
    virtual void                    onAbout();
    void                            onViewToolBar();
    void                            onViewStatusBar();
    void                            onToolAction();
    void                            onCreateNewView();
    void                            onWindowActivated ( QWidget * w );
    void                            windowsMenuAboutToShow();
    void                            windowsMenuActivated( bool checked/*int id*/ );
    void                            onSetMaterial( int theMaterial );
    void onSave();
    void onSlice();
    void onshowSelectedItem(QTreeWidgetItem*,int);
    void timeUpdate();
protected:
    virtual void                    resizeEvent( QResizeEvent* );
    bool                            isDocument();
    QMenu*                          getFilePopup();
    QMenu*                          getPrimitivePopup();
    QAction*                        getFileSeparator();
    QToolBar*                       getCasCadeBar();

private:
    void                            createStandardOperations();
    void                            createCasCadeOperations();
    void                            createWindowPopup();
    void                            createPrimitivePopup();
    void initStatusBar();
private:
    int                             myNbDocuments;
    bool                            myIsDocuments;

    QList<QAction*>                 myStdActions;
    QList<QAction*>                 myToolActions;
    QList<QAction*>                 myMaterialActions;
    //QList<DocumentCommon*>          myDocuments;

    QToolBar*                       myStdToolBar;
    QToolBar*                       myCasCadeBar;
    QMenu*                          myFilePopup;
    QMenu*                          myWindowPopup;
    QMenu*                          myPrimitivePopup;
    QAction*                        myFileSeparator;
    QTreeWidget *treewidget;
    QVector<QVector<int>> mysubWindows;
    QLabel *currentTimeLabel;
protected:
    QList<DocumentCommon*>          myDocuments;
};

#endif


