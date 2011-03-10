#ifndef _U2_UMUSCLE_PLUGIN_H_
#define _U2_UMUSCLE_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/AppContext.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtGui/QMenu>

namespace U2 {

class MSAEditor;
class MuscleMSAEditorContext;
class XMLTestFactory;

class MusclePlugin : public Plugin {
    Q_OBJECT
public:
    MusclePlugin();
    ~MusclePlugin();

public slots:
    void sl_runWithExtFileSpecify();

private:
    MuscleMSAEditorContext* ctx;
    
};


class MuscleMSAEditorContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    MuscleMSAEditorContext(QObject* p);

protected slots:
    void sl_align();
    void sl_alignSequencesToProfile();
    void sl_alignProfileToProfile();

protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* v, QMenu* m);
};

class MuscleAction : public GObjectViewAction {
    Q_OBJECT
public:
    MuscleAction(QObject* p, GObjectView* v, const QString& text, int order) 
		: GObjectViewAction(p, v, text, order) {}

    MSAEditor*  getMSAEditor() const;

private slots:
    void sl_lockedStateChanged();
};

} //namespace

#endif
