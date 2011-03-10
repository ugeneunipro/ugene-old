#include "SimpleTextObjectViewTasks.h"
#include "SimpleTextObjectView.h"

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/TextObject.h>
#include <U2Core/L10n.h>

namespace U2 {


//////////////////////////////////////////////////////////////////////////
//open view task

OpenSimpleTextObjectViewTask::OpenSimpleTextObjectViewTask(Document* _doc) 
: ObjectViewTask(SimpleTextObjectViewFactory::ID), doc(_doc)
{
    if (!doc->isLoaded()) {
        documentsToLoad.append(doc);
    }
}

OpenSavedTextObjectViewTask::OpenSavedTextObjectViewTask(const QString& vname, const QVariantMap& stateData) 
: ObjectViewTask(SimpleTextObjectViewFactory::ID, vname, stateData), doc(NULL)
{
    QString documentUrl = SimpleTextObjectView::getDocumentUrl(stateData);
    doc = AppContext::getProject()->findDocumentByURL(documentUrl);
    if (doc.isNull()) {
        stateIsIllegal = true;
        stateInfo.setError(L10N::errorDocumentNotFound(documentUrl));
        return;
    }
    if (!doc->isLoaded()) {
        documentsToLoad.append(doc);
    }

}

void OpenSavedTextObjectViewTask::open() {
    if (stateInfo.hasErrors() || doc.isNull()) {
        return;
    }
    assert(doc->isLoaded());
    QString objName = SimpleTextObjectView::getObjectName(stateData);
    GObject* obj = doc->findGObjectByName(objName);
    TextObject* to = qobject_cast<TextObject*>(obj);
    if (!to) {
        stateInfo.setError(tr("Text object '%1' is not found").arg(objName));
        stateIsIllegal = true;
        return;
    }
    SimpleTextObjectView* v = new SimpleTextObjectView(viewName, to, stateData);
    GObjectViewWindow* w = new GObjectViewWindow(v, viewName, !stateData.isEmpty());
    MWMDIManager* mdiManager =  AppContext::getMainWindow()->getMDIManager();
    mdiManager->addMDIWindow(w);
}

void OpenSimpleTextObjectViewTask::open() {
    if (stateInfo.hasErrors() || doc.isNull()) {
        return;
    }
    assert(doc->isLoaded());
    foreach(GObject* obj, doc->findGObjectByType(GObjectTypes::TEXT)) {
        TextObject* to = qobject_cast<TextObject*>(obj);
        assert(to);
        QString viewName = GObjectViewUtils::genUniqueViewName(doc, to);
        SimpleTextObjectView* v = new SimpleTextObjectView(viewName, to, stateData);
        GObjectViewWindow* w = new GObjectViewWindow(v, viewName, !stateData.isEmpty());
        MWMDIManager* mdiManager =  AppContext::getMainWindow()->getMDIManager();
        mdiManager->addMDIWindow(w);
    }
}


//////////////////////////////////////////////////////////////////////////
// update view task

UpdateSimpleTextObjectViewTask::UpdateSimpleTextObjectViewTask(GObjectView* v, const QString& stateName, const QVariantMap& stateData) 
: ObjectViewTask(v, stateName, stateData)
{
}

void UpdateSimpleTextObjectViewTask::update() {
    if (view.isNull()) {
        return;
    }
    SimpleTextObjectView* tv = qobject_cast<SimpleTextObjectView*>(view);
    if (tv == NULL) {
        return;
    }
    tv->updateView(stateData);
    
}


} // namespace
