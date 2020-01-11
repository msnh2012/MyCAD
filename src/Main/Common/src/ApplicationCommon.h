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

class ApplicationCommonWindow: public QMainWindow
{
    Q_OBJECT

public:
    enum { FileNewId, FilePrefUseVBOId, FileCloseId, FilePreferencesId, FileQuitId, ViewToolId, ViewStatusId, HelpAboutId,FileSaveId};
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
protected:
    virtual DocumentCommon*          createNewDocument();
    int&                             getNbDocument();
    QTextEdit *BEdt;
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
    void onshowSelectedItem(QTreeWidgetItem*,int);

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
protected:
    QList<DocumentCommon*>          myDocuments;
};

#endif


