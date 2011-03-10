#include "ProjectTreeItemSelectorDialog.h"
#include "ProjectTreeItemSelectorDialogImpl.h"
#include "ProjectTreeController.h"

#include <U2Core/DocumentModel.h>

#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QSpacerItem>
#include <QtGui/QPushButton>

namespace U2 {

ProjectTreeItemSelectorDialogImpl::ProjectTreeItemSelectorDialogImpl(QWidget* p, const ProjectTreeControllerModeSettings& s) 
: QDialog(p)
{
    setupUi(this);
    controller = new ProjectTreeController(this, treeWidget, s);
    //TODO: fix bug with double clicking on gobject
    //connect(controller, SIGNAL(si_doubleClicked(GObject*)), SLOT(sl_objectClicked(GObject*)));
    acceptByDoubleClick = true;
}

void ProjectTreeItemSelectorDialogImpl::sl_objectClicked(GObject* obj) {

    Document* d = obj->getDocument();
    assert(d != NULL);
    if (!d->isLoaded()) { 
        QAction* loadSelectedDocumentsAction = controller->getLoadSeletectedDocumentsAction();
        assert(loadSelectedDocumentsAction->isEnabled());
        loadSelectedDocumentsAction->trigger();
        return;
    }
   
    if (acceptByDoubleClick) {
         accept();
    }
}

ProjectTreeItemSelectorDialogImpl::~ProjectTreeItemSelectorDialogImpl() {
    delete controller;
}

QList<Document*> ProjectTreeItemSelectorDialog::selectDocuments(const ProjectTreeControllerModeSettings& s, QWidget* p) {
    QList<Document*> res;
    ProjectTreeItemSelectorDialogImpl d(p, s);
    int rc = d.exec();
    if (rc == QDialog::Accepted) {
        const DocumentSelection* ds = d.controller->getDocumentSelection();
        res << ds->getSelectedDocuments();
    }
    return res;    
}

QList<GObject*> ProjectTreeItemSelectorDialog::selectObjects(const ProjectTreeControllerModeSettings& s, QWidget* p) {
    QList<GObject*> res;
    ProjectTreeItemSelectorDialogImpl d(p, s);
    int rc = d.exec();
    if (rc == QDialog::Accepted) {
        const GObjectSelection* os = d.controller->getGObjectSelection();
        res << os->getSelectedObjects();
    }
    return res;
}

}//namespace
