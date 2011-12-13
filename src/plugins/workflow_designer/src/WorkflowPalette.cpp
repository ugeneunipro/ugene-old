/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

class PaletteDelegate: public QItemDelegate {
public:
    PaletteDelegate(WorkflowPalette *view) : QItemDelegate(view), m_view(view){}

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;

private:
    WorkflowPalette *m_view;
};

QTreeWidgetItem* WorkflowPalette::createItemWidget(QAction *a) {
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setToolTip(0, a->toolTip());
    item->setData(0, Qt::UserRole, qVariantFromValue<QAction*>(a));
    actionMap[a] = item;
    connect(a, SIGNAL(triggered()), SLOT(handleItemAction()));
    connect(a, SIGNAL(toggled(bool)), SLOT(handleItemAction()));

    return item;
}

void WorkflowPalette::handleItemAction() {
    QAction* a = qobject_cast<QAction*>(sender());
    assert(a);
    assert(actionMap[a]);
    if (a) {
        update(indexFromItem(actionMap[a]));
    }
}

QAction* WorkflowPalette::createItemAction(ActorPrototype* item) {
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

void WorkflowPalette::sl_selectProcess(bool checked) {
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

void WorkflowPalette::resetSelection() {
    if (currentAction) {
        currentAction->setChecked(false);
        currentAction = NULL;
    }
}

void WorkflowPalette::rebuild() {
    setMouseTracking(false);
    resetSelection();
    ActorPrototypeRegistry* reg = qobject_cast<ActorPrototypeRegistry*>(sender());
    if (reg) {
        QVariant v = saveState();
        clear();
        setContent(reg);
        restoreState(v);
    }
    setMouseTracking(true);
}

WorkflowPalette::WorkflowPalette(ActorPrototypeRegistry* reg, QWidget *parent) 
: QTreeWidget(parent), overItem(NULL), currentAction(NULL)
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
}

void WorkflowPalette::setContent(ActorPrototypeRegistry* reg) {
    QMapIterator<Descriptor, QList<ActorPrototype*> > it(reg->getProtos());
    while (it.hasNext()) {
        it.next();
        QTreeWidgetItem* category = new QTreeWidgetItem(this);
        category->setText(0, it.key().getDisplayName());
        category->setData(0, Qt::UserRole, it.key().getId());
        addTopLevelItem(category);
        
        foreach(ActorPrototype* proto, it.value()) {
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

void WorkflowPalette::sortTree() {
    sortItems(0,Qt::AscendingOrder);

    QString text = BaseActorCategories::CATEGORY_DATASRC().getDisplayName();
    QTreeWidgetItem *item;
    if(!findItems(text,Qt::MatchExactly).isEmpty()) {
        item = findItems(text,Qt::MatchExactly).first();
        takeTopLevelItem(indexFromItem(item).row());
        insertTopLevelItem(0,item);
    }

    text = BaseActorCategories::CATEGORY_DATASINK().getDisplayName();
    if(!findItems(text,Qt::MatchExactly).isEmpty()) {
        item = findItems(text,Qt::MatchExactly).first();
        takeTopLevelItem(indexFromItem(item).row());
        insertTopLevelItem(1,item);
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

    text = BaseActorCategories::CATEGORY_DATAFLOW().getDisplayName();
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

QMenu* WorkflowPalette::createMenu(const QString& name) {
    QMenu* itemsMenu = new QMenu(name, this);
    QMenu *dataSink = NULL, *dataSource = NULL, *userScript = NULL, *externalTools = NULL;
    QAction *firstAction = NULL;
    QMapIterator<QString, QList<QAction*> > it(categoryMap);
    while (it.hasNext()) {
        it.next();
        QMenu* grpMenu = new QMenu(it.key(), itemsMenu);
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
            QAction * a = itemsMenu->addMenu(grpMenu);
            firstAction = firstAction ? firstAction : a;
        }
    }

    itemsMenu->insertMenu(firstAction, dataSource);
    itemsMenu->insertMenu(firstAction, dataSink);
    if (userScript) {
        itemsMenu->addMenu(userScript);
    }
    if (externalTools) {
        itemsMenu->addMenu(externalTools);
    }

    return itemsMenu;
}

void WorkflowPalette::mouseMoveEvent(QMouseEvent * event) {
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
        mime->setData(MIME_TYPE, proto->getId().toAscii());
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

void WorkflowPalette::leaveEvent(QEvent *) {
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

void WorkflowPalette::mousePressEvent(QMouseEvent * event) {
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

void WorkflowPalette::contextMenuEvent(QContextMenuEvent *e)
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

void WorkflowPalette::removeElement() {
    QMessageBox msg(this);
    msg.setWindowTitle("Remove element");
    msg.setText("Remove this element?");
    msg.addButton(QMessageBox::Ok);
    msg.addButton(QMessageBox::Cancel);
    if(msg.exec() == QMessageBox::Cancel) {
        return;
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
        return;
    }
    
    QString id = proto->getId();
    emit si_protoDeleted(id);
    ActorPrototypeRegistry *reg = WorkflowEnv::getProtoRegistry();
    assert(reg);
    reg->unregisterProto(id);
}

void WorkflowPalette::editElement() {
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
        ExternalProcessConfig *cfg = WorkflowEnv::getExternalCfgRegistry()->getConfigByName(proto->getId());
        CreateExternalProcessDialog dlg(this, cfg);
        if(dlg.exec() == QDialog::Accepted) {
            cfg = dlg.config();

            WorkflowEnv::getExternalCfgRegistry()->unregisterConfig(oldName);
            WorkflowEnv::getExternalCfgRegistry()->registerExternalTool(cfg);
            if(oldName != cfg->name) {
                removeElement();
            } else {
                QString id = proto->getId();
                emit si_protoDeleted(id);
                reg->unregisterProto(proto->getId());
            }
            LocalWorkflow::ExternalProcessWorkerFactory::init(cfg);
        }
    }
}


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

        QStyleOption branchOption;
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

QVariant WorkflowPalette::saveState() const {
    QVariantMap m;
    for (int i = 0, count = topLevelItemCount(); i < count; ++i) {
        QTreeWidgetItem* it = topLevelItem(i);
        m.insert(it->data(0, Qt::UserRole).toString(), it->isExpanded());
    }
    return m;
}

void WorkflowPalette::restoreState(const QVariant& v) {
    QMapIterator<QString,QVariant> it(v.toMap());
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


}//namespace
