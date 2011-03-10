#include "SchemaConfigurationDialog.h"
#include "WorkflowEditorDelegates.h"
#include "IterationListWidget.h"

#include <U2Lang/WorkflowUtils.h>

#include <QtGui/QPushButton>
#include <QtCore/QAbstractItemModel>
#include <QtGui/QTableView>
#include <QtGui/QScrollBar>
#include <QtGui/QHeaderView>

namespace U2 {

class CfgTreeItem {
public:
    CfgTreeItem(Actor* a, CfgTreeItem *parent = 0) : parentItem(parent), actor(a) {}
    CfgTreeItem(const QString& name, CfgTreeItem *parent) : parentItem(parent), actor(parent->actor), name(name) {}
    ~CfgTreeItem() { qDeleteAll(childItems);}

    int childNumber() const {return parentItem ? parentItem->childItems.indexOf(const_cast<CfgTreeItem*>(this)) : 0;}

    QList<CfgTreeItem*> childItems;
    CfgTreeItem *parentItem;
    Actor* actor;
    QString name;
};

class CfgTreeModel : public QAbstractItemModel {
public:
    CfgTreeModel(QList<Actor*> actors, QList<Iteration> &lst, QObject *parent = 0) : QAbstractItemModel(parent), iterations(lst)
    {
        root = new CfgTreeItem(NULL);
        foreach(Actor* a, actors) {
            QStringList keys = a->getParameters().uniqueKeys();
            if (keys.size() == 0) {
                continue;
                actors.removeAll(a);
            }
            CfgTreeItem* ti = new CfgTreeItem(a, root);
            root->childItems.append(ti);
            foreach(QString name, keys) {
                ti->childItems.append(new CfgTreeItem(name, ti));
            }
        }
    }
    virtual ~CfgTreeModel() { delete root; }

    CfgTreeItem* getItem(const QModelIndex &index) const {
        CfgTreeItem *item = root;
        if (index.isValid()) {
            item = static_cast<CfgTreeItem*>(index.internalPointer());
            assert(item);
        }
        return item;
    }

    /*Given a model index for a parent item, this function allows views and delegates to access children of that item. 
    If no valid child item - corresponding to the specified row, column, and parent model index, can be found, 
    the function must return QModelIndex(), which is an invalid model index.*/
    QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const {
        if (parent.isValid() && parent.column() != 0)
            return QModelIndex();
        CfgTreeItem *parentItem = getItem(parent);
        if (!parentItem || row >= parentItem->childItems.size()) {
            return QModelIndex();
        }
        return createIndex(row, column, parentItem->childItems.at(row));
    }

    /*Provides a model index corresponding to the parent of any given child item. 
    If the model index specified corresponds to a top-level item in the model, 
    or if there is no valid parent item in the model, the function must return an invalid model index, 
    created with the empty QModelIndex() constructor.*/
    QModelIndex parent( const QModelIndex & index ) const {
        CfgTreeItem *childItem = getItem(index);
        if (childItem == root || childItem->parentItem == root) {
            return QModelIndex();
        }
        return createIndex(childItem->parentItem->childNumber(), 0, childItem->parentItem);
    }

    int columnCount(const QModelIndex &parent) const { 
        Q_UNUSED(parent);
        return iterations.size() + 2; 
    }

    int rowCount ( const QModelIndex & parent = QModelIndex() ) const {
        if (!parent.isValid()) {
            return root->childItems.size();
        }
        CfgTreeItem *item = getItem(parent);
        return item ? item->childItems.size() : 0;
    }

    Qt::ItemFlags flags ( const QModelIndex & index ) const {
        CfgTreeItem *item = getItem(index);
        if (item->name.isEmpty() || index.column() == 0)
            return Qt::ItemIsEnabled;
        if(index.column() == 1) {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }

        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch(section) {
                case 0: return SchemaConfigurationDialog::tr("Name");
                case 1: return SchemaConfigurationDialog::tr("Default value");
                default: return iterations.at(section - 2).name;
            }
        }
        return QVariant();
    }

    /*Used to supply item data to views and delegates. 
    Generally, models only need to supply data for Qt::DisplayRole and any application-specific user roles, 
    but it is also good practice to provide data for Qt::ToolTipRole, Qt::AccessibleTextRole, and Qt::AccessibleDescriptionRole.*/
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const {
        CfgTreeItem *item = getItem(index);
        if (!item || !item->actor) {
            return QVariant();
        }
        if (role == Qt::SizeHintRole) {
            int height = QFontMetrics(QFont()).height() + 6;
            return QSize(0, height);
        }
        int col = index.column();
        if (item->name.isEmpty()) {
            // actor
            if (col == 0) {
                switch (role)
                {
                case Qt::DisplayRole: return item->actor->getLabel();
                case Qt::DecorationRole: return item->actor->getProto()->getIcon();
                case Qt::ToolTipRole: return item->actor->getDescription()->toHtml();
                }
            }
            return QVariant();
        }
        
        if (col == 0) {
            // parameter decoration
            switch (role)
            {
            case Qt::DisplayRole: return item->actor->getParameter(item->name)->getDisplayName();
            case Qt::ToolTipRole: return item->actor->getParameter(item->name)->getDocumentation();
            }
            return QVariant();
        }
        // parameter value
        QVariant val = item->actor->getParameter(item->name)->getAttributePureValue();
        bool isDefaultVal = true;
        if (col > 1) {
            const CfgMap& cfg = iterations.at(col-2).cfg;
            ActorId aid = item->actor->getId();
            if (cfg.contains(aid)) {
                const QVariantMap& params = cfg[aid];
                if (params.contains(item->name)) {
                    val = params.value(item->name);
                    isDefaultVal = false;
                }
            }
        }
        ConfigurationEditor* ed = item->actor->getEditor();
        PropertyDelegate* pd = ed ? ed->getDelegate(item->name) : NULL;
        switch (role)
        {
        case Qt::DisplayRole: 
        case Qt::ToolTipRole:
            {
                if(pd) {
                    return pd->getDisplayValue(val);
                } else {
                    QString valueStr = WorkflowUtils::getStringForParameterDisplayRole(val);
                    return !valueStr.isEmpty() ? valueStr : val;
                }
            }
        case Qt::ForegroundRole:
            return isDefaultVal ? QVariant(QColor(Qt::gray)) : QVariant();
        case DelegateRole:
            return qVariantFromValue<PropertyDelegate*>(pd);
        case Qt::EditRole:
        case ConfigurationEditor::ItemValueRole:
            return val;
        }
        return QVariant();
    }

    /*Used to modify the item of data associated with a specified model index. 
    To be able to accept user input, provided by user interface elements, this function must handle data associated with Qt::EditRole. 
    The implementation may also accept data associated with many different kinds of roles specified by Qt::ItemDataRole. 
    After changing the item of data, models must emit the dataChanged() signal to inform other components of the change.*/
    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole ) {
        int col = index.column();
        CfgTreeItem *item = getItem(index);
        if (!item || !item->actor || item->name.isEmpty() || col < 1) {
            return false;
        }
        switch (role)
        {
        case Qt::EditRole:
        case ConfigurationEditor::ItemValueRole:
            /*if (col == 1) {
                QVariant old = item->actor->getParameter(item->name)->getAttributePureValue();
                if (old != value) {
                    item->actor->setParameter(item->name, value);
                    emit dataChanged(index, createIndex(index.row(), iterations.size() + 1, item));
                
                }return true;
            } else {*/
                ActorId aid = item->actor->getId();
                QVariantMap& cfg = iterations[col-2].cfg[aid];
                const QString& key = item->name;
                QVariant old = cfg.contains(key) ? cfg.value(key) : item->actor->getParameter(key)->getAttributePureValue();
                if (old != value) {
                    cfg.insert(key, value);

                    emit dataChanged(index, index);
                }
           // }
            return true;
        }
        return false;
    }

public:
    QList<Iteration>& iterations;
private:
    CfgTreeItem* root;
};



SchemaConfigurationDialog::SchemaConfigurationDialog(const Schema& sh, const QList<Iteration> &lst, QWidget* p) : QDialog(p)
{
    setupUi(this);

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Run"));
    //QPushButton * addButton ( const QString & text, ButtonRole role )

    iterationList = new IterationListWidget(this);
    iterationBox->layout()->addWidget(iterationList);
    iterationList->setList(lst);
    
    iterationBox->hide();
 
    CfgTreeModel *model = new CfgTreeModel(sh.getProcesses(), iterationList->list(), this);
    ProxyDelegate *delegate = new ProxyDelegate(this);
    treeView->setModel(model);
    treeView->setItemDelegate(delegate);  
    dataView->setModel(model);
    dataView->setItemDelegate(delegate);

    for (int i = 0; i < iterationList->list().size(); i++) {
        treeView->hideColumn(i + 2);
    }

    dataView->hideColumn(1);
    //dataView->hideColumn(0);
    //plain hiding of the lead column breaks tree expanded state synchronization
    //so below is a workaround to make the column effectively invisible.
    dataView->header()->resizeSection(0,0);
    dataView->header()->setResizeMode(0,QHeaderView::Fixed);
    
    treeView->expandAll();
    dataView->expandAll();

    treeView->header()->setResizeMode(1, QHeaderView::Stretch);
    treeView->header()->setCascadingSectionResizes(true);
    dataView->header()->setCascadingSectionResizes(true);
    dataView->header()->setStretchLastSection(true);
    dataView->header()->setMovable(true);
    dataView->header()->setHighlightSections(true);

    connect(dataView->verticalScrollBar(), SIGNAL(valueChanged(int)), treeView->verticalScrollBar(), SLOT(setValue(int)));
    connect(treeView->verticalScrollBar(), SIGNAL(valueChanged(int)), dataView->verticalScrollBar(), SLOT(setValue(int)));

    connect(iterationList, SIGNAL(iterationListAboutToChange()), SLOT(finishPropertyEditing()));
//    connect(iterationList, SIGNAL(selectionChanged()), SLOT(updateIterationData()));
//    connect(iterationList, SIGNAL(listChanged()), SLOT(commitIterations()));
//    connect(table, SIGNAL(itemSelectionChanged()), SLOT(sl_showPropDoc()));
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(setModified()));
}

void SchemaConfigurationDialog::finishPropertyEditing() {
    //table->setCurrentCell(0,0, QItemSelectionModel::NoUpdate);
}

void SchemaConfigurationDialog::commit() {
    finishPropertyEditing();
    //commitIterations();
}

QList<Iteration>& SchemaConfigurationDialog::getIterations() const {return iterationList->list();}

}//namespace
