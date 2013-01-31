/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "ObjectViewModel.h"

#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/OptionsPanel.h>

#include <QtCore/QFileInfo>
#include <QtGui/QVBoxLayout>

//BUG:535 refactor closing interface.
//Idea: make it QObject and call 'deleteLater' on it

namespace U2 {

const QString GObjectViewState::APP_CLOSING_STATE_NAME("Auto saved");
const GObjectViewFactoryId GObjectViewFactory::SIMPLE_TEXT_FACTORY("SimpleTextView");

void GObjectViewState::setViewName(const QString& newName) {
    // this method is not a real state modification: state caches view name as a reference, but not its internal data
    // it is used only on view renaming 
     viewName = newName; 
}

void GObjectViewState::setStateName(const QString& newName) {
    if (newName == stateName) {
        return;
    }
    stateName = newName;
    emit si_stateModified(this);
}

void GObjectViewState::setStateData(const QVariantMap& data) {
    stateData = data;
    emit si_stateModified(this);
}

void GObjectViewFactoryRegistry::registerGObjectViewFactory(GObjectViewFactory* f) {
    mapping[f->getId()] = f;
}

void GObjectViewFactoryRegistry::unregisterGObjectViewFactory(GObjectViewFactory* f) {
    assert(GObjectViewUtils::findViewsByFactoryId(f->getId()).isEmpty());
    mapping.remove(f->getId());
}

GObjectViewFactory* GObjectViewFactoryRegistry::getFactoryById(GObjectViewFactoryId t) const {
    return mapping.value(t, NULL);
}

//////////////////////////////////////////////////////////////////////////
/// GObjectView
GObjectView::GObjectView(GObjectViewFactoryId _factoryId, const QString& _viewName, QObject* prnt): QObject(prnt)
{
    factoryId = _factoryId;
    viewName = _viewName;
    widget = NULL;
    optionsPanel = NULL;
    closeInterface = NULL;
    closing = false;

    Project* p = AppContext::getProject();
    assert(p!=NULL);
    connect(p, SIGNAL(si_documentAdded(Document*)), SLOT(sl_onDocumentAdded(Document*)));
    connect(p, SIGNAL(si_documentRemoved(Document*)), SLOT(sl_onDocumentRemoved(Document*)));
    foreach(Document* d, p->getDocuments()) {
        sl_onDocumentAdded(d);
    }
}

bool GObjectView::canAddObject(GObject* obj) {
    if (objects.contains(obj)) {
        return false;
    }
    foreach(GObjectViewObjectHandler* h, objectHandlers) {
        if (h->canHandle(this, obj)) {
            return true;
        }
    }
    return false;
}

QString GObjectView::addObject(GObject* o) {
    assert(!closing);
    if (objects.contains(o)) {
        return tr("Object is already added to view %1!").arg(o->getGObjectName());
    }

    bool canBeAdded = canAddObject(o);
    if (!canBeAdded) {
        foreach(GObjectViewObjectHandler* h, objectHandlers) {
            canBeAdded = h->canHandle(this, o);
            if (canBeAdded) {
                break;
            }
        }
    }

    if (!canBeAdded) {
        return tr("Can't add object: %1").arg(o->getGObjectName());
    }

    objects.push_back(o);
    onObjectAdded(o);
    emit si_objectAdded(this, o);
    return QString::null;
}

void GObjectView::_removeObject(GObject* o) {
    o->disconnect(this);
    int i = objects.removeAll(o);
    assert(i==1); Q_UNUSED(i);
    closing = onObjectRemoved(o) || closing;
    emit si_objectRemoved(this, o);

    if (requiredObjects.contains(o)) {
        closing = true;
    }
}

void GObjectView::removeObject(GObject* o) {
    assert(!closing);
    _removeObject(o);
    if (closing) {
        SAFE_POINT(closeInterface != NULL, "No close interface", );
        closeInterface->closeView();
    }
}

void GObjectView::sl_onObjectRemovedFromDocument(GObject* o) {
    if (objects.contains(o)) {
        _removeObject(o);
        if (closing) {
            SAFE_POINT(closeInterface != NULL, "No close interface", );
            closeInterface->closeView();
        }
    }
}

bool GObjectView::onObjectRemoved(GObject* obj) {
    foreach(GObjectViewObjectHandler* oh, objectHandlers) {
        oh->onObjectRemoved(this, obj);
    }
    return false;
}

void GObjectView::onObjectAdded(GObject* obj) {
    connect(obj, SIGNAL(si_nameChanged(const QString&)), SLOT(sl_onObjectNameChanged(const QString&)));
    foreach(GObjectViewObjectHandler* oh, objectHandlers) {
        oh->onObjectAdded(this, obj);
    }
}



void GObjectView::sl_onDocumentAdded(Document* d) {
    connect(d, SIGNAL(si_objectRemoved(GObject*)), SLOT(sl_onObjectRemovedFromDocument(GObject*)));
    connect(d, SIGNAL(si_loadedStateChanged()), SLOT(sl_onDocumentLoadedStateChanged()));
}

void GObjectView::sl_onDocumentRemoved(Document* d) {
    if (closing) {
        return;
    }
    d->disconnect(this);
    foreach(GObject* o, d->getObjects()) {
        if (objects.contains(o)) {
            _removeObject(o);
        }
        if (closing) {
            SAFE_POINT(closeInterface != NULL, "No close interface", );
            closeInterface->closeView();
            break;
        }
    }
}

void GObjectView::sl_onObjectNameChanged(const QString& oldName) {
    Project* p = AppContext::getProject();
    if (p == NULL) {
        return;
    }
    GObject* obj = qobject_cast<GObject*>(sender());
    SAFE_POINT(obj != NULL, "Can't locate renamed object!",);
    onObjectRenamed(obj, oldName);
}

QWidget* GObjectView::getWidget()  {
    if (widget == NULL) {
        assert(closeInterface!=NULL);
        widget = createWidget();
    }
    return widget;
}

OptionsPanel* GObjectView::getOptionsPanel() {
    return 0;
}

void GObjectView::setClosingInterface(GObjectViewCloseInterface* i) {
    closeInterface = i;
}

void GObjectView::buildStaticToolbar(QToolBar* tb) {
    emit si_buildStaticToolbar(this, tb);
}

void GObjectView::buildStaticMenu(QMenu* m) {
    emit si_buildStaticMenu(this, m);
}


// Returns true if view  contains this object
bool GObjectView::containsObject(GObject* obj) const {
    return objects.contains(obj);
}

// Returns true if view  contains any objects from the document
bool GObjectView::containsDocumentObjects(Document* doc) const {
    bool result = false;
    foreach(GObject* o, doc->getObjects()) {
        if (objects.contains(o)) {
            result = true;
            break;
        }
    }
    return result;
}

void GObjectView::setName(const QString& newName) {
    QString oldName = viewName;
    if (oldName == newName) {
        return;
    }
    viewName = newName;
    emit si_nameChanged(oldName);
}

//////////////////////////////////////////////////////////////////////////
/// GObjectViewWindow

GObjectViewWindow::GObjectViewWindow(GObjectView* v, const QString& _viewName, bool _persistent) 
: MWMDIWindow(_viewName), view(v), persistent(_persistent)
{
    v->setParent(this);
    v->setClosingInterface(this);

    // Initialize the layout of the whole windows
    QHBoxLayout *windowLayout = new QHBoxLayout();
    windowLayout->setContentsMargins(0, 0, 0, 0);
    windowLayout->setSpacing(0);

    // Get the GObject widget and options panel
    QWidget* viewWidget = v->getWidget();
    SAFE_POINT((viewWidget != 0), "Internal error: not initialized GObjectView widget.",);

    OptionsPanel* optionsPanel = v->getOptionsPanel();

    // Initialize the layout of the object part only
    QVBoxLayout *objectLayout = new QVBoxLayout();
    objectLayout->setContentsMargins(0, 0, 0, 0);
    objectLayout->setSpacing(0);

    // Add the widget to the layout and "parent" it
    objectLayout->addWidget(viewWidget);
    
    // Set the layout of the whole window
    windowLayout->addLayout(objectLayout);
    if (optionsPanel != NULL)
    {
        windowLayout->addWidget(optionsPanel->getMainWidget());
    }
    setLayout(windowLayout);

    // Set the icon
    setWindowIcon(viewWidget->windowIcon());
}

void GObjectViewWindow::setPersistent(bool v)  {
    if (persistent==v) {
        return;
    }
    persistent = v;
    emit si_persistentStateChanged(this);
}

void GObjectViewWindow::closeView() {
    AppContext::getMainWindow()->getMDIManager()->closeMDIWindow(this);
}

bool GObjectViewWindow::onCloseEvent() {
    view->saveWidgetState();
    return view->onCloseEvent();
}

GObjectViewFactory* GObjectViewWindow::getViewFactory() const {
    GObjectViewFactory* f = AppContext::getObjectViewFactoryRegistry()->getFactoryById(view->getFactoryId());
    assert(f);
    return f;
}


void GObjectViewWindow::setupMDIToolbar(QToolBar* tb) {
    view->buildStaticToolbar(tb);
}

void GObjectViewWindow::setupViewMenu(QMenu* m) {
    view->buildStaticMenu(m);
}


//////////////////////////////////////////////////////////////////////////
/// Utils

GObjectViewWindow* GObjectViewUtils::findViewByName(const QString& name) {
    QList<MWMDIWindow*> windows = AppContext::getMainWindow()->getMDIManager()->getWindows();
    QList<GObjectViewWindow*> result;
    foreach(MWMDIWindow* w, windows) {
        if (w->windowTitle() == name) {
            GObjectViewWindow* v = qobject_cast<GObjectViewWindow*>(w);
            if (v!=NULL) {
                return v;
            }
        }
    }
    return NULL;
}

QString GObjectViewUtils::genUniqueViewName(const QString& name) {
    assert(!name.isEmpty());
    QSet<QString> usedNames; //set of names is derived from acttive views & saved states
    QList<MWMDIWindow*> windows = AppContext::getMainWindow()->getMDIManager()->getWindows();
    foreach(MWMDIWindow* w, windows) {
        usedNames.insert(w->windowTitle());
    }
    Project* p = AppContext::getProject();
    if (p!=NULL) {
        foreach(const GObjectViewState* s,  p->getGObjectViewStates()) {
            usedNames.insert(s->getViewName());
        }
    }
    QString res = TextUtils::variate(name, " ", usedNames, false, 2);
    return res;
}

QString GObjectViewUtils::genUniqueStateName(const QString& stateName) {
    QSet<QString> existingNames;
    const QList<GObjectViewState*> states = AppContext::getProject()->getGObjectViewStates();
    foreach(GObjectViewState* s, states) {
        existingNames.insert(s->getStateName());
    }
    QString res = TextUtils::variate(stateName, " ", existingNames, false, 2);
    return res;
}

QString GObjectViewUtils::genUniqueViewName(const Document* doc, const GObject* obj) {
    QString objName = obj->getGObjectName();
    const GObjectTypeInfo& ti = GObjectTypes::getTypeInfo(obj->getGObjectType());
    QString objType = " [" + ti.treeSign + "] ";
    QString docSuffix = doc->getURL().lastFileSuffix();
    QString docName = doc->getURL().fileName();
    docName.chop(docSuffix.length() == 0 ? 0 : docSuffix.length() + 1);
    QString viewName = docName + objType + objName;
    return genUniqueViewName(viewName);
}

QList<GObjectViewState*> GObjectViewUtils::findStatesByViewName(const QString& viewName) {
    QList<GObjectViewState*> result;
    if (AppContext::getProject() != NULL) {
        const QList<GObjectViewState*>& states = AppContext::getProject()->getGObjectViewStates();
        foreach(GObjectViewState* s, states) {
            if (s->getViewName() == viewName) {
                result.push_back(s);
            }
        }
    }
    return result;
}

GObjectViewState* GObjectViewUtils::findStateByName(const QString& viewName, const QString& stateName) {
    Project* p = AppContext::getProject();
    assert(p);
    const QList<GObjectViewState*>& allStates = p->getGObjectViewStates();
    return findStateInList(viewName, stateName, allStates);
}

GObjectViewState* GObjectViewUtils::findStateInList(const QString& viewName, const QString& stateName, const QList<GObjectViewState*>& states) {
    foreach(GObjectViewState* s , states) {
        if (s->getViewName() == viewName && s->getStateName() == stateName) {
            return s;
        }
    }
    return NULL;
}


QList<GObjectViewWindow*> GObjectViewUtils::getAllActiveViews() {
    QList<MWMDIWindow*> windows = AppContext::getMainWindow()->getMDIManager()->getWindows();
    QList<GObjectViewWindow*> result;
    foreach(MWMDIWindow* w, windows) {
        GObjectViewWindow* ov = qobject_cast<GObjectViewWindow*>(w);
        if (ov!=NULL) {
            result.append(ov);
        }
    }
    return result;
}


QList<GObjectViewWindow*> GObjectViewUtils::findViewsByFactoryId(GObjectViewFactoryId id) {
    QList<GObjectViewWindow*> result;
    MainWindow* mw = AppContext::getMainWindow();
    if (mw == NULL || mw->getMDIManager() == NULL) {
        return result; //MW is closed
    }
    QList<MWMDIWindow*> windows = mw->getMDIManager()->getWindows();
    foreach(MWMDIWindow* w, windows) {
        GObjectViewWindow* ov = qobject_cast<GObjectViewWindow*>(w);
        if (ov!=NULL && ov->getViewFactoryId() == id) {
            result.push_back(ov);
        }
    }
    return result;
}

QList<GObjectViewState*> GObjectViewUtils::selectStates(const MultiGSelection& ms, const QList<GObjectViewState*>& states) {
    QList<GObjectViewFactory*> fs = AppContext::getObjectViewFactoryRegistry()->getAllFactories();

    QList<GObjectViewState*> result;
    foreach(GObjectViewFactory* f, fs) {
        QList<GObjectViewState*> fResult = selectStates(f, ms, states);
        result+=fResult;
    }
    return result;
}


QList<GObjectViewState*> GObjectViewUtils::selectStates(GObjectViewFactory* f, const MultiGSelection& ms, const QList<GObjectViewState*>& states)
{
    QList<GObjectViewState*> result;
    foreach (GObjectViewState* s, states) {
        if (s->getViewFactoryId() == f->getId()) {
            if (f->isStateInSelection(ms, s->getStateData())) {
                result.append(s);
            }
        } 
    }
    return result;
}



QList<GObjectViewWindow*> GObjectViewUtils::findViewsWithObject(GObject* obj) {
    QList<GObjectViewWindow*> res;
    QList<GObjectViewWindow*> activeViews = getAllActiveViews();
    foreach(GObjectViewWindow* vw, activeViews) {
        if (vw->getObjects().contains(obj)) {
            res.append(vw);
        }
    }
    return res;
}

QList<GObjectViewWindow*> GObjectViewUtils::findViewsWithAnyOfObjects(const QList<GObject*>& objs) {
    QList<GObjectViewWindow*> res;
    foreach(GObject* obj, objs) {
        QList<GObjectViewWindow*> tmp = findViewsWithObject(obj);
        foreach(GObjectViewWindow* vw, tmp) {
            if(!res.contains(vw)) {
                res+=tmp;
            }
        }  
    }
    return res;
}


GObjectViewWindow* GObjectViewUtils::getActiveObjectViewWindow() {
    MWMDIWindow* w = AppContext::getMainWindow()->getMDIManager()->getActiveWindow();
    return qobject_cast<GObjectViewWindow*>(w);
}

//////////////////////////////////////////////////////////////////////////
// GObjectViewWindowContext

GObjectViewWindowContext::GObjectViewWindowContext(QObject* p, const GObjectViewFactoryId& _id) : 
QObject(p) , id(_id), initialzed(false) {}

void GObjectViewWindowContext::init() {
    initialzed = true;
    MWMDIManager* mdi = AppContext::getMainWindow()->getMDIManager();
    connect(mdi, SIGNAL(si_windowAdded(MWMDIWindow*)), SLOT(sl_windowAdded(MWMDIWindow*)));
    connect(mdi, SIGNAL(si_windowClosing(MWMDIWindow*)), SLOT(sl_windowClosing(MWMDIWindow*)));
    foreach(MWMDIWindow* w, mdi->getWindows()) {
        sl_windowAdded(w);
    }
}

GObjectViewWindowContext::~GObjectViewWindowContext() {
    assert(initialzed);
    MWMDIManager* mdi = AppContext::getMainWindow()->getMDIManager();
    if (mdi == NULL) { //TODO: disconnect context on view removal and assert (mdi!=NULL) here.
        return;
    }
    foreach(MWMDIWindow* w, mdi->getWindows()) {
        GObjectViewWindow* ow = qobject_cast<GObjectViewWindow*>(w);
        if (ow == NULL || (!id.isEmpty() && ow->getViewFactoryId() != id)) {
            continue;
        }
        GObjectView* v = ow->getObjectView();
        disconnectView(v);
    }
}

void GObjectViewWindowContext::sl_windowAdded(MWMDIWindow* w) {
    GObjectViewWindow* ow = qobject_cast<GObjectViewWindow*>(w);
    if (ow == NULL || (!id.isEmpty() && ow->getViewFactoryId() != id)) {
        return;
    }
    GObjectView* v = ow->getObjectView();
    assert(!viewResources.contains(v));

    v->addObjectHandler(this);

    initViewContext(v);

    connect(v, SIGNAL(si_buildPopupMenu(GObjectView*, QMenu*)), SLOT(sl_buildContextMenu(GObjectView*, QMenu*)));
    connect(v, SIGNAL(si_buildStaticMenu(GObjectView*, QMenu*)), SLOT(sl_buildStaticMenu(GObjectView*, QMenu*)));
}

void GObjectViewWindowContext::sl_windowClosing(MWMDIWindow* w) {
    GObjectViewWindow* ow = qobject_cast<GObjectViewWindow*>(w);
    if (ow == NULL || (!id.isEmpty() && ow->getViewFactoryId() != id)) {
        return;
    }
    GObjectView* v = ow->getObjectView();
    disconnectView(v);
}

void GObjectViewWindowContext::disconnectView(GObjectView* v) {
    QList<QObject*> resources = viewResources[v];
    foreach(QObject* a, resources) {
        a->deleteLater(); // deliver close signals, save view states first
    }
    viewResources.remove(v);
    v->removeObjectHandler(this);
}

void GObjectViewWindowContext::addViewResource(GObjectView* v, QObject* r) {
    assert(v!=NULL && (!id.isEmpty() || v->getFactoryId() == id));

    QList<QObject*> resources = viewResources[v];
    assert(!resources.contains(r));
    resources.append(r);
    viewResources[v] = resources;
}


QList<GObjectViewAction*> GObjectViewWindowContext::getViewActions(GObjectView* v) const {
    QList<GObjectViewAction*> actions;
    QList<QObject*> resources = viewResources[v];
    foreach(QObject* obj, resources) {
        GObjectViewAction* a = qobject_cast<GObjectViewAction*>(obj);
        if (a!=NULL) {
            actions.append(a);
        }
    }
    return actions;
}

//////////////////////////////////////////////////////////////////////////
// GObjectViewAction
void GObjectViewAction::addToMenuWithOrder(QMenu* menu) {
    foreach(QAction* action, menu->actions()) {
        GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(action);
        if (viewAction != NULL && viewAction->getActionOrder() > actionOrder) {
            menu->insertAction(action, this);
            return;
        }
    }
    menu->addAction(this);
}

}//namespace
