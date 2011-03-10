
#ifndef _U2_UINDEX_VIEWER_FACTORY_H_
#define  _U2_UINDEX_VIEWER_FACTORY_H_

#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class MultiGSelection;

class U2VIEW_EXPORT UIndexViewerFactory : public GObjectViewFactory {
    Q_OBJECT
public:
    UIndexViewerFactory();
    
    virtual bool canCreateView(const MultiGSelection& multiSelection);
    virtual bool isStateInSelection(const MultiGSelection& multiSelection, const QVariantMap& stateData);
    virtual Task* createViewTask(const MultiGSelection& multiSelection, bool single = false);
    virtual Task* createViewTask(const QString& viewName, const QVariantMap& stateData);
    bool supportsSavedStates() const;
    
    static const GObjectViewFactoryId ID;
    
}; // UIndexViewerFactory

} // U2

#endif // _U2_UINDEX_VIEWER_FACTORY_H_
