#ifndef _U2_TREE_VIEWER_FACTORY_H_
#define _U2_TREE_VIEWER_FACTORY_H_

#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class MultiGSelection;

class U2VIEW_EXPORT TreeViewerFactory : public GObjectViewFactory {
    Q_OBJECT
public:
    TreeViewerFactory();
    
    virtual bool canCreateView(const MultiGSelection& multiSelection);

    virtual Task* createViewTask(const MultiGSelection& multiSelection, bool single = false);

    virtual bool isStateInSelection(const MultiGSelection& multiSelection, const QVariantMap& stateData);

    virtual Task* createViewTask(const QString& viewName, const QVariantMap& stateData);

    virtual bool supportsSavedStates() const;

    static const GObjectViewFactoryId ID;
};

} // namespace

#endif
