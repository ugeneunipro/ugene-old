#include "IterationListWidget.h"
#include <U2Core/Log.h>
#include <U2Core/Settings.h>

#include <QtGui/QAction>
#include <QtGui/QTableWidget>

#include <QtGui/QHeaderView>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QAbstractListModel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QToolBar>
#include <QtGui/QMessageBox>

/* TRANSLATOR U2::LocalWorkflow::IterationListWidget */

namespace U2 {

static QString findIterationName(const QString & candidate, const QList<Iteration> & its) {
    bool unique = true;
    foreach(const Iteration & it, its) {
        if(candidate == it.name) {
            unique = false; break;
        }
    }
    if(unique) {
        return candidate;
    }
    
    QList<QString> similar;
    foreach(const Iteration & it, its) {
        if(it.name.startsWith(candidate)) {
            similar << it.name;
        }
    }
    int num = similar.size() + 1;
    foreach(const QString & name, similar) {
        QStringList lst = name.split(QRegExp("\\s+"));
        if(lst.size() > 1) {
            bool ok = false;
            int n = lst.last().toInt(&ok);
            if(ok) {
                num = qMax(num, n + 1);
            }
        }
    }
    return candidate + QString(" %1").arg(num);
}

class IterationListModel : public QAbstractListModel {
public:
    IterationListModel(QObject *parent = 0) : QAbstractListModel(parent) {}
    Qt::ItemFlags flags ( const QModelIndex & index ) const {
        Q_UNUSED(index);
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const {
        if (parent.isValid())
            return 0;
        return dataList.size();
    }
    /*Used to supply item data to views and delegates. 
    Generally, models only need to supply data for Qt::DisplayRole and any application-specific user roles, 
    but it is also good practice to provide data for Qt::ToolTipRole, Qt::AccessibleTextRole, and Qt::AccessibleDescriptionRole.*/
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const {
        if (index.row() < 0 || index.row() >= dataList.size())
            return QVariant();

        if (role == Qt::DisplayRole || role == Qt::EditRole)
            return dataList.at(index.row()).name;

        if (role == Qt::ToolTipRole) {
            return QString("%1 [id=%2]").arg(dataList.at(index.row()).name).arg(dataList.at(index.row()).id);
        }
        return QVariant();
    }
    /*Used to modify the item of data associated with a specified model index. 
    To be able to accept user input, provided by user interface elements, this function must handle data associated with Qt::EditRole. 
    The implementation may also accept data associated with many different kinds of roles specified by Qt::ItemDataRole. 
    After changing the item of data, models must emit the dataChanged() signal to inform other components of the change.*/
    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole ) {
        if (index.row() >= 0 && index.row() < dataList.size()
            && (role == Qt::EditRole || role == Qt::DisplayRole)) {
                QString name = value.toString();
                if (dataList.at(index.row()).name != name) {
                    dataList[index.row()].name = name;
                    emit dataChanged(index, index);
                }
                return true;
        }
        return false;
    }

    bool copySelected(const QItemSelection& sel) {
        QList<QPersistentModelIndex> lst;
        foreach (QModelIndex i, sel.indexes()) {
            lst.append(QPersistentModelIndex(i));
        }
        foreach(QPersistentModelIndex i, lst) {
            assert(i.isValid());
            int row = i.row();
            beginInsertRows(QModelIndex(), row, row);
            Iteration copy = dataList.at(row);
            copy.name = findIterationName(U2::IterationListWidget::tr("Copy of %1").arg(copy.name), dataList);
            dataList.insert(row, copy);
            endInsertRows();
        }
        return true;
    }

    bool removeSelected(const QItemSelection& sel) {
        QList<QPersistentModelIndex> lst;
        foreach (QModelIndex i, sel.indexes()) {
            lst.append(QPersistentModelIndex(i));
        }
        foreach(QPersistentModelIndex i, lst) {
            assert(i.isValid());
            int row = i.row();
            beginRemoveRows(QModelIndex(), row, row);
            dataList.removeAt(row);
            endRemoveRows();
        }
        return true;
    }

    bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() ) {
        if (count < 1 || row < 0 || row > rowCount(parent)) {
            return false;
        }
        
        beginInsertRows(QModelIndex(), row, row + count - 1);
        for (int r = 0; r < count; ++r) {
            dataList.insert(row, findIterationName(U2::IterationListWidget::tr("New Iteration"), dataList));
        }
        endInsertRows();
        return true;
    }

    bool removeRows(int row, int count, const QModelIndex &parent) {
        if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
            return false;

        beginRemoveRows(QModelIndex(), row, row + count - 1);
        for (int r = 0; r < count; ++r)
            dataList.removeAt(row);
        endRemoveRows();

        return true;
    }

    void replaceData(const QList<Iteration>& lst) {
        dataList = lst;
        reset();
    }

    QList<Iteration> dataList;
}; // IterationListModel

IterationListWidget::IterationListWidget(QWidget *parent) : QWidget(parent), cloneAction(NULL), removeAction(NULL) {
    QVBoxLayout* vert = new QVBoxLayout(this);
    vert->setSpacing(0);
    vert->setContentsMargins(0, 1, 0, 0);
    iterationList = new QListView(this);
    iterationList->setAlternatingRowColors(false);
    iterationList->setSelectionMode(QAbstractItemView::SingleSelection);
    iterationList->setSelectionBehavior(QAbstractItemView::SelectRows);
    vert->addWidget(iterationList);
    setupIterationUI(this);
    sl_updateActions();
}

QList<Iteration>& IterationListWidget::list() const {
    IterationListModel* mdl = static_cast<IterationListModel*>(iterationList->model());
    return mdl->dataList;
}

void IterationListWidget::setList(const QList<Iteration>& lst) {
    IterationListModel* mdl = static_cast<IterationListModel*>(iterationList->model());
    mdl->replaceData(lst);
    updateIterationState();
}

int IterationListWidget::current() const {
    int row = iterationList->selectionModel()->currentIndex().row();
    if (row == -1 && !iterationList->selectionModel()->selectedRows().isEmpty()) {
        row = iterationList->selectionModel()->selectedRows().first().row();
    }
    if (row == -1) {
        //TODO: assert(0);
        row = 0;
    }
    return row;
}

QItemSelection IterationListWidget::selection() const {
    return iterationList->selectionModel()->selection();
}

void IterationListWidget::setupIterationUI(QWidget* w) {
    QToolBar* tb = new QToolBar(w);
    cloneAction = new QAction(QIcon(), tr("Clone"), this);
    connect(cloneAction, SIGNAL(triggered()), SLOT(sl_cloneIteration()));
    tb->addAction(cloneAction);
    QAction* addAction = new QAction(QIcon(), tr("Add"), this);
    connect(addAction, SIGNAL(triggered()), SLOT(sl_addIteration()));
    tb->addAction(addAction);
    removeAction = new QAction(QIcon(), tr("Remove"), this);
    connect(removeAction, SIGNAL(triggered()), SLOT(sl_removeIteration()));
    tb->addAction(removeAction);
    QAction* selectAction = new QAction(QIcon(), tr("All"), this);
    connect(selectAction, SIGNAL(triggered()), SLOT(sl_selectIterations()));
    //tb->addAction(selectAction);
    
    iterationList->setModel(new IterationListModel(this));
    
    connect(iterationList->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(sl_iterationSelected()));
    connect(iterationList->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SIGNAL(listChanged()));
    connect(iterationList->model(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SIGNAL(iterationListAboutToChange()));
    connect(iterationList->model(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), SIGNAL(iterationListAboutToChange()));
    connect(this, SIGNAL(selectionChanged()), SLOT(sl_updateActions()));
    
    qobject_cast<QBoxLayout*>(w->layout())->insertWidget(0, tb);
}

void IterationListWidget::updateIterationState() {
    if (iterationList->model()->rowCount() == 0) {
        sl_addIteration();
        iterationList->model()->setData(iterationList->model()->index(0,0), tr("Default iteration"));
        sl_selectIterations();
    } else if (iterationList->selectionModel()->selection().isEmpty()) {
        iterationList->selectionModel()->setCurrentIndex(iterationList->model()->index(0,0), QItemSelectionModel::Select);
    }
}

void IterationListWidget::sl_addIteration() {
    int row = iterationList->currentIndex().row();
    if (row < 0) row = 0;
    iterationList->model()->insertRow(row);
    emit listChanged();
}
void IterationListWidget::sl_removeIteration(){
    IterationListModel* mdl = static_cast<IterationListModel*>(iterationList->model());
    mdl->removeSelected(iterationList->selectionModel()->selection());
    updateIterationState();
    emit listChanged();
}

void IterationListWidget::sl_cloneIteration() {
    IterationListModel* mdl = static_cast<IterationListModel*>(iterationList->model());
    QItemSelection sel = iterationList->selectionModel()->selection();
    if (!sel.isEmpty()) {
        mdl->copySelected(sel);
        emit listChanged();
    }
}
void IterationListWidget::sl_selectIterations(){
    iterationList->selectAll();
}

void IterationListWidget::sl_iterationSelected() {
    emit selectionChanged();
}

void IterationListWidget::sl_updateActions() {
    bool noSelection = iterationList->selectionModel()->selection().isEmpty();
    cloneAction->setEnabled(!noSelection);
    removeAction->setEnabled(!noSelection);
}

void IterationListWidget::selectIteration(int id) {
    IterationListModel* mdl = static_cast<IterationListModel*>(iterationList->model());
    for (int i = 0; i < mdl->dataList.size(); ++i)
    {
        if (mdl->dataList.at(i).id == id) {
            disconnect(iterationList->selectionModel(), 
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(sl_iterationSelected()));
            iterationList->selectionModel()->clear();
            iterationList->selectionModel()->setCurrentIndex( iterationList->model()->index(i,0), QItemSelectionModel::Select);
            connect(iterationList->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(sl_iterationSelected()));
            break;
        }
    }
}

bool IterationListWidget::expandList(const ActorId& id, const QString& key, const QVariant& val) {
    QVariantList vList = val.toList();
    if (vList.size() > 1) {
        QMessageBox askBox(QMessageBox::Question, tr("Workflow Designer"),
                            tr("Do you want to divide the list into separate iterations?"),
                            QMessageBox::Yes | QMessageBox::No, this);
        askBox.setDefaultButton(QMessageBox::No);
        askBox.setTextFormat(Qt::RichText);
        askBox.setInformativeText(tr("If you need to process each item in the list with different schema parameters,\n"
            "the Designer can create separate iterations for each item now.\n"
            "Otherwise, if all items should be processed uniformly,\nit is better to leave the list intact."));
        if (QMessageBox::Yes == askBox.exec()) {
            QList<Iteration>& dataList = list(); 
            Iteration& base = dataList[current()];
            QVariant v = vList.takeFirst();
            base.cfg[id].insert(key,v);
            base.name = v.toString();
            foreach (QVariant v, vList) {
                Iteration it(v.toString());
                it.cfg = base.cfg;
                it.cfg[id].insert(key,v);
                dataList.append(it);
            }
            setList(dataList);
            emit listChanged();
            return true;
        }
    }
    return false;
}

}//namespace
