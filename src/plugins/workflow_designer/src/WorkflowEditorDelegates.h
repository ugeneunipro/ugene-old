#ifndef _U2_WORKFLOW_DESIGNER_EDITOR_DELEGATES_H_
#define _U2_WORKFLOW_DESIGNER_EDITOR_DELEGATES_H_

#include <U2Lang/ConfigurationEditor.h>
#include <QtGui/QItemDelegate>

Q_DECLARE_METATYPE(U2::PropertyDelegate*)

namespace U2 {

class WorkflowEditor;

enum WorkflowEditorRoles {
    DelegateRole = Qt::UserRole + 100,
    DescriptorRole
};

class ProxyDelegate : public QItemDelegate {
public:
    ProxyDelegate(QWidget *parent = 0) : QItemDelegate(parent) {}

    virtual void setPropertyValue(const QString& name, QVariant val) const { Q_UNUSED(name);Q_UNUSED(val); }
    virtual bool handlePropertyValueList(const QString& name, QVariant list) const { Q_UNUSED(name);Q_UNUSED(list); return false; }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    
}; // ProxyDelegate

class SuperDelegate : public ProxyDelegate {
public:
    SuperDelegate(WorkflowEditor *parent);
    
    virtual bool handlePropertyValueList(const QString& name, QVariant list) const;
        
private:
    WorkflowEditor* owner;
    
}; // SuperDelegate

} // U2

#endif //_U2_WORKFLOW_DESIGNER_EDITOR_DELEGATES_H_
