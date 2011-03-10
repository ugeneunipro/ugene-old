#ifndef _U2_WORKFLOW_CFG_EDITOR_H_
#define _U2_WORKFLOW_CFG_EDITOR_H_

#include <U2Lang/Attribute.h>

#include <QtGui/QItemDelegate>

class QWidget;

namespace U2 {

class ConfigurationEditor;
class PropertyDelegate;

/**
 * base class for controller of configuration editor
 * editing comes from delegates (see PropertyDelegate)
 */
class U2LANG_EXPORT ConfigurationEditor {
public:
    enum ItemValueRole {
        ItemValueRole = Qt::UserRole + 2,
        ItemListValueRole
    }; // ItemValueRole

public:    
    virtual ~ConfigurationEditor() {}

    // editing widget
    virtual QWidget* getWidget() { return NULL; }
    
    // this controller is a container of delegates
    virtual PropertyDelegate* getDelegate(const QString&) { return NULL; }
    virtual PropertyDelegate* removeDelegate( const QString&) { return NULL; }
    virtual void addDelegate( PropertyDelegate * , const QString & ) {}
    
    // commit data to model
    virtual void commit() {}

}; // ConfigurationEditor

/**
 * provides display and editing facilities for data items
 * in our case, inheritors will provide this facilities for attributes of configuration
 */
class U2LANG_EXPORT PropertyDelegate : public QItemDelegate {
public:
    PropertyDelegate(QObject * parent = 0) : QItemDelegate(parent) {}
    virtual ~PropertyDelegate() {}
    virtual QVariant getDisplayValue(const QVariant& v) const {return v;}
    
}; // PropertyDelegate

}//GB2 namespace

#endif
