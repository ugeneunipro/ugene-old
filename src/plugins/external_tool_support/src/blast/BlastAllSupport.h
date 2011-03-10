#ifndef _U2_BLASTALL_SUPPORT_H
#define _U2_BLASTALL_SUPPORT_H

#include <U2Core/ExternalToolRegistry.h>
#include <U2Gui/ObjectViewModel.h>

#define BLASTALL_TOOL_NAME "BlastAll"
namespace U2 {

class BlastAllSupport : public ExternalTool {
    Q_OBJECT
public:
    BlastAllSupport(const QString& name, const QString& path = "");
    GObjectViewWindowContext* getViewContext(){ return viewCtx; }

public slots:
    void sl_runWithExtFileSpecify();
private:
    GObjectViewWindowContext* viewCtx;
    QString lastDBPath;
    QString lastDBName;
};

class BlastAllSupportContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    BlastAllSupportContext(QObject* p);

protected slots:
    void sl_showDialog();

protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* view, QMenu* m);
private:
    QString lastDBPath;
    QString lastDBName;
};

}//namespace
#endif // _U2_BLASTALL_SUPPORT_H
