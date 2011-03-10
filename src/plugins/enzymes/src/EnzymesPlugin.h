#ifndef _U2_ENZYMES_PLUGIN_H_
#define _U2_ENZYMES_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class EnzymesADVContext;

class EnzymesPlugin : public Plugin  {
    Q_OBJECT
public:
    EnzymesPlugin();
    ~EnzymesPlugin();
    void createToolsMenu();
private slots:
    void sl_onOpenDigestSequenceDialog();
    void sl_onOpenConstructMoleculeDialog();
    void sl_onOpenCreateFragmentDialog();
private:
    EnzymesADVContext*  ctxADV;
    QAction* openDigestSequenceDialog;
    QAction* openConstructMoleculeDialog;
    QAction* openCreateFragmentDialog;
};

class EnzymesADVContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    EnzymesADVContext(QObject* p, const QList<QAction*>& cloningActions);

protected slots:
    void sl_search();

protected:
    virtual void buildMenu(GObjectView* v, QMenu* m);
    virtual void initViewContext(GObjectView* view);
    QList<QAction*> cloningActions;
};

} //namespace

#endif
