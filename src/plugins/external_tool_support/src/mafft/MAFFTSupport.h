#ifndef _U2_MAFFT_SUPPORT_H
#define _U2_MAFFT_SUPPORT_H

#include <U2Core/ExternalToolRegistry.h>
#include <U2View/MSAEditor.h>
#include "utils/ExternalToolSupportAction.h"

#define MAFFT_TOOL_NAME "MAFFT"

namespace U2 {

class MAFFTSupport : public ExternalTool {
    Q_OBJECT
public:
    MAFFTSupport(const QString& name, const QString& path = "");
    GObjectViewWindowContext* getViewContext(){ return viewCtx; }
public slots:
    void sl_runWithExtFileSpecify();
private:
    GObjectViewWindowContext* viewCtx;
};

class MAFFTSupportContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    MAFFTSupportContext(QObject* p);

protected slots:
    void sl_align_with_MAFFT();

protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* view, QMenu* m);
};

class MAFFTSupportAction : public ExternalToolSupprotAction {
        Q_OBJECT
public:
    MAFFTSupportAction(QObject* p, GObjectView* v, const QString& text, int order)
        : ExternalToolSupprotAction(p,v,text,order, QStringList(MAFFT_TOOL_NAME)) {}
    MSAEditor*  getMSAEditor() const;

private slots:
    void sl_lockedStateChanged();
};

}//namespace
#endif // _U2_MAFFT_SUPPORT_H
