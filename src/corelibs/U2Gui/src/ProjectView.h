#ifndef _U2_PROJECT_VIEW_H_
#define _U2_PROJECT_VIEW_H_

#include <U2Core/PluginModel.h>
#include <U2Core/ServiceTypes.h>
#include <U2Core/SelectionModel.h>
#include <U2Core/DocumentModel.h>

class QMenu;
class QAction;

namespace U2 {

class DocumentSelection;
class GObjectSelection;

class U2GUI_EXPORT ProjectView : public Service {
    Q_OBJECT
public:
    ProjectView(const QString& sname, const QString& sdesc) 
        : Service(Service_ProjectView, sname, sdesc, QList<ServiceType>()<<Service_Project)
    {
    }

    virtual const DocumentSelection* getDocumentSelection() const = 0;

    virtual const GObjectSelection* getGObjectSelection() const = 0;

//    virtual QAction* getAddNewDocumentAction() const = 0;

    virtual QAction* getAddExistingDocumentAction() const = 0;

    virtual void highlightItem(Document*) = 0;

signals:
    void si_onDocTreePopupMenuRequested(QMenu& m);
};

}//namespace
#endif
