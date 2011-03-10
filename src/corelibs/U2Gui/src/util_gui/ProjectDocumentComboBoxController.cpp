#include "ProjectDocumentComboBoxController.h"

#include <U2Core/ProjectModel.h>

namespace U2 {

ProjectDocumentComboBoxController::ProjectDocumentComboBoxController(Project* _p, QComboBox* _cb, 
																	 QObject* parent, const DocumentFilter* f) 
: QObject(parent), p(_p), cb(_cb), filter(f)
{
    assert(filter!=NULL);
    cb->setInsertPolicy(QComboBox::InsertAlphabetically);

	connect(p, SIGNAL(si_documentAdded(Document*)), SLOT(sl_onDocumentAdded(Document*)));
	connect(p, SIGNAL(si_documentRemoved(Document*)), SLOT(sl_onDocumentRemoved(Document*)));
	foreach(Document* d, p->getDocuments()) {
		if (filter->matches(d)) {
			addDocument(d);
		}
	}
}

ProjectDocumentComboBoxController::~ProjectDocumentComboBoxController() {
}

void ProjectDocumentComboBoxController::sl_onDocumentAdded(Document* d) {
	if (filter->matches(d)) {
		addDocument(d);
	}
}

void ProjectDocumentComboBoxController::sl_onDocumentRemoved(Document* d) {
	removeDocument(d);
}


void ProjectDocumentComboBoxController::addDocument(Document* d) {
	cb->addItem(d->getURLString());
}

void ProjectDocumentComboBoxController::removeDocument(Document* d) {
	int i =cb->findText(d->getURLString());
	if (i >=0 ) {
		cb->removeItem(i);
	}
}

void ProjectDocumentComboBoxController::selectDocument(const QString& url) {
	int i = cb->findText(url);
    if (i >= 0) {
        cb->setCurrentIndex(i);
    }
}

void ProjectDocumentComboBoxController::selectDocument(Document* d) {
	assert(filter->matches(d));
    selectDocument(d->getURLString());
}

Document* ProjectDocumentComboBoxController::getDocument() const {
	QString url = cb->currentText();
	if (url.isEmpty()) {
		return NULL;
	}
	Document* doc = p->findDocumentByURL(url);
	assert(doc);
	return doc;
}

} // namespace
