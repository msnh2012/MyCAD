#ifndef SARIBBONPANNELOPTIONBUTTON_H
#define SARIBBONPANNELOPTIONBUTTON_H
#include <QToolButton>
#include "SARibbonGlobal.h"
class QAction;
class SARibbonPannelOptionButton : public QToolButton
{
    Q_OBJECT
public:
    SARibbonPannelOptionButton(QWidget *parent = Q_NULLPTR);
    //有别于setDefaultAction 此函数只关联action的响应，而不设置text，icon等
    void connectAction(QAction* action);
};

#endif // SAROBBONPANNELOPTIONBUTTON_H
