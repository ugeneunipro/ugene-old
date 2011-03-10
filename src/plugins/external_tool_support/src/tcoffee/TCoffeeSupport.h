#ifndef _U2_TCOFFEE_SUPPORT_H
#define _U2_TCOFFEE_SUPPORT_H

#include <U2Core/ExternalToolRegistry.h>
#include <U2View/MSAEditor.h>
#include "utils/ExternalToolSupportAction.h"

#define TCOFFEE_TOOL_NAME "T-Coffee"

namespace U2 {

class TCoffeeSupport : public ExternalTool {
    Q_OBJECT
public:
    TCoffeeSupport(const QString& name, const QString& path = "");
    GObjectViewWindowContext* getViewContext(){ return viewCtx; }
public slots:
    void sl_runWithExtFileSpecify();
private:
    GObjectViewWindowContext* viewCtx;
};

class TCoffeeSupportContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    TCoffeeSupportContext(QObject* p);

protected slots:
    void sl_align_with_TCoffee();

protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* view, QMenu* m);
};

class TCoffeeSupportAction : public ExternalToolSupprotAction {
        Q_OBJECT
public:
    TCoffeeSupportAction(QObject* p, GObjectView* v, const QString& text, int order)
        : ExternalToolSupprotAction(p,v,text,order, QStringList(TCOFFEE_TOOL_NAME)) {}
    MSAEditor*  getMSAEditor() const;

private slots:
    void sl_lockedStateChanged();
};

}//namespace
#endif // _U2_TCOFFEE_SUPPORT_H
