#ifndef _U2_ANNOTATED_DNA_VIEW_FACTORY_H_
#define _U2_ANNOTATED_DNA_VIEW_FACTORY_H_

#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class MultiGSelection;

class U2VIEW_EXPORT AnnotatedDNAViewFactory : public GObjectViewFactory {
    Q_OBJECT
public:
    AnnotatedDNAViewFactory();

    virtual bool canCreateView(const MultiGSelection& multiSelection);

    virtual Task* createViewTask(const MultiGSelection& multiSelection, bool single = false);

    virtual bool isStateInSelection(const MultiGSelection& multiSelection, const QVariantMap& stateData);

    virtual Task* createViewTask(const QString& viewName, const QVariantMap& stateData);

    virtual bool supportsSavedStates() const {return true;}

    static const GObjectViewFactoryId ID;
};



} // namespace

#endif
