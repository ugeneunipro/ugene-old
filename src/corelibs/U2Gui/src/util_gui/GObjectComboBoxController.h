#ifndef _U2_GOBJECT_COMBOBOX_CONTROLLER_H_
#define _U2_GOBJECT_COMBOBOX_CONTROLLER_H_

#include <U2Core/DocumentModel.h>
#include <U2Core/GObjectReference.h>

#include <QtGui/QComboBox>

namespace U2 {

//TODO: use custom class for objects filtering, share it with ProjectTreeController

class GObjectComboBoxControllerConstraints {
public:
    GObjectComboBoxControllerConstraints() {
        onlyWritable = false;
        uof = UOF_LoadedOnly;
    }

    GObjectType             typeFilter;
    GObjectRelation         relationFilter;
    bool                    onlyWritable;
    UnloadedObjectFilter    uof;
};

class U2GUI_EXPORT GObjectComboBoxController : public QObject {
    Q_OBJECT
public:
    GObjectComboBoxController(QObject* p, const GObjectComboBoxControllerConstraints& c, QComboBox* combo);

    bool setSelectedObject(GObject* obj) {return setSelectedObject(GObjectReference(obj));}
    bool setSelectedObject(const GObjectReference& ref);

    GObject* getSelectedObject() const;

private slots:
    void sl_onDocumentAdded(Document* d);
    void sl_onDocumentRemoved(Document* d);
    void sl_onObjectAdded(GObject* o);
    void sl_onObjectRemoved(GObject* o);
    void sl_lockedStateChanged();

private:
    void updateCombo();
    void addObject(GObject*);
    void removeObject(const GObjectReference& ref);
    void addDocumentObjects(Document* d);
    void removeDocumentObjects(Document* d);
    QString itemText(GObject* o);

    GObjectComboBoxControllerConstraints settings;
    QComboBox*          combo;
    QIcon               objectIcon;
    QIcon               unloadedObjectIcon;
};

}//namespace

#endif
