#ifndef _U2_EXPERT_DIS_PLUGIN_H_
#define _U2_EXPERT_DIS_PLUGIN_H_

#include <U2Core/PluginModel.h>

#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class ExpertDiscoveryPlugin : public Plugin  {
    Q_OBJECT
public:
    ExpertDiscoveryPlugin();

private slots:
    void sl_initExpertDiscoveryViewCtx();
    void sl_expertDiscoveryView();

private:
    GObjectViewWindowContext* viewCtx;
};


class ExpertDiscoveryViewFactory : public GObjectViewFactory{
    Q_OBJECT
public:
    ExpertDiscoveryViewFactory(GObjectViewFactoryId id, const QString& name, QObject* p = NULL);


    virtual bool canCreateView(const MultiGSelection& multiSelection);

    virtual Task* createViewTask(const MultiGSelection& multiSelection, bool single = false);

    //virtual bool isStateInSelection(const MultiGSelection& multiSelection, const QVariantMap& stateData);

    //virtual Task* createViewTask(const QString& viewName, const QVariantMap& stateData);

    virtual bool supportsSavedStates() const {return true;}

    static const GObjectViewFactoryId ID;
private:
    bool checkSelection(const MultiGSelection& multiSelection);

signals:
    void si_newTaskCreation(Task* t);

};
} //namespace

#endif
