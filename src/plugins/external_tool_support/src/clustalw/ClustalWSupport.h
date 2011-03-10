#ifndef _U2_CLUSTALW_SUPPORT_H
#define _U2_CLUSTALW_SUPPORT_H

#include <U2Core/ExternalToolRegistry.h>
#include <U2View/MSAEditor.h>
#include "utils/ExternalToolSupportAction.h"

#define CLUSTAL_TOOL_NAME "ClustalW"
namespace U2 {

class ClustalWSupport : public ExternalTool {
    Q_OBJECT
public:
    ClustalWSupport(const QString& name, const QString& path = "");
    GObjectViewWindowContext* getViewContext(){ return viewCtx; }

public slots:
    void sl_runWithExtFileSpecify();
private:
    GObjectViewWindowContext* viewCtx;
};

class ClustalWSupportContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    ClustalWSupportContext(QObject* p);

protected slots:
    void sl_align_with_ClustalW ();

protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* view, QMenu* m);
};

class ClustalWSupportAction : public ExternalToolSupprotAction {
        Q_OBJECT
public:
    ClustalWSupportAction(QObject* p, GObjectView* v, const QString& text, int order)
        : ExternalToolSupprotAction(p,v,text,order, QStringList(CLUSTAL_TOOL_NAME)) {}
    MSAEditor*  getMSAEditor() const;

private slots:
    void sl_lockedStateChanged();
};

}//namespace
#endif // _U2_CLUSTALW_SUPPORT_H
