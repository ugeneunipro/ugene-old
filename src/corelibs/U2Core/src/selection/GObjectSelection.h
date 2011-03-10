#ifndef _U2_GOBJECT_SELECTION_H_
#define _U2_GOBJECT_SELECTION_H_

#include "SelectionTypes.h"

#include <U2Core/SelectionModel.h>

namespace U2 {

class GObject;

class  U2CORE_EXPORT GObjectSelection : public GSelection {
    Q_OBJECT
public:
    GObjectSelection(QObject* p = NULL) : GSelection(GSelectionTypes::GOBJECTS, p) {}

    const QList<GObject*>& getSelectedObjects() const {return selectedObjects;}

    virtual bool isEmpty() const {return selectedObjects.isEmpty();}

    virtual void clear();

    void setSelection(const QList<GObject*>& objs);

    void addToSelection(const QList<GObject*>& obj);

    void addToSelection(GObject* obj);

    void removeFromSelection(GObject* obj);

    void removeFromSelection(const QList<GObject*>& obj);

    bool contains(GObject* obj) const {return selectedObjects.contains(obj);}

signals:
    void si_selectionChanged(GObjectSelection* thiz, const QList<GObject*>& objectsAdded, const QList<GObject*>& objectsRemoved);

protected:
    virtual void _append(GObject* o) {selectedObjects.append(o);}
    virtual void _remove(GObject* o) {selectedObjects.removeAll(o);};

private:
    QList<GObject*> selectedObjects;
};



/*
class Document;
class GDocumentObjectSelection : public GObjectSelection {
    Q_OBJECT
public:
    GDocumentObjectSelection(QObject* p = NULL) : GObjectSelection(p){};

protected:
    virtual void _append(GObject* o);
    virtual void _remove(GObject* o);

private slots:
    void sl_onObjectRemoved(Document* d, GObject* o);
};


class  GProjectObjectSelection : public GObjectSelection {
    Q_OBJECT
public:
    GProjectObjectSelection(QObject* p = NULL);

private slots:
    void sl_onDocumentRemoved(Document* d);
};
*/


} //namespace
#endif
