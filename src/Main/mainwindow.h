#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "SARibbonMainWindow.h"
#include <QElapsedTimer>

class SARibbonCategory;
class SARibbonContextCategory;
class QTextEdit;
class QAbstractButton;
class Project;
class MainWindow : public SARibbonMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* par=nullptr);
private slots:
    void onShowContextCategory(bool on);
    void onWpsStyle(bool on);
private:
    void createCategoryMain(SARibbonCategory* page);
    void createCategoryOther(SARibbonCategory* page);
    void createFileMenu(QAbstractButton *menu);
private slots:
    void onMenuButtonPopupCheckableTest(bool b);
    void onInstantPopupCheckableTest(bool b);
    void onDelayedPopupCheckableTest(bool b);

    void onMenuButtonPopupCheckabletriggered(bool b);
    void onInstantPopupCheckabletriggered(bool b);
    void onDelayedPopupCheckabletriggered(bool b);

    void onImporttriggered(bool b);
private:
    SARibbonContextCategory* m_contextCategory;
    QTextEdit* m_edit;

    QElapsedTimer timer;

    Project *project;
};

#endif // MAINWINDOW_H
