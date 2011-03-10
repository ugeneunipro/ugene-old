#include "DocumentSelection.h"

#include <U2Core/ProjectModel.h>
#include <U2Core/AppContext.h>

namespace U2 {

static QList<Document*> emptyDocs;
void DocumentSelection::clear() {
    QList<Document*> tmpRemoved = selectedDocs;
    selectedDocs.clear();
    if (!tmpRemoved.isEmpty()) {
        emit si_selectionChanged(this, emptyDocs, tmpRemoved);
    }
}

void DocumentSelection::setSelection(const QList<Document*>& docs) {
    if (docs.isEmpty()) {
        clear();
        return;
    }
    if (isEmpty()) {
        addToSelection(docs);
        return;
    }
    QList<Document*> tmpAdded;
    QList<Document*> tmpRemoved;

    foreach(Document* d, docs) {
        if (!selectedDocs.contains(d)) {
            tmpAdded.push_back(d);
        }
    }
    foreach(Document* d, selectedDocs) {
        if (!docs.contains(d)) {
            tmpRemoved.push_back(d);
        }
    }
    foreach(Document* d, tmpRemoved) {
        selectedDocs.removeAll(d);
    }
    foreach(Document* d, tmpAdded) {
        selectedDocs.push_back(d);
    }
    emit si_selectionChanged(this, tmpAdded, tmpRemoved);
}


void DocumentSelection::addToSelection(const QList<Document*>& docs) {
    QList<Document*> tmpAdded;
    int sBefore = selectedDocs.size();
    foreach(Document* d, docs) {
        if (!selectedDocs.contains(d)) {
            tmpAdded.push_back(d);
            selectedDocs.push_back(d);
        }
    }
    if (selectedDocs.size()!=sBefore) {
        emit si_selectionChanged(this, tmpAdded, emptyDocs);
    }
}

void DocumentSelection::removeFromSelection(const QList<Document*>& docs) {
    QList<Document*> tmpRemoved;
    int sBefore = selectedDocs.size();
    foreach(Document* d, docs) {
        if (selectedDocs.removeAll(d)!=0) {
            tmpRemoved.push_back(d);
        }
    }
    if (selectedDocs.size()!=sBefore) {
        emit si_selectionChanged(this, emptyDocs, tmpRemoved);
    }
}


/*
ProjectDocumentSelection::ProjectDocumentSelection(QObject* parent) : DocumentSelection(parent) {
    Project* p = AppContext::getProject();
    connect(p, SIGNAL(si_documentRemoved(Document*)), SLOT(sl_onDocumentRemoved(Document*)));
}

void ProjectDocumentSelection::sl_onDocumentRemoved(Document* d) {
    removeFromSelection(QList<Document*>()<<d);
}*/

}//namespace
