#include "ProjectImpl.h"

#include <U2Core/ServiceTypes.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>

#include <U2Core/GHints.h>

#include <U2Core/Log.h>

namespace U2 {

/* TRANSLATOR U2::ProjectImpl */


ProjectImpl::ProjectImpl(const QString& _name, const QString& _url, const QList<Document*>& _docs, const QList<GObjectViewState*>& _states)
: name(_name), url(_url) 
{
    mainThreadModel = true;
    foreach(Document* doc, _docs) {
        addDocument(doc);
    }
    foreach(GObjectViewState* state, _states) {
        addGObjectViewState(state);
    }
    setModified(false);

    resourceTracker = AppContext::getAppSettings()->getAppResourcePool()->getResource(RESOURCE_MEMORY);
}

ProjectImpl::~ProjectImpl() {
	//delete all docs
	while (!docs.isEmpty()) {
		Document* d = docs.takeLast();
		delete d;
	}

	//delete all views
	while (!objectViews.isEmpty()) {
		GObjectViewState* s = objectViews.takeLast();
		delete s;
	}
}

void ProjectImpl::makeClean() {
	if (!isTreeItemModified()) {
		return;
	}
	setModified(false);
	foreach(Document* d, docs) {
		d->makeClean();
	}
	assert(!isTreeItemModified());
}

void ProjectImpl::setProjectName(const QString& newName) {
	if (name==newName) {
		return;
	}
	setModified(true);
	name = newName;
	emit si_projectRenamed(this);
}

void ProjectImpl::setProjectURL(const QString& newURL) {
    if (url == newURL) {
        return;
    }
    
    coreLog.details(tr("Project URL is set to %1\n").arg(newURL));

    setModified(true);
    QString oldURL = url;
    url = newURL;
    emit si_projectURLChanged(oldURL);
}

Document* ProjectImpl::findDocumentByURL(const QString & url) const {
	foreach(Document* d, docs) {
		if (d->getURLString() == url) {
			return d;
		}
	}
	return NULL;
}

void ProjectImpl::addDocument(Document* d) {
    coreLog.details(tr("Adding document to the project: %1").arg(d->getURLString()));

	assert(findDocumentByURL(d->getURL())==NULL);
	setParentStateLockItem_static(d, this);

    d->setGHints(new ModTrackHints(this, d->getGHintsMap(), true));

	docs.push_back(d);

    /*QFileInfo file(d->getURLString());
    qint64 memUseMB = file.size()/(1024*1024);
    resourseUsage[d->getName()] = memUseMB;
    resourceTracker->acquire(memUseMB);*/
	emit si_documentAdded(d);
	setModified(true);
}

bool ProjectImpl::lockResoureces(int sizeMB, const QString & url, QString& error) {
    if(resourceTracker->isAvailable(sizeMB)) {
        Document *doc = findDocumentByURL(url);
        if(doc) { 
            resourceUsage[doc->getName()] = sizeMB;
            resourceTracker->acquire(sizeMB);
        }
        return true;
    }
    else {
        error = tr("Not enough resources for load document, resource name: '%1' available: %2%3 requested: %4%3").
            arg(resourceTracker->name).arg(resourceTracker->maxUse - resourceTracker->currentUse).arg(resourceTracker->suffix).arg(sizeMB);
        return false;
    }
}

void ProjectImpl::removeDocument(Document* d, bool autodelete) {
    coreLog.details(tr("Removing document from the project: %1").arg(d->getURLString()));

	setParentStateLockItem_static(d, NULL);
	docs.removeOne(d);

    d->setGHints(new GHintsDefaultImpl(d->getGHints()->getMap()));

	emit si_documentRemoved(d);
	if (autodelete) {
        if(resourceUsage.contains(d->getName())) {
            resourceTracker->release(resourceUsage[d->getName()]);
            resourceUsage.remove(d->getName());
        }
		delete d;
	}
    setModified(true);
} 


void ProjectImpl::sl_onStateModified(GObjectViewState*) {
	setModified(true);
}

void ProjectImpl::addState(GObjectViewState* s) {
	assert(!objectViews.contains(s));
	connect(s, SIGNAL(si_stateModified(GObjectViewState*)), SLOT(sl_onStateModified(GObjectViewState*)));
	objectViews.append(s);
	setModified(true);
}

void ProjectImpl::addGObjectViewState(GObjectViewState* s) {
	assert(GObjectViewUtils::findStateInList(s->getViewName(), s->getStateName(), objectViews) == NULL);
	addState(s);
	emit si_objectViewStateAdded(s);
}

void ProjectImpl::removeGObjectViewState(GObjectViewState* s) {
	int i = objectViews.removeAll(s);
    Q_UNUSED(i);
    assert(i == 1);
	emit si_objectViewStateRemoved(s);
	setModified(true);
}

}
