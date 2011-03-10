#ifndef _U2_DOCUMENT_SELECTION_H_
#define _U2_DOCUMENT_SELECTION_H_

#include "SelectionTypes.h"

#include <U2Core/SelectionModel.h>


namespace U2 {

class Document;

class U2CORE_EXPORT DocumentSelection : public GSelection {
    Q_OBJECT
public:
    DocumentSelection(QObject* p = NULL) : GSelection(GSelectionTypes::DOCUMENTS, p) {}

    const QList<Document*>& getSelectedDocuments() const {return selectedDocs;}

    virtual bool isEmpty() const {return selectedDocs.isEmpty();}

    virtual void clear();

    void setSelection(const QList<Document*>& docs);

    void addToSelection(const QList<Document*>& docs);

    void removeFromSelection(const QList<Document*>& docs);

    bool contains(Document* doc) const {return selectedDocs.contains(doc);}

signals:
    void si_selectionChanged(DocumentSelection* thiz, const QList<Document*>& docsAdded, const QList<Document*>& docsRemoved);

private:
    QList<Document*> selectedDocs;
};

/*
class  ProjectDocumentSelection : public DocumentSelection {
    Q_OBJECT

public:
    ProjectDocumentSelection(QObject* parent = NULL);

private slots:
    void sl_onDocumentRemoved(Document*);
};
*/

}//namespace

#endif
