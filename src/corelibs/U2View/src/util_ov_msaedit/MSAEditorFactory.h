#ifndef _U2_MSA_EDITOR_FACTORY_H_
#define _U2_MSA_EDITOR_FACTORY_H_

#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class MultiGSelection;
class MSAColorSchemeRegistry;

class U2VIEW_EXPORT MSAEditorFactory : public GObjectViewFactory {
    Q_OBJECT
public:
    MSAEditorFactory();
    
    virtual bool canCreateView(const MultiGSelection& multiSelection);

    virtual Task* createViewTask(const MultiGSelection& multiSelection, bool single = false);

    virtual bool isStateInSelection(const MultiGSelection& multiSelection, const QVariantMap& stateData);

    virtual Task* createViewTask(const QString& viewName, const QVariantMap& stateData);

    virtual bool supportsSavedStates() const {return true;}

    static const GObjectViewFactoryId ID;
};



} // namespace

#endif
