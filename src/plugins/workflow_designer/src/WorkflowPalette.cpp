/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "WorkflowPalette.h"

#include "CreateScriptWorker.h"
#include "library/ScriptWorker.h"
#include "library/ExternalProcessWorker.h"
#include "library/CreateExternalProcessDialog.h"

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/WorkflowEnv.h>

#include <U2Lang/WorkflowSettings.h>

#include <U2Core/Log.h>
#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>

#include <QtGui/QMenu>
#include <QtGui/QToolBox>
#include <QtGui/QButtonGroup>
#include <QtGui/QToolButton>
#include <QtGui/QAction>

#include <QtGui/QHeaderView>
#include <QtGui/QApplication>
#include <QtCore/QAbstractItemModel>
#include <QtGui/QTreeView>
#include <QtGui/QStyle>
#include <QtGui/QPainter>
#include <QtGui/QItemDelegate>
#include <QtGui/QTreeView>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QMessageBox>

#include <QtCore/QDir>


namespace U2 {

const QString WorkflowPalette::MIME_TYPE("application/x-ugene-workflow-id");

WorkflowPalette::WorkflowPalette(ActorPrototypeRegistry* reg, QWidget *parent) 
: QWidget(parent)
{
    setupUi(this);
    elementsList = new WorkflowPaletteElements(reg, this);
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);

    this->layout()->addWidget(elementsList);

    delTextAction = new QAction(this);
    delTextAction->setShortcut(QKeySequence(tr("Esc")));
    this->addAction(delTextAction);

    connect(delTextAction, SIGNAL(triggered()), filterNameEdit, SLOT(clear()));

    connect(elementsList, SIGNAL(processSelected(Workflow::ActorPrototype*)), SIGNAL(processSelected(Workflow::ActorPrototype*)));
    connect(elementsList, SIGNAL(si_protoDeleted(const QString &)), SIGNAL(si_protoDeleted(const QString &)));
    connect(elementsList, SIGNAL(si_protoChanged()), SIGNAL(si_protoChanged()));
    connect(elementsList, SIGNAL(si_protoListModified()), SIGNAL(si_protoListModified()));

    connect(filterNameEdit, SIGNAL(textChanged(const QString &)), elementsList, SLOT(sl_nameFilterChanged(const QString &)));
    this->setObjectName("palette");
}

QMenu* WorkflowPalette::createMenu(const QString &name) {
    return elementsList->createMenu(name);
}

void WorkflowPalette::createMenu(QMenu *menu) {
    elementsList->createMenu(menu);
}

void WorkflowPalette::resetSelection() {
    elementsList->resetSelection();
}

QVariant WorkflowPalette::saveState() const {
    return elementsList->saveState();
}

void WorkflowPalette::restoreState(const QVariant& v) {
    elementsList->restoreState(v);
}

void WorkflowPalette::setFocus(){
    filterNameEdit->setFocus();
}

class PaletteDelegate: public QItemDelegate {
public:
    PaletteDelegate(WorkflowPaletteElements *view) : QItemDelegate(view), m_view(view){}

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;

private:
    WorkflowPaletteElements *m_view;
};

void PaletteDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    if (!model->parent(index).isValid()) {
        // this is a top-level item.
        QStyleOptionButton buttonOption;

        buttonOption.state = option.state;
#ifdef Q_WS_MAC
        buttonOption.state |= QStyle::State_Raised;
#endif
        buttonOption.state &= ~QStyle::State_HasFocus;

        buttonOption.rect = option.rect;
        buttonOption.palette = option.palette;
        buttonOption.features = QStyleOptionButton::None;
        m_view->style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter, m_view);

        QStyleOptionViewItemV2 branchOption;
        static const int i = 9; // ### hardcoded in qcommonstyle.cpp
        QRect r = option.rect;
        branchOption.rect = QRect(r.left() + i/2, r.top() + (r.height() - i)/2, i, i);
        branchOption.palette = option.palette;
        branchOption.state = QStyle::State_Children;

        if (m_view->isExpanded(index))
            branchOption.state |= QStyle::State_Open;

        m_view->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, m_view);

        // draw text
        QRect textrect = QRect(r.left() + i*2, r.top(), r.width() - ((5*i)/2), r.height());
        QString text = elidedText(option.fontMetrics, textrect.width(), Qt::ElideMiddle, 
            model->data(index, Qt::DisplayRole).toString());
        m_view->style()->drawItemText(painter, textrect, Qt::AlignCenter,
            option.palette, m_view->isEnabled(), text);

    } else {
        QStyleOptionToolButton buttonOption;
        buttonOption.state = option.state;
        buttonOption.state &= ~QStyle::State_HasFocus;
        buttonOption.direction = option.direction;
        buttonOption.rect = option.rect;
        buttonOption.font = option.font;
        buttonOption.fontMetrics = option.fontMetrics;
        buttonOption.palette = option.palette;
        buttonOption.subControls = QStyle::SC_ToolButton;
        buttonOption.features = QStyleOptionToolButton::None;

        QAction* action = qVariantValue<QAction*>(index.data(Qt::UserRole));
        buttonOption.text = action->text();
        buttonOption.icon = action->icon();
        if (!buttonOption.icon.isNull()) {
            buttonOption.iconSize = QSize(22, 22);
        }
        if (action->isChecked()) {
            buttonOption.state |= QStyle::State_On;
            buttonOption.state |= QStyle::State_Sunken;
            buttonOption.activeSubControls = QStyle::SC_ToolButton;
        } else {
            buttonOption.state |= QStyle::State_Raised;
            buttonOption.activeSubControls = QStyle::SC_None;
        }

        if (m_view->overItem == m_view->itemFromIndex(index)) {
            buttonOption.state |= QStyle::State_MouseOver;
        }

        buttonOption.state |= QStyle::State_AutoRaise;

        buttonOption.toolButtonStyle = Qt::ToolButtonTextBesideIcon;
        m_view->style()->drawComplexControl(QStyle::CC_ToolButton, &buttonOption, painter, m_view);

        //QItemDelegate::paint(painter, option, index);
    }
}

QSize PaletteDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    QStyleOptionViewItem option = opt;
    bool top = !model->parent(index).isValid();
    QSize sz = QItemDelegate::sizeHint(opt, index) + QSize(top?2:20, top?2:20);
    return sz;
}

/************************************************************************/
/* WorkflowPaletteElements */
/************************************************************************/
WorkflowPaletteElements::WorkflowPaletteElements(ActorPrototypeRegistry* reg, QWidget *parent) 
: QTreeWidget(parent), overItem(NULL), currentAction(NULL), protoRegistry(reg)
{
    setFocusPolicy(Qt::NoFocus);
    setSelectionMode(QAbstractItemView::NoSelection);
    setItemDelegate(new PaletteDelegate(this));
    setRootIsDecorated(false);
    //setAnimated(true);
    setMouseTracking(true);
    setColumnCount(1);
    header()->hide();
    header()->setResizeMode(QHeaderView::Stretch);
    //setTextElideMode (Qt::ElideMiddle);
    setContent(reg);
    connect(reg, SIGNAL(si_registryModified()), SLOT(rebuild()));
    this->setObjectName("WorkflowPaletteElements");
}

QMenu * WorkflowPaletteElements::createMenu(const QString &name) {
    QMenu *menu = new QMenu(name, this);
    createMenu(menu);
    return menu;
}

void WorkflowPaletteElements::createMenu(QMenu *menu) {
    menu->clear();
    QMenu *dataSink = NULL, *dataSource = NULL, *userScript = NULL, *externalTools = NULL;
    QAction *firstAction = NULL;
    QMapIterator<QString, QList<QAction*> > it(categoryMap);
    while (it.hasNext()) {
        it.next();
        QMenu* grpMenu = new QMenu(it.key(), menu);
        QMap<QString, QAction *> map;
        foreach(QAction* a, it.value()) {
            map[a->text()] = a;
        }
        QMapIterator<QString, QAction *> jt(map);
        while(jt.hasNext()) {
            jt.next();
            grpMenu->addAction(jt.value());
        }
        if(it.key() == BaseActorCategories::CATEGORY_DATASRC().getDisplayName()) {
            dataSource = grpMenu;
        }else if (it.key() == BaseActorCategories::CATEGORY_DATASINK().getDisplayName()) {
            dataSink = grpMenu;
        } else if (it.key() == BaseActorCategories::CATEGORY_SCRIPT().getDisplayName()) {
            userScript = grpMenu;
        } else if (it.key() == BaseActorCategories::CATEGORY_EXTERNAL().getDisplayName()) {
            externalTools = grpMenu;
        } else {
            QAction * a = menu->addMenu(grpMenu);
            firstAction = firstAction ? firstAction : a;
        }
    }

    if (NULL != dataSource) {
        menu->insertMenu(firstAction, dataSource);
    }
    if (NULL != dataSink) {
        menu->insertMenu(firstAction, dataSink);
    }
    if (userScript) {
        menu->addMenu(userScript);
    }
    if (externalTools) {
        menu->addMenu(externalTools);
    }
}

static bool filterMatched(const QString &nameFilter, const QString &name) {
    static QRegExp spaces("\\s");
    QStringList filterWords = nameFilter.split(spaces);
    foreach (const QString &word, filterWords) {
        if (!name.contains(word)) {
            return false;
        }
    }
    return true;
}

void WorkflowPaletteElements::setContent(ActorPrototypeRegistry* reg) {
    QMapIterator<Descriptor, QList<ActorPrototype*> > it(reg->getProtos());
    categoryMap.clear();
    while (it.hasNext()) {
        it.next();
        QTreeWidgetItem* category = NULL;

        foreach(ActorPrototype* proto, it.value()) {
            QString name = proto->getDisplayName().toLower();
            if (!filterMatched(nameFilter, name) && !filterMatched(nameFilter, it.key().getDisplayName().toLower())) {
                continue;
            }
            if (NULL == category) {
                category = new QTreeWidgetItem(this);
                category->setText(0, it.key().getDisplayName());
                category->setData(0, Qt::UserRole, it.key().getId());
                addTopLevelItem(category);
            }
            QAction* action = createItemAction(proto);

            int i = 0;
            while(category->child(i)) {
                QString s1 = category->child(i)->data(0,Qt::UserRole).value<QAction* >()->text();
                QString s2 = action->text();

                if(QString::compare(s1,s2, Qt::CaseInsensitive) > 0) {
                    categoryMap[it.key().getDisplayName()] << action;
                    category->insertChild(i,createItemWidget(action));
                    break;
                }
                i++;
            }
            if(!category->child(i)) {
                categoryMap[it.key().getDisplayName()] << action;
                category->addChild(createItemWidget(action));
            }
        }
    }
    sortTree();
}

void WorkflowPaletteElements::rebuild() {
    setMouseTracking(false);
    resetSelection();
    ActorPrototypeRegistry* reg = qobject_cast<ActorPrototypeRegistry*>(sender());
    if (!reg) {
        reg = protoRegistry;
    }

    if (reg) {
        QVariant saved = saveState();
        clear();
        setContent(reg);
        QVariant changed = changeState(saved);
        restoreState(changed);
    }

    setMouseTracking(true);
    emit si_protoListModified();
}

void WorkflowPaletteElements::sortTree() {
    sortItems(0,Qt::AscendingOrder);
    int categoryIdx = 0;

    QString text = BaseActorCategories::CATEGORY_DATASRC().getDisplayName();
    QTreeWidgetItem *item;
    if(!findItems(text,Qt::MatchExactly).isEmpty()) {
        item = findItems(text,Qt::MatchExactly).first();
        takeTopLevelItem(indexFromItem(item).row());
        insertTopLevelItem(categoryIdx,item);
        categoryIdx++;
    }

    text = BaseActorCategories::CATEGORY_DATASINK().getDisplayName();
    if(!findItems(text,Qt::MatchExactly).isEmpty()) {
        item = findItems(text,Qt::MatchExactly).first();
        takeTopLevelItem(indexFromItem(item).row());
        insertTopLevelItem(categoryIdx,item);
        categoryIdx++;
    }

    text = BaseActorCategories::CATEGORY_DATAFLOW().getDisplayName();
    if (!findItems(text, Qt::MatchExactly).isEmpty())
    {
        item = findItems(text, Qt::MatchExactly).first();
        if (item) {
            takeTopLevelItem(indexFromItem(item).row());
            insertTopLevelItem(categoryIdx,item);
            categoryIdx++;
        }
    }

    text = BaseActorCategories::CATEGORY_SCRIPT().getDisplayName();
    if(!findItems(text,Qt::MatchExactly).isEmpty()) {
        item = findItems(text,Qt::MatchExactly).first();
        if(item) {
            takeTopLevelItem(indexFromItem(item).row());
            addTopLevelItem(item);
        }
    }

    text = BaseActorCategories::CATEGORY_EXTERNAL().getDisplayName();
    if (!findItems(text, Qt::MatchExactly).isEmpty())
    {
        item = findItems(text, Qt::MatchExactly).first();
        if (item)
        {
            takeTopLevelItem(indexFromItem(item).row());
            addTopLevelItem(item);
        }
    }
}

QTreeWidgetItem* WorkflowPaletteElements::createItemWidget(QAction *a) {
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setToolTip(0, a->toolTip());
    item->setData(0, Qt::UserRole, qVariantFromValue<QAction*>(a));
    actionMap[a] = item;
    connect(a, SIGNAL(triggered()), SLOT(handleItemAction()));
    connect(a, SIGNAL(toggled(bool)), SLOT(handleItemAction()));

    return item;
}

QAction* WorkflowPaletteElements::createItemAction(ActorPrototype* item) {
    QAction* a = new QAction(item->getDisplayName(), this);
    a->setToolTip(item->getDocumentation());
    a->setCheckable(true);
    if (item->getIcon().isNull()) {
        item->setIconPath(":workflow_designer/images/green_circle.png");
    }
    a->setIcon(item->getIcon());
    a->setData(qVariantFromValue(item));
    connect(a, SIGNAL(triggered(bool)), SLOT(sl_selectProcess(bool)));
    connect(a, SIGNAL(toggled(bool)), SLOT(sl_selectProcess(bool)));
    return a;
}

void WorkflowPaletteElements::resetSelection() {
    if (currentAction) {
        currentAction->setChecked(false);
        currentAction = NULL;
    }
}

QVariant WorkflowPaletteElements::saveState() const {
    QVariantMap m = expandState;
    for (int i = 0, count = topLevelItemCount(); i < count; ++i) {
        QTreeWidgetItem* it = topLevelItem(i);
        m.insert(it->data(0, Qt::UserRole).toString(), it->isExpanded());
    }
    return m;
}

void WorkflowPaletteElements::restoreState(const QVariant& v) {
    expandState = v.toMap();
    QMapIterator<QString,QVariant> it(expandState);
    while (it.hasNext())
    {
        it.next();
        for (int i = 0; i < topLevelItemCount(); i++) {
            if (topLevelItem(i)->data(0, Qt::UserRole) == it.key()) {
                topLevelItem(i)->setExpanded(it.value().toBool());
                break;
            }
        }
    }
}

void WorkflowPaletteElements::handleItemAction() {
    QAction* a = qobject_cast<QAction*>(sender());
    assert(a);
    assert(actionMap[a]);
    if (a) {
        update(indexFromItem(actionMap[a]));
    }
}

void WorkflowPaletteElements::sl_selectProcess(bool checked) {
    if (currentAction && currentAction != sender()) {
        currentAction->setChecked(false);
    }
    if (!checked) {
        currentAction = NULL;
    } else {
        currentAction = qobject_cast<QAction*>(sender());
        assert(currentAction);
    }
    emit processSelected(currentAction ? 
        qVariantValue<Workflow::ActorPrototype*>(currentAction->data()) : NULL );
}

void WorkflowPaletteElements::editElement() {
    ActorPrototype *proto = currentAction->data().value<ActorPrototype *>();
    QString oldName = proto->getDisplayName();
    ActorPrototypeRegistry *reg = WorkflowEnv::getProtoRegistry();
    QMap<Descriptor, QList<ActorPrototype*> > categories = reg->getProtos();

    if(categories.value(BaseActorCategories::CATEGORY_SCRIPT()).contains(proto)) {
        CreateScriptElementDialog dlg(this, proto);
        if(dlg.exec() == QDialog::Accepted) {
            ActorPrototypeRegistry *reg = WorkflowEnv::getProtoRegistry();
            assert(reg);

            QList<DataTypePtr > input = dlg.getInput();
            QList<DataTypePtr > output = dlg.getOutput();
            QList<Attribute*> attrs = dlg.getAttributes();
            QString name = dlg.getName();
            QString desc = dlg.getDescription();

            if(oldName != name) {
                removeElement();
            } else {
                QString id = proto->getId();
                emit si_protoDeleted(id);
                reg->unregisterProto(proto->getId());
            }
            LocalWorkflow::ScriptWorkerFactory::init(input, output, attrs, name, desc, dlg.getActorFilePath());
        }
    } else { //External process category
        ExternalProcessConfig *oldCfg = WorkflowEnv::getExternalCfgRegistry()->getConfigByName(proto->getId());
        ExternalProcessConfig *cfg = new ExternalProcessConfig(*oldCfg);
        CreateExternalProcessDialog dlg(this, cfg, false);
        if(dlg.exec() == QDialog::Accepted) {
            cfg = dlg.config();

            bool deleted = true;
            if (!(*oldCfg == *cfg)) {
                if(oldName != cfg->name) {
                    deleted = removeElement();
                } else {
                    emit si_protoDeleted(proto->getId());
                    reg->unregisterProto(proto->getId());
                    delete proto;
                }

                LocalWorkflow::ExternalProcessWorkerFactory::init(cfg);
            }
            if (deleted) {
                WorkflowEnv::getExternalCfgRegistry()->unregisterConfig(oldName);
            }
            WorkflowEnv::getExternalCfgRegistry()->registerExternalTool(cfg);
            emit si_protoChanged();
        }
    }
}

bool WorkflowPaletteElements::removeElement() {
    QMessageBox msg(this);
    msg.setWindowTitle("Remove element");
    msg.setText("Remove this element?");
    msg.addButton(QMessageBox::Ok);
    msg.addButton(QMessageBox::Cancel);
    if(msg.exec() == QMessageBox::Cancel) {
        return false;
    }

    ActorPrototype *proto = currentAction->data().value<ActorPrototype *>();

    QString path = WorkflowSettings::getUserDirectory();

    QString fileName = path + proto->getDisplayName() + ".usa"; //use constant
    if(!QFile::exists(fileName)) {
        fileName = WorkflowSettings::getExternalToolDirectory() + proto->getDisplayName() + ".etc";
    }
    QFile::setPermissions(fileName, QFile::ReadOwner | QFile::WriteOwner);
    if(!QFile::remove(fileName)) {
        uiLog.error(tr("Can't remove element %1").arg(proto->getDisplayName()));
        return true;
    }

    // remove proto from categoryMap
    QMap<QString, QList<QAction*> >::iterator i = categoryMap.begin();
    for(; i != categoryMap.end(); i++) {
        i->removeAll(currentAction);
    }

    // unresister prototype
    QString id = proto->getId();
    emit si_protoDeleted(id);
    ActorPrototypeRegistry *reg = WorkflowEnv::getProtoRegistry();
    assert(reg);
    reg->unregisterProto(id);
    return true;
}

void WorkflowPaletteElements::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu;
    menu.addAction(tr("Expand all"), this, SLOT(expandAll()));
    menu.addAction(tr("Collapse all"), this, SLOT(collapseAll()));
    if(itemAt(e->pos()) && itemAt(e->pos())->parent() && (itemAt(e->pos())->parent()->text(0) == BaseActorCategories::CATEGORY_SCRIPT().getDisplayName() 
        || itemAt(e->pos())->parent()->text(0) == BaseActorCategories::CATEGORY_EXTERNAL().getDisplayName())) {
            menu.addAction(tr("Edit"), this, SLOT(editElement()));
            menu.addAction(tr("Remove"), this, SLOT(removeElement()));
            currentAction = actionMap.key(itemAt(e->pos()));
    }
    e->accept();
    menu.exec(mapToGlobal(e->pos()));
}

void WorkflowPaletteElements::mouseMoveEvent(QMouseEvent * event) {
    if (!hasMouseTracking()) return;
    if ((event->buttons() & Qt::LeftButton) && !dragStartPosition.isNull()) {
        if ((event->pos() - dragStartPosition).manhattanLength() <= QApplication::startDragDistance()) return;
        QTreeWidgetItem* item = itemAt(event->pos());
        if (!item) return;
        QAction* action = qVariantValue<QAction*>(item->data(0, Qt::UserRole));
        if (!action) return;
        ActorPrototype* proto = action->data().value<ActorPrototype*>();
        assert(proto);
        QMimeData* mime = new QMimeData();
        mime->setData(WorkflowPalette::MIME_TYPE, proto->getId().toLatin1());
        mime->setText(proto->getId());
        QDrag *drag = new QDrag(this);
        drag->setMimeData(mime);
        drag->setPixmap(action->icon().pixmap(QSize(44, 44)));

        resetSelection();
        dragStartPosition = QPoint();
        Qt::DropAction dropAction = drag->exec(Qt::CopyAction, Qt::CopyAction);
        Q_UNUSED(dropAction);
        return;
    }
    QTreeWidgetItem* prev = overItem;
    overItem = itemAt(event->pos());
    if (prev) {
        update(indexFromItem(prev));
    }
    if (overItem) {
        update(indexFromItem(overItem));
    }

    QTreeWidget::mouseMoveEvent(event);
}

void WorkflowPaletteElements::mousePressEvent(QMouseEvent * event) {
    if (!hasMouseTracking()) return;
    dragStartPosition = QPoint();
    if ((event->buttons() & Qt::LeftButton)) {
        QTreeWidgetItem* item = itemAt(event->pos());
        if (!item) return;
        event->accept();
        if (item->parent() == 0) {
            setItemExpanded(item, !isItemExpanded(item));
            return;
        }

        QAction* action = qVariantValue<QAction*>(item->data(0, Qt::UserRole));
        if (action) {
            action->toggle();
            dragStartPosition = event->pos();
        }
    }
}

void WorkflowPaletteElements::leaveEvent(QEvent *) {
    if (!hasMouseTracking()) {
        return;
    }
    QTreeWidgetItem* prev = overItem;
    overItem = NULL;
    if (prev) {
        QModelIndex index = indexFromItem(prev);
        update(index);
    };
}

QVariant WorkflowPaletteElements::changeState(const QVariant& savedState){
    QVariantMap m = savedState.toMap();

    for (int i = 0, count = topLevelItemCount(); i < count; ++i) {
        QTreeWidgetItem* it = topLevelItem(i);         
        bool expanded = m.value(it->data(0, Qt::UserRole).toString()).toBool();

        QRegExp nonWhitespase("\\s");
        QStringList splitNew = nameFilter.split(nonWhitespase, QString::SkipEmptyParts);
        bool hasCharsNewFilter = splitNew.size() > 0 && !splitNew.first().isEmpty();
        QStringList splitOld = oldNameFilter.split(nonWhitespase, QString::SkipEmptyParts);
        bool hasCharsOldFilter = splitOld.size() > 0 && !splitOld.first().isEmpty();

        if(hasCharsNewFilter && !hasCharsOldFilter){expanded = true;}
        else if(!hasCharsNewFilter && hasCharsOldFilter){expanded = false;}
        
        m.insert(it->data(0, Qt::UserRole).toString(), expanded);
    }   
    return m;
}

void WorkflowPaletteElements::sl_nameFilterChanged(const QString &filter) {
    overItem = NULL;
    oldNameFilter = nameFilter;
    nameFilter = filter.toLower();
    rebuild();
}

}//namespace
