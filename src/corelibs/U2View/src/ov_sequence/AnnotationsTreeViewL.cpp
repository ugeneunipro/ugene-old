/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "AnnotationsTreeViewL.h"

#include "AnnotatedDNAView.h"
#include "ADVConstants.h"
#include "ADVSequenceObjectContext.h"

#include "EditAnnotationDialogController.h"

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/Settings.h>
#include <U2Core/Timer.h>
#include <U2Core/DBXRefRegistry.h>

#include <U2Formats/GenbankLocationParser.h>
#include <U2Core/AnnotationSettings.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationSelection.h>
#include <U2Gui/ProjectTreeController.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/EditQualifierDialog.h>
#include <U2Gui/TreeWidgetUtils.h>
#include <U2Gui/OpenViewTask.h>

#include <QtCore/QFileInfo>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QClipboard>
#include <QtGui/QToolTip>
#include <QtGui/QMessageBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Core/U2SafePoints.h>

/* TRANSLATOR U2::AnnotationsTreeView */

namespace U2 {

class TreeSorter {
public:
    TreeSorter(AnnotationsTreeViewL* t) : w(t) {
        w->setSortingEnabled(false);
    }
    virtual ~TreeSorter() {
        w->setSortingEnabled(true);
    }
    AnnotationsTreeViewL* w;
};

#define SETTINGS_ROOT QString("view_adv/annotations_tree_view/")
#define COLUMN_SIZES QString("columnSizes")

const QString AnnotationsTreeViewL::annotationMimeType = "application/x-annotations-and-groups";

AnnotationsTreeViewL::AnnotationsTreeViewL(AnnotatedDNAView* _ctx) : ctx(_ctx){
    lastMB = Qt::NoButton;
    lastClickedColumn = 0;

    tree = new LazyTreeView(this);
    LazyAnnotationTreeViewModel *model = new LazyAnnotationTreeViewModel();
    tree->setObjectName("tree_widget");
    tree->setModel(model);

    //tree->setSortingEnabled(true);
    //tree->sortItems(0, Qt::AscendingOrder);

    tree->setColumnCount(2);
    headerLabels << tr("Name") << tr("Value");

    tree->setHeaderLabels(headerLabels);
    tree->setHeaderHidden(false);
    //tree->header()->setStretchLastSection(true);
    tree->setUniformRowHeights(true);
    tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree->viewport()->installEventFilter(this);
    tree->setMouseTracking(true);
    tree->setAutoScroll(false);
    //tree->setNumberOfItems();

    connect(tree, SIGNAL(itemEntered(QTreeWidgetItem*, int)), SLOT(sl_itemEntered(QTreeWidgetItem*, int)));
    connect(tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(sl_itemClicked(QTreeWidgetItem*, int)));
    connect(tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(sl_itemDoubleClicked(QTreeWidgetItem*, int)));
    connect(tree, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT(sl_itemExpanded(QTreeWidgetItem*)));
    connect(tree, SIGNAL(itemCollapsed(QTreeWidgetItem*)), SLOT(sl_itemCollapsed(QTreeWidgetItem*)));
    //connect(tree, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(sl_itemActivated(QTreeWidgetItem*, int)));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->addWidget(tree);
    setLayout(layout);
    
    restoreWidgetState();

    connect(ctx, SIGNAL(si_buildPopupMenu(GObjectView*, QMenu*)), SLOT(sl_onBuildPopupMenu(GObjectView*, QMenu*)));
    connect(ctx, SIGNAL(si_annotationObjectAdded(AnnotationTableObject*)), SLOT(sl_onAnnotationObjectAdded(AnnotationTableObject*)));
    connect(ctx, SIGNAL(si_annotationObjectRemoved(AnnotationTableObject*)), SLOT(sl_onAnnotationObjectRemoved(AnnotationTableObject*)));
    foreach(AnnotationTableObject* obj, ctx->getAnnotationObjects()) {
        sl_onAnnotationObjectAdded(obj);
    }
    connectAnnotationSelection();
    connectAnnotationGroupSelection();
    connect(tree, SIGNAL(itemSelectionChanged()), SLOT(sl_onItemSelectionChanged()));

    connect(AppContext::getAnnotationsSettingsRegistry(),
        SIGNAL(si_annotationSettingsChanged(const QStringList&)),
        SLOT(sl_onAnnotationSettingsChanged(const QStringList&)));

#define SORT_INTERVAL 500
    sortTimer.setInterval(SORT_INTERVAL);
    sortTimer.setSingleShot(true);
    connect(&sortTimer, SIGNAL(timeout()), SLOT(sl_sortTree()));

    addColumnIcon = QIcon(":core/images/add_column.png");
    removeColumnIcon = QIcon(":core/images/remove_column.png");
    
    addAnnotationObjectAction = new QAction(tr("Objects with annotations..."), this);
    connect(addAnnotationObjectAction, SIGNAL(triggered()), SLOT(sl_onAddAnnotationObjectToView()));

    removeObjectsFromViewAction = new QAction(tr("Selected objects with annotations from view"), this);
    removeObjectsFromViewAction->setShortcut(QKeySequence(Qt::SHIFT| Qt::Key_Delete));
    removeObjectsFromViewAction->setShortcutContext(Qt::WidgetShortcut);
    connect(removeObjectsFromViewAction, SIGNAL(triggered()), SLOT(sl_removeObjectFromView()));
    tree->addAction(removeObjectsFromViewAction);

    removeAnnsAndQsAction = new QAction(tr("Selected annotations and qualifiers"), this);
    removeAnnsAndQsAction->setShortcut(QKeySequence(Qt::Key_Delete));
    removeAnnsAndQsAction->setShortcutContext(Qt::WindowShortcut);
    connect(removeAnnsAndQsAction, SIGNAL(triggered()), SLOT(sl_removeAnnsAndQs()));
    tree->addAction(removeAnnsAndQsAction);

    copyQualifierAction = new QAction(tr("Copy qualifier text"), this);
    connect(copyQualifierAction, SIGNAL(triggered()), SLOT(sl_onCopyQualifierValue()));

    copyQualifierURLAction = new QAction(tr("Copy qualifier URL"), this);
    connect(copyQualifierURLAction, SIGNAL(triggered()), SLOT(sl_onCopyQualifierURL()));

    toggleQualifierColumnAction = new QAction(tr("Toggle column"), this);
    connect(toggleQualifierColumnAction, SIGNAL(triggered()), SLOT(sl_onToggleQualifierColumn()));

    removeColumnByHeaderClickAction = new QAction(tr("Hide column"), this);
    removeColumnByHeaderClickAction->setIcon(removeColumnIcon);
    connect(removeColumnByHeaderClickAction, SIGNAL(triggered()), SLOT(sl_onRemoveColumnByHeaderClick()));

    copyColumnTextAction = new QAction(tr("Copy column text"), this);
    connect(copyColumnTextAction, SIGNAL(triggered()), SLOT(sl_onCopyColumnText()));

    copyColumnURLAction = new QAction(tr("copy column URL"), this);
    connect(copyColumnURLAction, SIGNAL(triggered()), SLOT(sl_onCopyColumnURL()));

    renameAction = new QAction(tr("Edit item"), this);
    renameAction->setShortcut(QKeySequence(Qt::Key_F2));
    renameAction->setShortcutContext(Qt::WidgetShortcut);
    connect(renameAction, SIGNAL(triggered()), SLOT(sl_rename()));
    tree->addAction(renameAction);

    editAction = new QAction(tr("Edit qualifier"), this);
    editAction->setShortcut(QKeySequence(Qt::Key_F4));
    editAction->setShortcutContext(Qt::WidgetShortcut);
    connect(editAction, SIGNAL(triggered()), SLOT(sl_edit()));
    tree->addAction(editAction);

    viewAction = new QAction(tr("View qualifier"), this);
    viewAction->setShortcut(QKeySequence(Qt::Key_F3));
    viewAction->setShortcutContext(Qt::WidgetShortcut);
    connect(viewAction, SIGNAL(triggered()), SLOT(sl_edit()));
    tree->addAction(viewAction);

    addQualifierAction = new QAction(tr("U2Qualifier..."), this);
    addQualifierAction->setShortcut(QKeySequence(Qt::Key_Insert));
    addQualifierAction->setShortcutContext(Qt::WindowShortcut);
    connect(addQualifierAction, SIGNAL(triggered()), SLOT(sl_addQualifier()));
    tree->addAction(addQualifierAction);

/*    cutAnnotationsAction = new QAction(tr("cut_annotations"), this);
//    cutAnnotationsAction->setShortcut(QKeySequence(Qt::K));
    connect(cutAnnotationsAction, SIGNAL(triggered()), SLOT(sl_cutAnnotations));
    tree->addAction(cutAnnotationsAction);

    copyAnnotationsAction = new QAction(tr("copy_annotations"), this);
//    copyAnnotationsAction->setShortcut(QKeySequence(Qt::K));
    connect(copyAnnotationsAction, SIGNAL(triggered()), SLOT(sl_copyAnnotations));
    tree->addAction(copyAnnotationsAction);

    pasteAnnotationsAction = new QAction(tr("paste_annotations"), this);
//    pasteAnnotationsAction->setShortcut(QKeySequence(Qt::K));
    connect(pasteAnnotationsAction, SIGNAL(triggered()), SLOT(sl_pasteAnnotations));
    tree->addAction(pasteAnnotationsAction);*/

    updateState();

    isDragging = false;
    resetDragAndDropData();
    tree->setAcceptDrops(true);
    renameFlag = false;
}

void AnnotationsTreeViewL::restoreWidgetState() { 
    QStringList geom = AppContext::getSettings()->getValue(SETTINGS_ROOT + COLUMN_SIZES, QStringList()).toStringList();
    if (geom.isEmpty()) {
        tree->setColumnWidth(0, 300);
        tree->setColumnWidth(1, 300);
    } else {
        for (int i=0;i<geom.size(); i++) {
            const QString& w = geom.at(i);
            bool ok = false;
            int width  = w.toInt(&ok);
            if (ok) {
                tree->setColumnWidth(i, width);
            }
        }
    }
}

void AnnotationsTreeViewL::saveWidgetState() {
    QStringList geom;
    for (int i=0; i < tree->model()->columnCount(); i++) {
        QString s = QString::number(tree->columnWidth(i));
        geom.append(s);
    }
    AppContext::getSettings()->setValue(SETTINGS_ROOT+COLUMN_SIZES, geom);
}


AVGroupItemL* AnnotationsTreeViewL::findGroupItem(const AnnotationGroup* g) const {
    GTIMER(c2,t2,"AnnotationsTreeView::findGroupItem");
    if (g->getParentGroup() == NULL) {
        LazyAnnotationTreeViewModel *model = static_cast<LazyAnnotationTreeViewModel*>(tree->model());
        AVItemL *root = model->getItem(QModelIndex());
        for(int i = 0; i < root->childCount(); i++) {
            AVGroupItemL* item = static_cast<AVGroupItemL*>(root->child(i));
            if(item->group == g) {
                return item;
            }
        }

    } else {
        AVGroupItemL* parentGroupItem = findGroupItem(g->getParentGroup());
        if (parentGroupItem != NULL) {
            for(int i = 0, n = parentGroupItem->childCount(); i < n; i++) {
                AVItemL* item = static_cast<AVItemL*>(parentGroupItem->child(i));
                if (item->type != AVItemType_Group) {
                    continue;
                }
                AVGroupItemL* gItem = static_cast<AVGroupItemL*>(item);
                if (gItem->group == g) {
                    return gItem;
                }
            }
        }
    }
    return NULL;
}

AVAnnotationItemL* AnnotationsTreeViewL::findAnnotationItem(const AVGroupItemL* groupItem, const Annotation* a) const {
    GTIMER(c2,t2,"AnnotationsTreeView::findAnnotationItem");
    for(int i = 0, n = groupItem->childCount(); i < n; i++) {
        AVItemL* item = static_cast<AVItemL*>(groupItem->child(i));
        if (item->type != AVItemType_Annotation) {
            continue;
        }
        AVAnnotationItemL* aItem = static_cast<AVAnnotationItemL*>(item);
        if (aItem->annotation == a) {
            return aItem;
        }
    }
    return NULL;
}

AVAnnotationItemL* AnnotationsTreeViewL::findAnnotationItem(const AnnotationGroup* g, const Annotation* a) const {
    AVGroupItemL* groupItem = findGroupItem(g);
    if (groupItem == NULL) {
        return NULL;
    }
    return findAnnotationItem(groupItem, a);
}

/** This method is optimized to use annotation groups. 
So can only be used for annotations that belongs to some object */
QList<AVAnnotationItemL*> AnnotationsTreeViewL::findAnnotationItems(const Annotation* a) const {
    assert(a->getGObject() != NULL && ctx->getAnnotationObjects().contains(a->getGObject()));

    QList<AVAnnotationItemL*> res;
    foreach(AnnotationGroup* g, a->getGroups()) {
        AVGroupItemL* gItem = findGroupItem(g);
        if(gItem == NULL) {
            return QList<AVAnnotationItemL*>();
        }
        AVAnnotationItemL* aItem = findAnnotationItem(gItem, a);
        res.append(aItem);
    }
    return res;
}

void AnnotationsTreeViewL::connectAnnotationSelection() {
    connect(ctx->getAnnotationsSelection(), 
        SIGNAL(si_selectionChanged(AnnotationSelection*, const QList<Annotation*>&, const QList<Annotation*>& )), 
        SLOT(sl_onAnnotationSelectionChanged(AnnotationSelection*, const QList<Annotation*>&, const QList<Annotation*>&)));
}

void AnnotationsTreeViewL::connectAnnotationGroupSelection() {
    connect(ctx->getAnnotationsGroupSelection(), 
        SIGNAL(si_selectionChanged(AnnotationGroupSelection*, const QList<AnnotationGroup*>&, const QList<AnnotationGroup*>& )), 
        SLOT(sl_onAnnotationGroupSelectionChanged(AnnotationGroupSelection*, const QList<AnnotationGroup*>&, const QList<AnnotationGroup*>&)));
}

void AnnotationsTreeViewL::sl_onItemSelectionChanged() {
    AnnotationSelection* as = ctx->getAnnotationsSelection();
    as->disconnect(this);
    as->clear();

    AnnotationGroupSelection* ags = ctx->getAnnotationsGroupSelection();
    ags->disconnect(this);
    ags->clear();


    QList<QTreeWidgetItem*> items = tree->selectedItems();
    foreach(QTreeWidgetItem* i, items) {
        AVItemL* item  = static_cast<AVItemL*>(i);
        if (item->type == AVItemType_Annotation) {
            AVAnnotationItemL* aItem = static_cast<AVAnnotationItemL*>(item);
            assert(aItem->annotation!=NULL);
            assert(aItem->annotation->getGObject()!=NULL);
            as->addToSelection(aItem->annotation);
        } else if (item->type == AVItemType_Group) {
            AVGroupItemL* gItem = static_cast<AVGroupItemL*>(item);
            assert(gItem->group!=NULL);
            ags->addToSelection(gItem->group);
        }
    }
    connectAnnotationSelection();
    connectAnnotationGroupSelection();
    updateState();
}

void AnnotationsTreeViewL::sl_onAnnotationSelectionChanged(AnnotationSelection*, const QList<Annotation*>& added, const QList<Annotation*>& removed) {
    tree->disconnect(this, SIGNAL(sl_onItemSelectionChanged()));

    foreach(Annotation* a, removed) {
        foreach(AnnotationGroup* g, a->getGroups()) {
            AVAnnotationItemL* item = findAnnotationItem(g, a);
            if (item && item->isSelected()) {
                item->setSelected(false);
            }
        }
    }
    AVAnnotationItemL* toVisible = NULL;
    QList<AVAnnotationItemL*> selectedItems;

    if(added.size() > 0 && !findAnnotationItem(added.first()->getGroups().first(), added.first())) {
        focusOnItem(added.first());
    }

    foreach(Annotation* a, added) {
        foreach(AnnotationGroup* g, a->getGroups()) {
            AVAnnotationItemL* item = findAnnotationItem(g, a);
            if (!item->isSelected()) {
                item->setSelected(true);
                selectedItems.append(item);
                /*for (QTreeWidgetItem* p = item->parent(); p!=NULL; p = p->parent()) {
                    if (!p->isExpanded()) {
                        p->setExpanded(true);
                    }
                }*/
            }
            toVisible = item;
        }
    }
    
    if( !selectedItems.isEmpty() ) {
        tree->setCurrentItem(selectedItems.first());
    }
    
    connect(tree, SIGNAL(itemSelectionChanged()), SLOT(sl_onItemSelectionChanged ()));

    //make item visible if special conditions are met
    if (toVisible != NULL && added.size() == 1) {
        tree->scrollToItem(toVisible, QAbstractItemView::EnsureVisible);
    }
    updateState();
}


void AnnotationsTreeViewL::sl_onAnnotationGroupSelectionChanged(AnnotationGroupSelection* s, 
                                                               const QList<AnnotationGroup*>& added, 
                                                               const QList<AnnotationGroup*>& removed) 
{
    Q_UNUSED(s);

    foreach(AnnotationGroup* g, removed) {
        AVGroupItemL* item = findGroupItem(g);
        if (item->isSelected()) {
            item->setSelected(false);
        }
    }

    foreach(AnnotationGroup* g, added) {
        AVGroupItemL* item = findGroupItem(g);
        if (!item->isSelected()) {
            item->setSelected(true);
        }
    }

    if (added.size() == 1) {
        AVGroupItemL* item = findGroupItem(added.first());
        tree->scrollToItem(item, QAbstractItemView::EnsureVisible);
    }
}


void AnnotationsTreeViewL::sl_onAnnotationObjectAdded(AnnotationTableObject* obj) {
    GTIMER(c2,t2,"AnnotationsTreeView::sl_onAnnotationObjectAdded");
    TreeSorter ts(this);
    
    assert(findGroupItem(obj->getRootGroup()) == NULL);
    LazyAnnotationTreeViewModel *model = static_cast<LazyAnnotationTreeViewModel *>(tree->model());
    AVGroupItemL *root = static_cast<AVGroupItemL *>(model->getItem(QModelIndex()));
    AVGroupItemL* groupItem = createGroupItem(root, obj->getRootGroup());
    if(tree->lineHeight == 1) {
        //tree->lineHeight = tree->rowHeight(model->guessIndex(groupItem));
        tree->setLineHeight(tree->rowHeight(tree->guessIndex(groupItem)));
    }
    tree->calculateIndex(obj);
    tree->realNumberOfItems++;
    tree->updateSlider();

    connect(obj, SIGNAL(si_onAnnotationsAdded(const QList<Annotation*>&)), SLOT(sl_onAnnotationsAdded(const QList<Annotation*>&)));
    connect(obj, SIGNAL(si_onAnnotationsRemoved(const QList<Annotation*>&)), SLOT(sl_onAnnotationsRemoved(const QList<Annotation*>&)));
    connect(obj, SIGNAL(si_onAnnotationModified(const AnnotationModification&)), SLOT(sl_onAnnotationModified(const AnnotationModification&)));
    connect(obj, SIGNAL(si_onAnnotationsInGroupRemoved(const QList<Annotation*>&, AnnotationGroup*)), 
        SLOT(sl_onAnnotationsInGroupRemoved(const QList<Annotation*>&, AnnotationGroup*)));

    connect(obj, SIGNAL(si_onGroupCreated(AnnotationGroup*)), SLOT(sl_onGroupCreated(AnnotationGroup*)));
    connect(obj, SIGNAL(si_onGroupRemoved(AnnotationGroup*, AnnotationGroup*)), SLOT(sl_onGroupRemoved(AnnotationGroup*, AnnotationGroup*)));
    connect(obj, SIGNAL(si_onGroupRenamed(AnnotationGroup*, const QString& )), SLOT(sl_onGroupRenamed(AnnotationGroup*, const QString& )));

    connect(obj, SIGNAL(si_modifiedStateChanged()), SLOT(sl_onAnnotationObjectModifiedStateChanged()));
    connect(obj, SIGNAL(si_nameChanged(const QString&)), SLOT(sl_onAnnotationObjectRenamed(const QString& )));
}

void AnnotationsTreeViewL::sl_onAnnotationObjectRemoved(AnnotationTableObject* obj) {
    TreeSorter ts(this);

    AVGroupItemL* groupItem = findGroupItem(obj->getRootGroup());
    if (groupItem) {
        destroyTree(groupItem);
        tree->removeItem(groupItem, true);
    }
    
    obj->disconnect(this);
}

void AnnotationsTreeViewL::sl_onAnnotationsInGroupRemoved(const QList<Annotation*>&, AnnotationGroup* group) {
    AVGroupItemL *parentGroupItem = findGroupItem(group);
    if(parentGroupItem != NULL) {
        tree->realNumberOfItems -= tree->getExpandedNumber(group);
        destroyTree(parentGroupItem);
        tree->removeItem(parentGroupItem);
    }
    group->getGObject()->releaseLocker();
}


void AnnotationsTreeViewL::sl_onAnnotationsAdded(const QList<Annotation*>& as) {
    GTIMER(c1,t1,"AnnotationsTreeView::sl_onAnnotationsAdded");
    TreeSorter ts(this);

    QSet<AVGroupItemL*> toUpdate;
    foreach(Annotation* a, as) {
        foreach(AnnotationGroup* ag, a->getGroups()) {
            AVGroupItemL* gi = findGroupItem(ag);
            if(!gi) {
                AnnotationGroup* childGroup = ag;
                while(true) {
                    gi = findGroupItem(childGroup->getParentGroup());
                    if (gi != NULL) {
                        break;
                    }
                    childGroup = childGroup->getParentGroup();
                }
            }
            toUpdate.insert(gi);
            //tree->realNumberOfItems++;
            tree->treeWalker->addItem(a, ag);
        }
    }
    GTIMER(c2,t2,"AnnotationsTreeView::sl_onAnnotationsAdded [updateVisual]");
    while (!toUpdate.isEmpty()) {
        AVGroupItemL* i= *toUpdate.begin();
        toUpdate.remove(i);
        i->updateVisual();
        AVGroupItemL* p = (AVGroupItemL*)(i->parent());
        if (p != NULL && p->parent() != NULL) {
            toUpdate.insert(p);
        }
    }
}

void AnnotationsTreeViewL::sl_onAnnotationsRemoved(const QList<Annotation*>& as) {
    TreeSorter ts(this);
    
    tree->disconnect(this, SIGNAL(sl_onItemSelectionChanged()));

    AnnotationTableObject* aObj = qobject_cast<AnnotationTableObject*>(sender());
    assert(aObj != NULL);
    AVGroupItemL* groupItem = findGroupItem(aObj->getRootGroup());    
    QSet<AVGroupItemL*> groupsToUpdate;

    foreach(Annotation* a, as) {
        assert(a->getGObject() == NULL);
        QList<AVAnnotationItemL*> aItems;
        groupItem->findAnnotationItems(aItems, a);
        foreach(AnnotationGroup *gr, a->getGroups()) {
            if(tree->treeWalker->isExpanded(gr)) {
                tree->realNumberOfItems--;
                tree->updateSlider();
            }
        }
        //assert(!aItems.isEmpty());
        foreach(AVAnnotationItemL* ai, aItems) {
            tree->treeWalker->deleteItem(a, (static_cast<AVGroupItemL*>(ai->parent())->getAnnotationGroup()));
            AVGroupItemL * parentGroup = static_cast<AVGroupItemL*>(ai->parent());
            groupsToUpdate.insert(parentGroup);
            tree->removeItem(ai);
        }
    }
    foreach(AVGroupItemL* g, groupsToUpdate) {
        g->updateVisual();
    }

    connect(tree, SIGNAL(itemSelectionChanged()), SLOT(sl_onItemSelectionChanged ()));
    
    sl_onItemSelectionChanged();
}

void AnnotationsTreeViewL::sl_onAnnotationModified(const AnnotationModification& md) {
    switch(md.type) {
        case AnnotationModification_NameChanged: 
        case AnnotationModification_LocationChanged:
            {
                QList<AVAnnotationItemL*> aItems = findAnnotationItems(md.annotation);
                //assert(!aItems.isEmpty());
                foreach(AVAnnotationItemL* ai, aItems) {
                    ai->updateVisual(ATVAnnUpdateFlag_BaseColumns);
                }
            }
            break;

        case AnnotationModification_QualifierRemoved:
            {
                const QualifierModification& qm = (const QualifierModification&)md;
                QList<AVAnnotationItemL*> aItems  = findAnnotationItems(qm.annotation);

                foreach(AVAnnotationItemL* ai, aItems) {
                    for(int i=0, n = ai->childCount(); i < n; i++) {
                        AVQualifierItemL* qi = static_cast<AVQualifierItemL*>(ai->child(i));
                        if (qi->qName == qm.qualifier.name && qi->qValue == qm.qualifier.value) {
                            tree->removeItem(qi);
                            tree->realNumberOfItems--;
                            tree->updateSlider();
                            break;
                        }
                    }
                    ai->updateVisual(ATVAnnUpdateFlag_QualColumns);
                    
                }
            }
            break;
        case AnnotationModification_QualifierAdded:
            {
                const QualifierModification& qm = (const QualifierModification&)md;
                QList<AVAnnotationItemL*> aItems  = findAnnotationItems(qm.annotation);
                foreach(AVAnnotationItemL* ai, aItems) {
                    if (ai->isExpanded() || ai->childCount() > 0 || renameFlag) { //if item was expanded - add real qualifier items

                        AVQualifierItemL* qi = new AVQualifierItemL(ai, qm.qualifier);
                        tree->insertItem(ai->childCount() - 1, qi, false);
                        tree->realNumberOfItems++;
                        tree->updateSlider();
                        tree->emptyExpand = true;
                        renameFlag = false;
                        tree->expand(tree->guessIndex(qi->parent()));
                        tree->emptyExpand = false;

                    } else {
                        ai->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator); //otherwise process indicator only
                    }
                }
            }
            break;
        case AnnotationModification_AddedToGroup:
            {
                const AnnotationGroupModification& gmd = (const AnnotationGroupModification&)md;
                AVGroupItemL* gi = findGroupItem(gmd.group);
                assert(gi!=NULL);
                createAnnotationItem(gi, gmd.annotation);
                gi->updateVisual();
            }
            break;

        case AnnotationModification_RemovedFromGroup:
            {
                const AnnotationGroupModification& gmd = (const AnnotationGroupModification&)md;
                AVAnnotationItemL* ai = findAnnotationItem(gmd.group, gmd.annotation);
                assert(ai!=NULL);
                AVGroupItemL* gi = dynamic_cast<AVGroupItemL*>(ai->parent());
                tree->removeItem(ai);
                gi->updateVisual();

            }
            break;
    }
}

void AnnotationsTreeViewL::sl_onGroupCreated(AnnotationGroup* g) {
    LazyAnnotationTreeViewModel *model = static_cast<LazyAnnotationTreeViewModel *>(tree->model());
    AVGroupItemL* pg = g->getParentGroup()== NULL ? static_cast<AVGroupItemL*>(model->getItem(QModelIndex())) : findGroupItem(g->getParentGroup());
    tree->treeWalker->addItem(g);
    /*if(pg && pg->childCount() == 0) {
        tree->insertItem(pg->parent()->indexOfChild(pg), pg);
    }*/
    if(pg != NULL) {
        tree->updateItem(pg);
        pg->updateVisual();
    }
}

void AnnotationsTreeViewL::sl_onGroupRemoved(AnnotationGroup*, AnnotationGroup* g) {
    AVGroupItemL *gr = findGroupItem(g);
    if (gr != NULL) {
        destroyTree(gr);
        tree->treeWalker->deleteItem(g);
        tree->realNumberOfItems -= tree->getExpandedNumber(gr) + 1;
        tree->removeItem(gr);
    }
}

void AnnotationsTreeViewL::sl_onGroupRenamed(AnnotationGroup* g, const QString&) {
    AVGroupItemL* gi = findGroupItem(g);
    if (gi != NULL) {
        gi->updateVisual();
    }
}

AVGroupItemL* AnnotationsTreeViewL::createGroupItem(AVGroupItemL* parentGroupItem, AnnotationGroup* g) {
    AVGroupItemL* groupItem = new AVGroupItemL(this, parentGroupItem, g);

    tree->insertItem(parentGroupItem->childCount() - 1, groupItem);

    if(!g->getAnnotations().isEmpty() || ! g->getSubgroups().isEmpty()) {
        groupItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
    groupItem->updateVisual();
    return groupItem;
}

AVAnnotationItemL* AnnotationsTreeViewL::createAnnotationItem(AVGroupItemL* parentGroup, Annotation* a, bool removeLast) {
    AVAnnotationItemL* annotationItem = new AVAnnotationItemL(parentGroup, a);
    
    tree->insertItem(parentGroup->childCount() - 1, annotationItem, removeLast);

    const QVector<U2Qualifier>& qualifiers = a->getQualifiers();
    if (!qualifiers.isEmpty()) {
        annotationItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
    return annotationItem;
}

class SettingsUpdater : public TreeWidgetVisitor {
public:
    SettingsUpdater(const QStringList& cs) : changedSettings(cs) {}
    
    bool isChildVisitRequired(QTreeWidgetItem* ti) {
        AVItemL* item = static_cast<AVItemL*>(ti);
        assert(item->type != AVItemType_Qualifier);
        return item->type == AVItemType_Group; //visit children only for group items
    }

    void visit(QTreeWidgetItem* ti) {
        AVItemL* item = static_cast<AVItemL*>(ti);
        if (item->type == AVItemType_Group) {
            AVGroupItemL* gi = static_cast<AVGroupItemL*>(item);
            gi->updateVisual();
        } else if (item->type == AVItemType_Annotation) {
            AVAnnotationItemL* ai = static_cast<AVAnnotationItemL*>(item);
            QString name = ai->annotation->getAnnotationName();
            if (changedSettings.contains(name)) {
                ai->updateVisual(ATVAnnUpdateFlag_BaseColumns);                
            }
        }
    }

private:
    QStringList changedSettings;
};
/*
void AnnotationsTreeViewL::sl_onAnnotationSettingsChanged(const QStringList& changedSettings) {
    // first clear colors cache for changed annotations
    QMap<QString, QIcon>& cache = AVAnnotationItemL::getIconsCache();
    foreach (const QString& name, changedSettings) {
        cache.remove(name);
    }
    // second -> update all annotations
    SettingsUpdater su(changedSettings);
    TreeWidgetUtils::visitDFS(tree, &su);
}*/

void AnnotationsTreeViewL::updateColumnContextActions(AVItemL* item, int col) {
    copyColumnTextAction->setEnabled(item!=NULL && (col >= 2 || (item->type == AVItemType_Annotation && col == 1)) && !item->text(col).isEmpty());
    copyColumnURLAction->setEnabled(item!=NULL && col >= 2 && item->isColumnLinked(col));
    if (!copyColumnTextAction->isEnabled()) {
        copyColumnTextAction->setText(tr("Copy column text"));
    } else {
        QString colName;
        if (col >= 2) {
            assert(qColumns.size() > col - 2);
            colName = qColumns[col - 2];
            copyColumnTextAction->setText(tr("Copy column '%1' text").arg(colName));
        } else {
            AVAnnotationItemL* ai = static_cast<AVAnnotationItemL*>(item);
            copyColumnTextAction->setText(tr("Copy '%1' annotation location").arg(ai->annotation->getAnnotationName()));
        }
    }

    if (!copyColumnURLAction->isEnabled()) {
        copyColumnURLAction->setText(tr("copy column URL"));
    } else {
        assert(qColumns.size() > col - 2);
        QString colName = qColumns[col - 2];
        copyColumnURLAction->setText(tr("Copy column '%1' URL").arg(colName));
    }
}

void AnnotationsTreeViewL::sl_onBuildPopupMenu(GObjectView*, QMenu* m) {
    // Add actions that not depends on the point popup is called
    adjustMenu(m);
    
    QPoint globalPos = QCursor::pos();
    QPoint treePos = tree->mapFromGlobal(globalPos);
    if (!tree->rect().contains(treePos)) {
        return;
    }
    
    // Check is popup is called for column header
    QHeaderView* header = tree->header();
    QPoint headerPoint = header->mapFromGlobal(globalPos);
    if (header->rect().contains(headerPoint)) {
        int idx = header->logicalIndexAt(headerPoint);
        if (idx >= 2) {
            assert(idx - 2 < qColumns.size());
            lastClickedColumn = idx;
            removeColumnByHeaderClickAction->setText(tr("Hide '%1' column").arg(qColumns[lastClickedColumn-2]));
            QAction* first = m->actions().first();
            m->insertAction(first, removeColumnByHeaderClickAction);
            m->insertSeparator(first);
        }
        return;
    }

    //Ensure that item clicked is in the tree selection. Do not destroy multi-selection if present
    QList<QTreeWidgetItem*> selItems = tree->selectedItems();
    QPoint viewportPos = tree->viewport()->mapFromGlobal(globalPos);
    if (selItems.size() <= 1) {
        QTreeWidgetItem* item = tree->itemAt(viewportPos);
        if (item!=NULL) {
            if (selItems.size() == 1 && selItems.first()!=item) {
                //tree->setItemSelected(selItems.first(), false);
                selItems.first()->setSelected(false);
            }
            if(!selItems.isEmpty()) {
                selItems.first()->setSelected(true);
            }
            //tree->setItemSelected(item, true);
        }
    }
    
    //Update column sensitive actions that appears only in context menu
    selItems = tree->selectedItems();
    lastClickedColumn = tree->columnAt(viewportPos.x());
    updateColumnContextActions(selItems.size() == 1 ? static_cast<AVItemL*>(selItems.first()) : static_cast<AVItemL*>(NULL), lastClickedColumn);

    //Add active context actions to the top level menu
    QList<QAction*> contextActions;
    contextActions << copyQualifierAction << copyQualifierURLAction 
        << toggleQualifierColumnAction << copyColumnTextAction 
        << copyColumnURLAction << editAction
;//        << cutAnnotationsAction << copyAnnotationsAction << pasteAnnotationsAction;
    
    QMenu* copyMenu = GUIUtils::findSubMenu(m, ADV_MENU_COPY);
    foreach(QAction* a, contextActions) {
        if (a->isEnabled()) {
            copyMenu->addAction(a);
        }
    }
    int nActive = 0;
    QAction* first = m->actions().first();
    foreach(QAction* a, contextActions) {
        if (a->isEnabled()) {
            nActive++;
            m->insertAction(first, a);
        }
    }
    if (nActive > 0) {
        m->insertSeparator(first);
    }
}

void AnnotationsTreeViewL::adjustMenu(QMenu* m) const {
    QMenu* addMenu = GUIUtils::findSubMenu(m, ADV_MENU_ADD);
    assert(addMenu!=NULL);
    addMenu->addAction(addAnnotationObjectAction);
    addMenu->addAction(addQualifierAction);

    QMenu* removeMenu = GUIUtils::findSubMenu(m, ADV_MENU_REMOVE);
    assert(removeMenu!=NULL);
    removeMenu->addAction(removeObjectsFromViewAction);
    removeMenu->addAction(removeAnnsAndQsAction);
}

void AnnotationsTreeViewL::sl_onAddAnnotationObjectToView() {
    ProjectTreeControllerModeSettings s;
    s.objectTypesToShow.append(GObjectTypes::ANNOTATION_TABLE);
    s.groupMode = ProjectTreeGroupMode_Flat;
    foreach(GObject* o, ctx->getObjects()) {
        s.excludeObjectList.append(o);
    }
    QList<GObject*> objs = ProjectTreeItemSelectorDialog::selectObjects(s, this);
    foreach(GObject* obj, objs) {
        assert(obj->getGObjectType() == GObjectTypes::ANNOTATION_TABLE);
        ctx->tryAddObject(obj);
    } 
}



static QList<AVGroupItemL*> selectGroupItems(const QList<QTreeWidgetItem*>& items, TriState readOnly, TriState rootOnly) {
    QList<AVGroupItemL*> res;
    foreach(QTreeWidgetItem* i, items) {
        AVItemL* item = static_cast<AVItemL*>(i);
        if (item->type == AVItemType_Group) {
            AVGroupItemL* gItem = static_cast<AVGroupItemL*>(item);
            if (rootOnly != TriState_Unknown) {
                bool groupIsRoot = gItem->parent()->parent() == NULL;
                if ( (rootOnly == TriState_Yes && !groupIsRoot) || (rootOnly==TriState_No && groupIsRoot)) {
                    continue;
                }
            }
            if (readOnly != TriState_Unknown) {
                bool groupReadOnly = gItem->isReadonly();
                if ( (readOnly == TriState_Yes && !groupReadOnly) || (readOnly==TriState_No && groupReadOnly)) {
                    continue;
                }
            }
            res.append(gItem);
        }
    }
    return res;
}

static QList<AVAnnotationItemL*> selectAnnotationItems(const QList<QTreeWidgetItem*>& items, TriState readOnly) {
    QList<AVAnnotationItemL*> res;
    foreach(QTreeWidgetItem* i, items) {
        AVItemL* item = static_cast<AVItemL*>(i);
        if (item->type == AVItemType_Annotation) {
            AVAnnotationItemL* aItem = static_cast<AVAnnotationItemL*>(item);
            if (readOnly != TriState_Unknown) {
                bool aReadOnly= aItem->isReadonly();
                if ( (readOnly == TriState_Yes && !aReadOnly) || (readOnly==TriState_No && aReadOnly)) {
                    continue;
                }
            }
            res.append(aItem);
        }
    }
    return res;
}

static QList<AVQualifierItemL*> selectQualifierItems(const QList<QTreeWidgetItem*>& items, TriState readOnly) {
    QList<AVQualifierItemL*> res;
    foreach(QTreeWidgetItem* i, items) {
        AVItemL* item = static_cast<AVItemL*>(i);
        if (item->type == AVItemType_Qualifier) {
            AVQualifierItemL* qItem = static_cast<AVQualifierItemL*>(item);
            if (readOnly != TriState_Unknown) {
                bool qReadOnly= qItem->isReadonly();
                if ( (readOnly == TriState_Yes && !qReadOnly) || (readOnly==TriState_No && qReadOnly)) {
                    continue;
                }
            }
            res.append(qItem);
        }
    }
    return res;
}

void AnnotationsTreeViewL::sl_removeObjectFromView() {
    QList<AVGroupItemL*> topLevelGroups = selectGroupItems(tree->selectedItems(), TriState_Unknown, TriState_Yes);
    QList<GObject*> objects;
    foreach(AVGroupItemL* gItem, topLevelGroups) {
        objects.append(gItem->group->getGObject());
    }
    foreach(GObject* obj, objects) {
        assert(obj->getGObjectType() == GObjectTypes::ANNOTATION_TABLE);
        ctx->removeObject(obj);
    }
}

static bool groupDepthInverseComparator(const AVGroupItemL* i1, const AVGroupItemL* i2) {
    int depth1 = i1->group->getGroupDepth();
    int depth2 = i2->group->getGroupDepth();
    return depth1 > depth2;
}

void AnnotationsTreeViewL::sl_removeAnnsAndQs() {
    //remove selected qualifiers first (cache them, since different qualifier items with equal name/val are not distinguished)
    QList<AVQualifierItemL*> qualifierItemsToRemove = selectQualifierItems(tree->selectedItems(), TriState_No);
    QVector<U2Qualifier>  qualsToRemove(qualifierItemsToRemove.size());
    QVector<Annotation*>  qualAnnotations(qualifierItemsToRemove.size());
    for(int i=0, n = qualifierItemsToRemove.size(); i<n ; i++) {
        AVQualifierItemL* qi = qualifierItemsToRemove[i];
        AVAnnotationItemL* ai = static_cast<AVAnnotationItemL*>(qi->parent());
        qualAnnotations[i] = ai->annotation;
        qualsToRemove[i] = U2Qualifier(qi->qName, qi->qValue);
    }
    for(int i=0, n = qualifierItemsToRemove.size(); i<n ; i++) {
        Annotation* a = qualAnnotations.at(i);
        const U2Qualifier& q = qualsToRemove.at(i);

        a->removeQualifier(q);
    }
    

    //remove selected annotations now
    QList<AVAnnotationItemL*> annotationItemsToRemove = selectAnnotationItems(tree->selectedItems(), TriState_No);
    QMultiMap<AnnotationGroup*, Annotation*> annotationsByGroup;
    foreach(AVAnnotationItemL* aItem, annotationItemsToRemove) {
        assert(!aItem->annotation->getGObject()->isStateLocked());
        AnnotationGroup* ag = (static_cast<AVGroupItemL*>(aItem->parent())->group);
        annotationsByGroup.insert(ag, aItem->annotation);
    }

    QList<AnnotationGroup*> agroups = annotationsByGroup.uniqueKeys();
    foreach(AnnotationGroup* ag, agroups)  {
        QList<Annotation*> annotations = annotationsByGroup.values(ag);
        ag->removeAnnotations(annotations);
    }


    //now remove selected groups
    QList<AVGroupItemL*> groupItemsToRemove = selectGroupItems(tree->selectedItems(), TriState_No, TriState_No); 
    
    qSort(groupItemsToRemove.begin(), groupItemsToRemove.end(), groupDepthInverseComparator);
    //now remove all groups
    foreach(AVGroupItemL* gi, groupItemsToRemove) {
        AnnotationGroup* pg = gi->group->getParentGroup();
        pg->removeSubgroup(gi->group);
    }
}

void AnnotationsTreeViewL::updateState() {
    QList<QTreeWidgetItem*> items = tree->selectedItems();

    QList<AVGroupItemL*> topLevelGroups = selectGroupItems(items, TriState_Unknown, TriState_Yes);

    foreach (AVGroupItemL* item, topLevelGroups) {
        AnnotationTableObject* aObj = item->getAnnotationTableObject();
        if (AutoAnnotationsSupport::isAutoAnnotation(aObj)) {
            break;
        }
    }

    removeObjectsFromViewAction->setEnabled(!topLevelGroups.isEmpty());

    QList<AVGroupItemL*> nonRootModGroups = selectGroupItems(items, TriState_No, TriState_No);
    QList<AVAnnotationItemL*> modAnnotations = selectAnnotationItems(items, TriState_No);
    QList<AVQualifierItemL*>  modQualifiers = selectQualifierItems(items, TriState_No);
    removeAnnsAndQsAction->setEnabled(!nonRootModGroups.isEmpty() || !modAnnotations.isEmpty() || !modQualifiers.isEmpty());

    bool hasOnly1QualifierSelected = items.size() == 1 && (static_cast<AVItemL*>(items.first()))->type == AVItemType_Qualifier;
    QString qName = hasOnly1QualifierSelected ? (static_cast<AVQualifierItemL*>(items.first()))->qName : QString("");

    copyQualifierAction->setEnabled(hasOnly1QualifierSelected);
    copyQualifierAction->setText(hasOnly1QualifierSelected ? tr("Copy qualifier '%1' value").arg(qName) : tr("Copy qualifier text"));

    bool hasOnly1QualifierSelectedWithURL = hasOnly1QualifierSelected && (static_cast<AVItemL*>(items.first()))->isColumnLinked(1);
    copyQualifierURLAction->setEnabled(hasOnly1QualifierSelectedWithURL);
    copyQualifierURLAction->setText(hasOnly1QualifierSelectedWithURL ? tr("Copy qualifier '%1' URL").arg(qName) : tr("Copy qualifier URL"));


    toggleQualifierColumnAction->setEnabled(hasOnly1QualifierSelected);
    bool hasColumn = qColumns.contains(qName);
    toggleQualifierColumnAction->setText(!hasOnly1QualifierSelected ? tr("Toggle column")
        : (qColumns.contains(qName) ? tr("Hide '%1' column"): tr("Add '%1' column")).arg(qName));

    toggleQualifierColumnAction->setIcon(hasOnly1QualifierSelected ? (hasColumn ? removeColumnIcon : addColumnIcon) : QIcon());

    QTreeWidgetItem * ciBase = tree->currentItem();
    AVItemL* ci = static_cast<AVItemL*>(ciBase);
    bool editableItemSelected = items.size() == 1 && ci!=NULL && ci == items.first() && !ci->isReadonly();
    renameAction->setEnabled(editableItemSelected);
    editAction->setEnabled(hasOnly1QualifierSelected && editableItemSelected);
    viewAction->setEnabled(hasOnly1QualifierSelected);

    bool hasEditableAnnotationContext = editableItemSelected && (ci->type == AVItemType_Annotation || ci->type == AVItemType_Qualifier);
    addQualifierAction->setEnabled(hasEditableAnnotationContext);
}

static bool isReadOnly(QTreeWidgetItem *item) {
    for (; item; item = item->parent()) {
        if(item->parent() == NULL) {
            continue;
        }
        AVItemL *itemi = dynamic_cast<AVItemL*>(item);
        AnnotationTableObject *obj;
        switch (itemi->type) {
            case AVItemType_Group: obj = dynamic_cast<AVGroupItemL*>(itemi)->group->getGObject(); break;
            case AVItemType_Annotation: obj = dynamic_cast<AVAnnotationItemL*>(itemi)->annotation->getGObject(); break;
            default: continue;
        }
        if (obj->isStateLocked())
            return true;
    }
    return false;
}

void AnnotationsTreeViewL::resetDragAndDropData() {
    dndCopyOnly = false;
    dndSelItems.clear();
    dropDestination = NULL;
}

bool AnnotationsTreeViewL::eventFilter(QObject* o, QEvent* e) {
    if (o != tree->viewport()) {
        return false;
    }
    QEvent::Type etype = e->type();
    switch (etype) {
        case QEvent::ToolTip: {
            QHelpEvent* he = (QHelpEvent*)e;
            QPoint globalPos = he->globalPos();
            QPoint viewportPos = tree->viewport()->mapFromGlobal(globalPos);
            QTreeWidgetItem* item = tree->itemAt(viewportPos);
            if (item != NULL) {
                AVItemL* avi = static_cast<AVItemL*>(item);
                if (avi->type == AVItemType_Annotation) {
                    AVAnnotationItemL* ai = static_cast<AVAnnotationItemL*>(avi);
                    ADVSequenceObjectContext* sc = ctx->getSequenceContext(ai->getAnnotationTableObject());
                    QString tip = ai->annotation->getQualifiersTip(15, 
                        sc?sc->getSequenceObject():NULL,
                        sc?sc->getComplementTT():NULL);
                    if (!tip.isEmpty()) {
                        QToolTip::showText(he->globalPos(), tip);
                        return true;
                    }
                }
            }
            return false;
        }
        case QEvent::MouseButtonRelease:
            lastMB = ((QMouseEvent*)e)->button();
            isDragging = false;
            return false;
        case QEvent::MouseButtonPress: {
            QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
            if (me->modifiers() == Qt::NoModifier && me->button() == Qt::LeftButton) {
                QTreeWidgetItem *item = tree->itemAt(dragStartPos = me->pos());
                if (item != NULL) {
                    AVItemTypeL type = dynamic_cast<AVItemL*>(item)->type;
                    if (type == AVItemType_Annotation || type == AVItemType_Group) {
                        if (!tree->selectedItems().contains(item)) {
                            tree->setCurrentItem(item);
                        }
                        isDragging = true;
                        return false;
                    }
                }
            }
            isDragging = false;
            return false;
        }
        case QEvent::MouseMove: {
            QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
            if (!(me->buttons() & Qt::LeftButton) || !isDragging) {
                return false;
            }
            if ((me->pos() - dragStartPos).manhattanLength() < QApplication::startDragDistance()) {
                return true;
            }
            initiateDragAndDrop(me);
            return true;
        }
        case QEvent::DragEnter: {
            QDragEnterEvent *de = dynamic_cast<QDragEnterEvent*>(e);
            if (de->mimeData()->hasFormat(annotationMimeType)) {
                de->acceptProposedAction();
                uiLog.trace("Drag enter event in Annotations view : format is OK");
                return true;
            }
            uiLog.trace("Drag enter event in Annotations view : unsupported mime format!");
            return false;
        }
        case QEvent::DragMove: { // update current D&D state
            QDragMoveEvent *de = dynamic_cast<QDragMoveEvent*>(e);
            if (de->mimeData()->hasFormat(annotationMimeType)) {
                QTreeWidgetItem *item = tree->itemAt(de->pos());
                if (item == NULL) {
                    de->ignore();
                    uiLog.trace("Drag event in Annotations view : invalid destination");
                    return true;
                }
                if (isReadOnly(item)) {
                    de->ignore();
                    uiLog.trace(QString("Drag event in Annotations view : destination is read-only: %1").arg(item->text(0)));
                    return true;
                }
                Qt::DropAction dndAction = (de->keyboardModifiers() & Qt::ShiftModifier) ? Qt::CopyAction : Qt::MoveAction;
                if (dndAction == Qt::MoveAction && dndCopyOnly) {
                    de->ignore();
                    uiLog.trace("Drag event in Annotations view : source is read-only, can't move");
                    return true;
                }
                
                for (AVItemL *itemi = dynamic_cast<AVItemL*>(item); itemi != NULL; itemi = dynamic_cast<AVItemL*>(itemi->parent())) {
                    if (itemi->type == AVItemType_Group) {
                        for (int i = 0, s = dndSelItems.size(); i < s; ++i) {
                            AVItemL* dndItem = dndSelItems[i];
                            if (dndItem == itemi) { // do not allow drop group into itself 
                                de->ignore();
                                uiLog.trace("Drag event in Annotations view : can't move group into itself!");
                                return true;
                            }
                        }
                    }
                }
                if (item->parent() == NULL) { //destination is root item
                    for (int i = 0, s = dndSelItems.size(); i < s; ++i) {
                        AVItemL* dndItem = dndSelItems[i];
                        if (dndItem->type == AVItemType_Annotation) {
                            de->ignore();
                            // root group can't have annotations -> problem with store/load invariant..
                            uiLog.trace("Drag event in Annotations view : can't move annotations, destination group is not found!");
                            return true;
                        }
                    }
                }
                de->setDropAction(dndAction);
                de->accept();
                uiLog.trace(QString("Drag event in Annotations view, status: OK, type: %1").arg(dndAction == Qt::MoveAction? "Move" : "Copy"));
                return true;
            }
            return false;
        }
        case QEvent::Drop: {
            uiLog.trace("Drop event in Annotations view");
            QDropEvent *de = dynamic_cast<QDropEvent*>(e);
            const QMimeData *mime = de->mimeData();
            if (mime->hasFormat(annotationMimeType)) {
                QTreeWidgetItem *item = tree->itemAt(de->pos());
                if (item != NULL) {
                    AVItemL *avItem= dynamic_cast<AVItemL*>(item);
                    while (avItem != NULL && avItem->type != AVItemType_Group) {
                        avItem = dynamic_cast<AVItemL*>(avItem->parent());
                    }
                    if (avItem != NULL) {
                        dropDestination = static_cast<AVGroupItemL*>(avItem);
                        if (de->keyboardModifiers() & Qt::ShiftModifier) {
                            de->setDropAction(Qt::CopyAction);
                        } else {
                            de->setDropAction(Qt::MoveAction);
                        }
                        de->accept();
                        uiLog.trace(QString("Drop event in Annotations view : destination group %1").arg(dropDestination->group->getGroupPath()));
                    } else {
                        uiLog.trace("Drop event in Annotations view : something weird, can't derive destination group!");
                    }
                    return true;
                } else {
                    uiLog.trace("Drop event in Annotations view : invalid destination");
                }
            } else {
                uiLog.trace("Drop event in Annotations view : unknown MIME format");
                return false;
            }

            de->ignore();
            return true;
        }
        default:
            return false;
    }
}


bool AnnotationsTreeViewL::initiateDragAndDrop(QMouseEvent*) {
    uiLog.trace("Starting drag & drop in annotations view");

    resetDragAndDropData();
    
    // Now filter selection: keep only parent items in the items list, remove qualifiers, check if 'move' is allowed
    QList<QTreeWidgetItem*> initialSelItems = tree->selectedItems(); // whole selection will be moved
    dndCopyOnly = false; // allow 'move' by default first
    for (int i = 0, n = initialSelItems.size(); i < n; i++) {
        AVItemL *itemi = dynamic_cast<AVItemL*>(initialSelItems[i]);
        if (itemi->parent() && !dndCopyOnly && isReadOnly(itemi)) {
            dndCopyOnly = true;
        }
        if (itemi->type == AVItemType_Annotation) { 
            // if annotation group is in the selection -> remove annotation from the list, it will be moved/copied automatically with group
            bool addItem = true;
            for (QTreeWidgetItem *cur = itemi->parent(); cur != NULL; cur = cur->parent()) {
                if (initialSelItems.contains(cur)) {
                    addItem = false;
                    break;
                }
            }
            if (addItem) {
                dndSelItems.append(itemi);
            }
        } else if (itemi->type == AVItemType_Qualifier) { // remove qualifiers from selection
            dndSelItems[i]->setSelected(false); 
        } else {
            assert(itemi->type == AVItemType_Group);
            if (itemi->parent() == NULL) { // object level group -> add all subgroups
                for (int j = 0, m = itemi->childCount(); j < m; j++) {
                    AVItemL* citem = dynamic_cast<AVItemL*>(itemi->child(j));
                    assert(citem->type == AVItemType_Group || citem->type == AVItemType_Annotation);
                    dndSelItems.append(citem);
                }
            } else {
                dndSelItems.append(itemi);
            }
        }
    }
    if (dndSelItems.isEmpty()) {
        resetDragAndDropData();
        uiLog.trace("No items to drag & drop");
        return false;
    }

    // serialize selected items : // TODO: actually we do not need it today, so skipping this step
    //QStringList annotationsAndGroupsRefs;
    QByteArray serializedReferences;
    serializedReferences.append(serializedReferences);
    
    // initiate dragging
    QMimeData* mimeData = new QMimeData();
    mimeData->setData(annotationMimeType, serializedReferences);
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    Qt::DropAction dndAction = drag->exec(Qt::MoveAction | Qt::CopyAction, Qt::CopyAction);
    if (dndAction == Qt::MoveAction || dndAction == Qt::CopyAction) {
        finishDragAndDrop(dndAction);
    } else {
        uiLog.trace("Drag & drop in Annotations view was ignored");
    }
    resetDragAndDropData();
    return true;
}

typedef QPair<AnnotationGroup*, QString> GroupChangeInfo; 
typedef QPair<GroupChangeInfo, Annotation*> AnnotationDnDInfo; 

static void collectAnnotationDnDInfo(AnnotationGroup* ag, const QString& destGroupPath, QList<AnnotationDnDInfo>& annotationsToProcess) {
    foreach(Annotation* a, ag->getAnnotations()) {
        annotationsToProcess << AnnotationDnDInfo(GroupChangeInfo(ag, destGroupPath), a);
    }
    QString newDestGroupPath = destGroupPath + "/" + ag->getGroupName();
    foreach(AnnotationGroup* sag, ag->getSubgroups()) {
        collectAnnotationDnDInfo(sag, newDestGroupPath, annotationsToProcess);
    }
}

void AnnotationsTreeViewL::finishDragAndDrop(Qt::DropAction dndAction) {
    AnnotationTableObject* dstObject = dropDestination->getAnnotationTableObject();
    QString destGroupPath = dropDestination->group->getGroupPath();

    QList<AnnotationGroup*> affectedGroups;
    QList<AnnotationDnDInfo> affectedAnnotations; 
    QStringList manualCreationGroups;

    for (int i = 0, n = dndSelItems.size(); i < n; ++i) {
        AVItemL* selItem = dndSelItems.at(i);
        assert(selItem->parent()!= NULL); // we never have no top-level object items in dndSelItems
        if (selItem->type == AVItemType_Annotation) {
            AVGroupItemL* fromGroupItem = dynamic_cast<AVGroupItemL*>(selItem->parent());
            AVAnnotationItemL* ai = dynamic_cast<AVAnnotationItemL*>(selItem);
            affectedAnnotations << AnnotationDnDInfo(GroupChangeInfo(fromGroupItem->group, destGroupPath), ai->annotation);
        } else {
            AVGroupItemL* movedGroupItem = dynamic_cast<AVGroupItemL*>(selItem);
            if (movedGroupItem->group->getParentGroup() == dropDestination->group) {
                continue; // can't drop group into itself
            }
            QString toGroupPath = destGroupPath + (destGroupPath.isEmpty() ? "" : "/") + movedGroupItem->group->getGroupName();
            if (movedGroupItem->group->getAnnotations().isEmpty()) {
                //this group will not be created as part of annotation move process -> create it manually
                manualCreationGroups.append(toGroupPath);
            }
            collectAnnotationDnDInfo(movedGroupItem->group, toGroupPath, affectedAnnotations);
            affectedGroups.append(movedGroupItem->group);
        }
    }

    uiLog.trace(QString("Finishing drag & drop in Annotations view, affected groups: %1 , top-level annotations: %2").arg(affectedGroups.size()).arg(affectedAnnotations.size()));

    // Move or Copy annotation reference inside of the object
    foreach(const AnnotationDnDInfo& adnd, affectedAnnotations) {
        const QString& toGroupPath = adnd.first.second;
        AnnotationGroup* dstGroup = dstObject->getRootGroup()->getSubgroup(toGroupPath, true);
        if (dstGroup == dstObject->getRootGroup()) { // root group can't have annotations -> problem with store/load invariant..
            continue;
        }
        AnnotationGroup* srcGroup = adnd.first.first;
        Annotation *srcAnnotation = adnd.second;
        Annotation* dstAnnotation = (dstObject == srcAnnotation->getGObject()) ? srcAnnotation : new Annotation(srcAnnotation->data());
        bool doAdd = !dstGroup->getAnnotations().contains(dstAnnotation);
        bool doRemove = dndAction == Qt::MoveAction && doAdd;
        if (doAdd) {
            dstGroup->addAnnotation(dstAnnotation);
        }
        if (doRemove) {
            srcGroup->removeAnnotation(srcAnnotation);
        }
    }
    // Process groups
    if (dndAction == Qt::MoveAction) {
        foreach(AnnotationGroup* ag, affectedGroups) {
            ag->getParentGroup()->removeSubgroup(ag);
        }
    }
    // manually create empty group items
    foreach(const QString& path, manualCreationGroups) {
        dstObject->getRootGroup()->getSubgroup(path, true);
    }
}

void AnnotationsTreeViewL::sl_itemEntered(QTreeWidgetItem * i, int column) {
    AVItemL* item = static_cast<AVItemL*>(i);
    Qt::CursorShape newShape = Qt::ArrowCursor;
    Qt::CursorShape currentShape = tree->cursor().shape();
    if (item != NULL) {
        if (item->isColumnLinked(column)) {
            newShape = Qt::PointingHandCursor;
        }
    }
    if (newShape == Qt::PointingHandCursor || ((newShape == Qt::ArrowCursor && currentShape == Qt::PointingHandCursor))) {
        tree->setCursor(newShape);
    }
}

void AnnotationsTreeViewL::sl_itemDoubleClicked(QTreeWidgetItem *i, int) {
    AVItemL* item = static_cast<AVItemL*>(i);
    if (item->type == AVItemType_Qualifier) {
        editItem(item);
    }
}

void AnnotationsTreeViewL::sl_itemClicked(QTreeWidgetItem * i, int column) {
    AVItemL* item = static_cast<AVItemL*>(i);
    if (lastMB != Qt::LeftButton || item==NULL || !item->isColumnLinked(column)) {
        return;
    }
    QString fileUrl = item->getFileUrl(column);
    if (!fileUrl.isEmpty()) {
        Task* task = new LoadRemoteDocumentAndOpenViewTask(fileUrl);
        AppContext::getTaskScheduler()->registerTopLevelTask(task);
    } else {
        GUIUtils::runWebBrowser(item->buildLinkURL(column));
    }
}

void AnnotationsTreeViewL::sl_itemExpanded(QTreeWidgetItem* qi) {
    int current = tree->onScreen.size();
    int lim = tree->getMaxItem();
    int itemsToAdd;
    AVItemL* i = static_cast<AVItemL*>(qi);
    if(qi->childCount() != 0) {
        return;
    }

    itemsToAdd = tree->getExpandedNumber(i);
    int acceptable = lim - current;
    int j = 0;
    for(;j < acceptable && j < itemsToAdd; j++) { //Add while there is space on viewport
        i = tree->getNextItemDown(i);
        tree->insertItem(i->parent()->childCount() - 1, i, false);
        if(tree->treeWalker->isExpanded(i)) {
            tree->emptyExpand = true;
            tree->expand(tree->guessIndex(i));
            tree->emptyExpand = false;
        }
    }

    QTreeWidgetItem *bottomItem = tree->onScreen.last();
    QTreeWidgetItem *lastInserted = i;

    for(; j< itemsToAdd;j++) {
        if(lastInserted == bottomItem) {
            break;
        }
        i = tree->getNextItemDown(i);
        tree->insertItem(i->parent()->childCount() - 1, i);
        if(tree->treeWalker->isExpanded(i)) {
            tree->emptyExpand = true;
            tree->expand(tree->guessIndex(i));
            tree->emptyExpand = false;
        }

        tree->removeItem(bottomItem);
        bottomItem = tree->onScreen.last();
        lastInserted = i;
    }
}

void AnnotationsTreeViewL::sl_itemCollapsed(QTreeWidgetItem* qi) {
    destroyTree(qi);
    qi->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    
    int current = tree->onScreen.size();
    int lim = tree->getMaxItem();
    int acceptable = lim - current;

    AVItemL *item =  NULL;
    if(acceptable > 0) {
        item = tree->getNextItemDown(static_cast<AVItemL*>(tree->onScreen.last()));
    }
    while(item && acceptable > 0) {
        tree->insertItem(item->parent()->childCount() - 1, item, false);
        if(tree->treeWalker->isExpanded(item)) {
            tree->expand(tree->guessIndex(item));
        }
        acceptable--;
        if(acceptable > 0) {
            item = tree->getNextItemDown(static_cast<AVItemL*>(tree->onScreen.last()));
        }
    }

    if(acceptable > 0) {
        item = tree->getNextItemUp();
    }
    while(item && acceptable > 0) {
        QTreeWidgetItem *last = tree->onScreen.first();
        if(item == last->parent() || item->parent()->parent() == NULL) {
            tree->onScreen.insert(0, item);
        } else {
            tree->insertItem(0,  item);
        }
        acceptable--;
        if(acceptable > 0) {
            item = tree->getNextItemUp();
        }
    }
}

void AnnotationsTreeViewL::destroyTree(QTreeWidgetItem* qi) {
    while(qi->childCount() > 0) {
        destroyTree(qi->child(0));
        tree->removeItem(qi->child(0));
    }
}

void AnnotationsTreeViewL::sl_onCopyQualifierValue() {
    QList<QTreeWidgetItem*> items = tree->selectedItems();
    assert(items.size() == 1);
    AVItemL* item = static_cast<AVItemL*>(items.first());
    assert(item->type == AVItemType_Qualifier);
    AVQualifierItemL* qi = static_cast<AVQualifierItemL*>(item);
    QApplication::clipboard()->setText(qi->qValue);
}

void AnnotationsTreeViewL::sl_onCopyQualifierURL() {
    QList<QTreeWidgetItem*> items = tree->selectedItems();
    assert(items.size() == 1);
    AVItemL* item = static_cast<AVItemL*>(items.first());
    if (item->isColumnLinked(1)) {
        QApplication::clipboard()->setText(item->buildLinkURL(1));
    }
}

void AnnotationsTreeViewL::sl_onCopyColumnText() {
    QList<QTreeWidgetItem*> items = tree->selectedItems();
    assert(items.size() == 1);
    AVItemL* item = static_cast<AVItemL*>(items.first());
    QApplication::clipboard()->setText(item->text(lastClickedColumn));
}

void AnnotationsTreeViewL::sl_onCopyColumnURL() {
    QList<QTreeWidgetItem*> items = tree->selectedItems();
    assert(items.size() == 1);
    AVItemL* item = static_cast<AVItemL*>(items.first());
    QApplication::clipboard()->setText(item->buildLinkURL(lastClickedColumn));
}

void AnnotationsTreeViewL::sl_onToggleQualifierColumn() {
    QList<QTreeWidgetItem*> items = tree->selectedItems();
    assert(items.size() == 1);
    AVItemL* item = static_cast<AVItemL*>(items.first());
    assert(item->type == AVItemType_Qualifier);
    AVQualifierItemL* qi = static_cast<AVQualifierItemL*>(item);
    if (qColumns.contains(qi->qName)) {
        removeQualifierColumn(qi->qName);
    } else {
        addQualifierColumn(qi->qName);
    }
}

void AnnotationsTreeViewL::sl_onRemoveColumnByHeaderClick() {
    assert(lastClickedColumn >= 2);
    assert(lastClickedColumn-2 <= qColumns.size());
    removeQualifierColumn(qColumns[lastClickedColumn-2]);
}


void AnnotationsTreeViewL::updateAllAnnotations(ATVAnnUpdateFlags flags) {
    QString emptyFilter;
    for(int i=0; i<tree->topLevelItemCount(); i++) {
        AVGroupItemL* gi = static_cast<AVGroupItemL*>(tree->topLevelItem(i));
        gi->updateAnnotations(emptyFilter, flags);
    }
}

void AnnotationsTreeViewL::addQualifierColumn(const QString& q) {
    TreeSorter ts(this);

    qColumns.append(q);
    int nColumns = headerLabels.size() + qColumns.size();
    //tree->setColumnCount(nColumns);
    LazyAnnotationTreeViewModel *lm = static_cast<LazyAnnotationTreeViewModel*>(tree->model());
    tree->setHeaderLabels(headerLabels + qColumns);
    lm->insertColumn(nColumns);
    
    tree->setColumnWidth(nColumns-2, nColumns - 2 == 1 ? 200 : 100);
    
    updateAllAnnotations(ATVAnnUpdateFlag_QualColumns);
    updateState();
}


void AnnotationsTreeViewL::removeQualifierColumn(const QString& q) {
    int ind = qColumns.indexOf(q);
    bool ok = qColumns.removeOne(q);
    if (!ok) {
        return;
    }

    TreeSorter ts(this);

    //tree->setColumnCount(headerLabels.size() + qColumns.size());
    LazyAnnotationTreeViewModel *lm = static_cast<LazyAnnotationTreeViewModel*>(tree->model());
    tree->setHeaderLabels(headerLabels + qColumns);
    lm->removeColumn(ind);
    
    updateAllAnnotations(ATVAnnUpdateFlag_QualColumns);

    updateState();
}

#define COLUMN_NAMES "ATV_COLUMNS"

void AnnotationsTreeViewL::saveState(QVariantMap& map) const {
    map.insert(COLUMN_NAMES, QVariant(qColumns));
    
    QStringList columns = map.value(COLUMN_NAMES).toStringList();
    assert(columns == qColumns);
}

void AnnotationsTreeViewL::updateState(const QVariantMap& map) {
    QStringList columns = map.value(COLUMN_NAMES).toStringList();
    //QByteArray geom = map.value(COLUMNS_GEOM).toByteArray();
    
    if (columns != qColumns && !columns.isEmpty()) {
        TreeSorter ts(this);
        foreach(QString q, qColumns) {
            removeQualifierColumn(q);
        }
        foreach(QString q, columns) {
            addQualifierColumn(q);
        }
    }
    /*if (columns == qColumns && !geom.isEmpty()) {
        tree->header()->restoreState(geom);
    }*/
}

void AnnotationsTreeViewL::setSortingEnabled(bool v) {
    if (sortTimer.isActive()) {
        sortTimer.stop();
    }
    if (v) {
        sortTimer.start();
    } else {
        tree->setSortingEnabled(false);
    }
}
/*
void AnnotationsTreeView::sl_sortTree() {
    tree->setSortingEnabled(true);
}*/

void AnnotationsTreeViewL::sl_rename() {
    AVItemL* item = static_cast<AVItemL*>(tree->currentItem());
    renameItem(item);
}

void AnnotationsTreeViewL::sl_edit() {
    AVItemL* item = static_cast<AVItemL*>(tree->currentItem());
    if (item != NULL) {
        editItem(item);
    }
}

void AnnotationsTreeViewL::editItem(AVItemL* item) {
    //warn: item could be readonly here -> used just for viewing advanced context
    if (item->type == AVItemType_Qualifier) {
        AVQualifierItemL* qi  = static_cast<AVQualifierItemL*>(item);
        AVAnnotationItemL* ai = static_cast<AVAnnotationItemL*>(qi->parent());
        U2Qualifier q;
        bool ro = qi->isReadonly();
        bool ok = editQualifierDialogHelper(qi, ro, q);
        if (!ro && ok && (q.name!=qi->qName || q.value!=qi->qValue)) {
            Annotation* a = (static_cast<AVAnnotationItemL*>(qi->parent()))->annotation;
            renameFlag = true;
            a->removeQualifier(qi->qName, qi->qValue);
            a->addQualifier(q);
            AVQualifierItemL* qi = ai->findQualifierItem(q.name, q.value);
            tree->setCurrentItem(qi);
            tree->scrollToItem(qi);
        }  
    }
}

void AnnotationsTreeViewL::moveDialogToItem(QTreeWidgetItem* item, QDialog& d) {
    if (item == NULL) {
        return;
    }
    tree->scrollToItem(item);

    //try place dialog right below or right above the item
    d.layout()->update();
    /*QRect itemRect = tree->visualItemRect(item).translated(tree->viewport()->mapToGlobal(QPoint(0, 0)));
    QSize dialogSize = d.layout()->minimumSize();
    QRect dialogRect(0, 0, dialogSize.width(), dialogSize.height() + 35); //+35 -> estimation for a title bar
    QRect widgetRect = rect().translated(mapToGlobal(QPoint(0, 0)));
    QRect finalDialogRect = dialogRect.translated(itemRect.bottomLeft());
    if (!widgetRect.contains(finalDialogRect)) {
        finalDialogRect = dialogRect.translated(itemRect.topLeft()).translated(QPoint(0, -dialogRect.height()));
    }
    if (widgetRect.contains(finalDialogRect)) {
        d.move(finalDialogRect.topLeft());
    }*/
}

QString AnnotationsTreeViewL::renameDialogHelper(AVItemL* i, const QString& defText, const QString& title) {
    QDialog d(this);
    d.setWindowTitle(title);
    QVBoxLayout* l = new QVBoxLayout();
    d.setLayout(l);

    QLineEdit* edit = new QLineEdit(&d);
    edit->setText(defText);
    edit->setSelection(0, defText.length());
    connect(edit, SIGNAL(returnPressed()), &d, SLOT(accept()));
    l->addWidget(edit);

    moveDialogToItem(i, d);

    int rc = d.exec();
    if (rc == QDialog::Rejected) {
        return defText;
    }
    return edit->text();
}

bool AnnotationsTreeViewL::editQualifierDialogHelper(AVQualifierItemL* i, bool ro, U2Qualifier& q) {
    EditQualifierDialog d(this, U2Qualifier(i == NULL ? "new_qualifier" : i->qName , i == NULL ? "" : i->qValue), ro, i != NULL);
    moveDialogToItem(i == NULL ? tree->currentItem() : i, d);
    int rc = d.exec();
    q = d.getModifiedQualifier();
    return rc == QDialog::Accepted;
}

void AnnotationsTreeViewL::renameItem(AVItemL* item) {
    if (item->isReadonly()) {
        return;
    }
    if (item->type == AVItemType_Group) {
        AVGroupItemL* gi = static_cast<AVGroupItemL*>(item);
        assert(gi->group->getParentGroup()!=NULL); //not a root group
        QString oldName = gi->group->getGroupName();
        QString newName = renameDialogHelper(item, oldName, tr("Rename group"));
        if (newName != oldName && AnnotationGroup::isValidGroupName(newName, false) 
            && gi->group->getParentGroup()->getSubgroup(newName, false) == NULL) 
        {
            gi->group->setGroupName(newName);
            gi->updateVisual();
        }
    } else if (item->type == AVItemType_Annotation) {
        AVAnnotationItemL* ai = static_cast<AVAnnotationItemL*>(item);
        QVector<U2Region> l = ai->annotation->getRegions();
        QList<ADVSequenceObjectContext*> soList = ctx->findRelatedSequenceContexts(ai->annotation->getGObject());
        assert(soList.size() == 1);
        ADVSequenceObjectContext* so = soList.first();
        U2Region seqRange(0, so->getSequenceObject()->getSequenceLength());
        EditAnnotationDialogController dialog(ai->annotation, seqRange, this);
        moveDialogToItem(ai, dialog);
        int result = dialog.exec();
        if(result == QDialog::Accepted){

            QString newName = dialog.getName();
            if (newName!=ai->annotation->getAnnotationName()) {
                ai->annotation->setAnnotationName(newName);
                QList<AVAnnotationItemL*> ais = findAnnotationItems(ai->annotation);
                foreach(AVAnnotationItemL* a, ais) {
                    a->updateVisual(ATVAnnUpdateFlag_BaseColumns);
                }
            }
            U2Location location = dialog.getLocation();
            if( !location->regions.isEmpty() && l != location->regions){
                ai->annotation->replaceRegions(location->regions);
            }
            ai->annotation->setLocationOperator(location->op);
            ai->annotation->setStrand(location->strand);
            ai->annotation->getGObject()->setModified(true);
        }
    } else {
        assert(item->type == AVItemType_Qualifier);
        AVQualifierItemL* qi = static_cast<AVQualifierItemL*>(item);
        AVAnnotationItemL* ai = static_cast<AVAnnotationItemL*>(qi->parent());
        QString newName = renameDialogHelper(item, qi->qName, tr("Rename qualifier"));
        if (newName != qi->qName) {
            Annotation* a = (static_cast<AVAnnotationItemL*>(qi->parent()))->annotation;
            QString val = qi->qValue;
            renameFlag = true;
            a->removeQualifier(qi->qName, val);
            a->addQualifier(newName, val);
            AVQualifierItemL* qi = ai->findQualifierItem(newName, val);
            tree->setCurrentItem(qi);
            tree->scrollToItem(qi);
        }  
    }
}

void AnnotationsTreeViewL::sl_addQualifier() {
    AVItemL* item = static_cast<AVItemL*>(tree->currentItem());
    if (item->isReadonly() || item->type == AVItemType_Group) {
        return;
    }
    U2Qualifier q;
    bool ok = editQualifierDialogHelper(NULL, false, q);
    if (ok) {
        assert(!q.name.isEmpty());
        AVAnnotationItemL* ai = item->type == AVItemType_Annotation ? static_cast<AVAnnotationItemL*>(item) : static_cast<AVAnnotationItemL*>(item->parent());
        Annotation* a = ai->annotation;
        a->addQualifier(q);
        //ai->setExpanded(true);
        AVQualifierItemL* qi = new AVQualifierItemL(ai, q);
        //sl_itemExpanded(ai);
        tree->expand(tree->guessIndex(qi->parent()));
        //AVQualifierItemL* qi = ai->findQualifierItem(q.name, q.value);
        tree->setCurrentItem(qi);
        tree->scrollToItem(qi);
    }
}

/*void AnnotationsTreeView::sl_cutAnnotations() {
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    QList<QTreeWidgetItem*> selItems = tree->selectedItems();
    int s = selItems.size();
    dataStream << true;
    dataStream << s;
    for (int i = 0; i < s; ++i) {
        AVItemL *itemi = dynamic_cast<AVItemL*>(selItems[i]);
        dataStream << (itemi->type == AVItemType_Group);
        if (itemi->type == AVItemType_Annotation)
            dataStream << *dynamic_cast<const AVAnnotationItemL*>(itemi)->annotation->data();
        else
            dataStream << *dynamic_cast<const AVGroupItemL*>(itemi)->group;
    }
    QMimeData *mimeData = new QMimeData;
    mimeData->setData(annotationMimeType, itemData);
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);
}

void AnnotationsTreeView::sl_copyAnnotations() {
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    QList<QTreeWidgetItem*> selItems = tree->selectedItems();
    int s = selItems.size();
    dataStream << false;
    dataStream << s;
    for (int i = 0; i < s; ++i) {
        AVItemL *itemi = dynamic_cast<AVItemL*>(selItems[i]);
        dataStream << (itemi->type == AVItemType_Group);
        if (itemi->type == AVItemType_Annotation)
            dataStream << *dynamic_cast<const AVAnnotationItemL*>(itemi)->annotation->data();
        else
            dataStream << *dynamic_cast<const AVGroupItemL*>(itemi)->group;
    }
    QMimeData *mimeData = new QMimeData;
    mimeData->setData(annotationMimeType, itemData);
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);
}

void AnnotationsTreeView::sl_pasteAnnotations() {
}*/

void AnnotationsTreeViewL::sl_onAnnotationObjectModifiedStateChanged() {
    AnnotationTableObject* ao = qobject_cast<AnnotationTableObject*>(sender());
    assert(ao!=NULL);
    AVGroupItemL* gi = findGroupItem(ao->getRootGroup());
    assert(gi!=NULL);
    gi->updateVisual();
}

void AnnotationsTreeViewL::sl_onAnnotationObjectRenamed(const QString&) {
    AnnotationTableObject* ao = qobject_cast<AnnotationTableObject*>(sender());
    AVGroupItemL* gi = findGroupItem(ao->getRootGroup());
    if (gi != NULL) {
        gi->updateVisual();
        tree->updateItem(gi); //TODO: why manual update is required?
    }
}

AVItemL* AnnotationsTreeViewL::currentItem(){
    return static_cast<AVItemL*>(tree->currentItem());
}



QList<QTreeWidgetItem*> LazyTreeView::selectedItems() {
    QList<QTreeWidgetItem *> list;
    LazyAnnotationTreeViewModel *modell = static_cast<LazyAnnotationTreeViewModel*>(model());
    foreach(const QModelIndex &index, selectedIndexes()) {
        QTreeWidgetItem* item = (QTreeWidgetItem*)modell->getItem(index);
        if(!list.contains(item)) {
            list << item;
        }
    }
    return list;
}
QTreeWidgetItem* LazyTreeView::currentItem() {
    QModelIndex index = currentIndex();
    if(index.isValid()) {
        LazyAnnotationTreeViewModel *modell = static_cast<LazyAnnotationTreeViewModel*>(model());
        QTreeWidgetItem*item =  (QTreeWidgetItem*)modell->getItem(index);
        return item;
    } else {
        return NULL;
    }
}

void LazyTreeView::scrollToItem(QTreeWidgetItem *item, ScrollHint hint) {
    QModelIndex index = guessIndex(item);
    scrollTo(index, hint);
}

void LazyTreeView::setCurrentItem(QTreeWidgetItem *item) {
    QModelIndex index = guessIndex(item);
    setCurrentIndex (index);
}

int LazyTreeView::topLevelItemCount() const {
    LazyAnnotationTreeViewModel *modell = static_cast<LazyAnnotationTreeViewModel*>(model());
    return modell->getItem(QModelIndex())->childCount();
}

QTreeWidgetItem * LazyTreeView::topLevelItem(int i) {
    LazyAnnotationTreeViewModel *modell = static_cast<LazyAnnotationTreeViewModel*>(model());
    AVItemL *root = modell->getItem(QModelIndex());
    return (QTreeWidgetItem *)(root->child(i));
}

void LazyTreeView::setColumnCount(int nCol) {
    LazyAnnotationTreeViewModel *modell = static_cast<LazyAnnotationTreeViewModel*>(model());
    modell->setColumnCount(nCol);
}

void LazyTreeView::setHeaderLabels(QStringList headers) {
    LazyAnnotationTreeViewModel *modell = static_cast<LazyAnnotationTreeViewModel*>(model());
    modell->setHeaderLabels(headers);
}

QTreeWidgetItem *LazyTreeView::itemAt(const QPoint pos) {
    QModelIndex index = indexAt(pos);
    LazyAnnotationTreeViewModel *modell = static_cast<LazyAnnotationTreeViewModel*>(model());
    return (QTreeWidgetItem*)(modell->getItem(index));
}

void LazyTreeView::scrollContentsBy ( int, int dy ) {
    int numToScroll = qAbs(dy);
    if(flag) {
        flag = false;
        return;
    }

    CustomSlider *slider = static_cast<CustomSlider*>(verticalScrollBar());

    if(slider->numToScroll() != 0) {
        numToScroll = qBound(0, slider->numToScroll(), slider->getMaxVal());
        slider->resetNumToScroll();
    }

    int pos = slider->sliderCustomPosition();
    coreLog.trace("old position was: " + QString::number(pos));
    coreLog.trace("numToScroll was: " + QString::number(numToScroll));
    if(dy < 0) {
        int res = 1;
        while(numToScroll > 0 && res) {
            res = scrollOneItemDown();
            numToScroll -= res;
            pos += res;
        }
    } else {
        int res = 1;
        while(numToScroll > 0 && res) {
            res = scrollOneItemUp();
            numToScroll -= res;
            pos -= res;
        }
    }

    if(indexAt(QPoint(0,0)) != guessIndex(onScreen.first())) {
        flag = true;
        emptyExpand = true;
        //expand(guessIndex(onScreen.first()->parent())); //if don't expand parent, qtreeview itself will expand it and resize scroll bar
        //emptyExpand = false;
        scrollTo(guessIndex(onScreen.first()), QAbstractItemView::PositionAtTop);
        emptyExpand = false;
        
    }
    slider->setPosition(pos);
    coreLog.trace("new position was set to: " + QString::number(pos));
    slider->update();
    viewport()->update();
}

int LazyTreeView::scrollOneItemUp() {
    AVItemL *item = getNextItemUp();
    assert(item != NULL);
    QTreeWidgetItem *last = onScreen.first();
    QTreeWidgetItem *toDelete = onScreen.last();
    if(item == last->parent() || item->parent()->parent() == NULL) {
        onScreen.insert(0, item);
    } else {
        insertItem(0, item);
    }
    removeItem(toDelete);

    return 1;
}

int LazyTreeView::scrollOneItemDown() {
    AVItemL *item = getNextItemDown(static_cast<AVItemL*>(onScreen.last()));
    if(item) {
        QTreeWidgetItem *toDelete = onScreen.first();
        insertItem(item->parent()->childCount() - 1, item, false);
        if(treeWalker->isExpanded(item)) {
            emptyExpand = true;
            expand(guessIndex(item));
            emptyExpand = false;
        }

        QTreeWidgetItem *parentToDelete = toDelete->parent();
        if(toDelete->childCount() == 0) {
            removeItem(toDelete);
        }
        while(parentToDelete->childCount() == 0) {
            toDelete = parentToDelete;
            parentToDelete = parentToDelete->parent();
            removeItem(toDelete);
        }
        return 1;
    } else {
        return 0;
    }
}

int LazyTreeView::getExpandedNumber(AnnotationGroup *gr) {
    int num = 0;
    num += gr->getSubgroups().size() + gr->getAnnotations().size();

    foreach(AnnotationGroup *g, gr->getSubgroups()) {
        if(treeWalker->isExpanded(g)) {
            num += getExpandedNumber(g);
        }
    }
    foreach(Annotation *a, gr->getAnnotations()) {
        if(treeWalker->isExpanded(a, gr)) {
            num += a->getQualifiers().size();
        }
    }
    return num;
}

int LazyTreeView::getExpandedNumber(AVItemL *item) {
    int num = 0;
    AVGroupItemL *groupItem;
    AVAnnotationItemL *annItem;
    switch(item->type) {
        case AVItemType_Group:
            groupItem = static_cast<AVGroupItemL *>(item);
            num += groupItem->getAnnotationGroup()->getSubgroups().size() + groupItem->getAnnotationGroup()->getAnnotations().size();
            foreach(AnnotationGroup *gr, groupItem->getAnnotationGroup()->getSubgroups()) {
                if(treeWalker->isExpanded(gr)) {
                    num += getExpandedNumber(gr);
                }
            }
            foreach(Annotation *a, groupItem->getAnnotationGroup()->getAnnotations()) {
                if(treeWalker->isExpanded(a, groupItem->getAnnotationGroup())) {
                    num += a->getQualifiers().size();
                }
            }
            break;
        case AVItemType_Annotation:
            annItem = static_cast<AVAnnotationItemL*>(item);
            num += annItem->annotation->getQualifiers().size();
            break;
        default:
            assert(0);
    }
    return num; 
}

void LazyTreeView::sl_expanded(const QModelIndex &index) {
    if(emptyExpand) {
        emptyExpand = false;
        return;
    }
    LazyAnnotationTreeViewModel *modell = static_cast<LazyAnnotationTreeViewModel*>(model());
    QTreeWidgetItem *item = modell->getItem(index);
    treeWalker->expand(modell->getItem(index));
    emit itemExpanded(item);
    
    realNumberOfItems += getExpandedNumber(static_cast<AVItemL*>(item));
    updateSlider();
}

void LazyTreeView::sl_collapsed(const QModelIndex &index) {
    LazyAnnotationTreeViewModel *modell = static_cast<LazyAnnotationTreeViewModel*>(model());
    QTreeWidgetItem *item = modell->getItem(index);

    treeWalker->collapse(modell->getItem(index));
    realNumberOfItems -= getExpandedNumber(static_cast<AVItemL*>(item));

    updateSlider();
    emit itemCollapsed(item);
}

void LazyTreeView::insertItemBehindView(int row, QTreeWidgetItem *item) {
    QTreeWidgetItem *parent = item->parent();
    LazyAnnotationTreeViewModel *lm = static_cast<LazyAnnotationTreeViewModel*>(model());
    lm->nextToAdd = static_cast<AVItemL*>(item);

    lm->debugFlag = true;
    lm->insertRow(row, guessIndex(parent));
}

void LazyTreeView::insertItem(int row, QTreeWidgetItem *item, bool removeLast) {

    QTreeWidgetItem *parent = item->parent();
    LazyAnnotationTreeViewModel *lm = static_cast<LazyAnnotationTreeViewModel*>(model());
    lm->nextToAdd = static_cast<AVItemL*>(item);

    lm->debugFlag = true;
    lm->insertRow(row, guessIndex(parent));

    int ind;
    if(onScreen.indexOf(parent) != -1) {
        if(row > 0) {
        QTreeWidgetItem* prevChild = parent->child(row - 1);
        while(prevChild->childCount() > 0) {
            prevChild = prevChild->child(prevChild->childCount() -1);
        }
        ind = onScreen.indexOf(prevChild) + 1;
        } else {
            ind = onScreen.indexOf(parent) + 1;
        }
    } else if(row > 0){
        ind = numOnScreen;
    } else {
        ind = 0;
    }
    onScreen.insert(ind, item);
    if(onScreen.size() > numOnScreen) {
        if(removeLast) {
            onScreen.removeLast();
        } else {
            onScreen.removeFirst();
        }
    }
}

void LazyTreeView::removeItem(QTreeWidgetItem *item, bool removeAll) {
    QTreeWidgetItem *parent = item->parent();
    LazyAnnotationTreeViewModel *lm = static_cast<LazyAnnotationTreeViewModel*>(model());

    onScreen.removeOne(item);
    if(item->parent()->parent() != NULL || removeAll) {
        lm->debugFlag = true;
        if(currentItem() == item) {
            setCurrentIndex(QModelIndex());
        }
        lm->removeRow(parent->indexOfChild(item), guessIndex(parent));
    }
}

AVItemL * LazyTreeView::getNextItemDown(AVItemL *bottom){ //Get item next down from bottom
    SAFE_POINT(bottom != NULL, "NULL current item", NULL);
    AVItemL *nextItem = NULL;
    AVQualifierItemL *currentQual = NULL;
    AVAnnotationItemL *currentAnn = NULL;
    AVGroupItemL *currentGroup = NULL;
    AVGroupItemL *parentGroup = NULL;
    U2Qualifier qual;
    int index;
    switch(bottom->type) {
        case AVItemType_Qualifier: //Current item is qualifier
            currentQual = static_cast<AVQualifierItemL*>(bottom);
            currentAnn = static_cast<AVAnnotationItemL*>(currentQual->parent());
            qual = U2Qualifier(currentQual->qName, currentQual->qValue);
            index = currentAnn->annotation->getQualifiers().indexOf(qual);
            if(index < currentAnn->annotation->getQualifiers().size() - 1) { //Next item is nearby qualifier
                nextItem = new AVQualifierItemL(currentAnn, currentAnn->annotation->getQualifiers()[index + 1]);
                return nextItem;
            } //If not go to higher level
        case AVItemType_Annotation:  
            if(currentAnn == NULL) {
                currentAnn = static_cast<AVAnnotationItemL*>(bottom);
                parentGroup = static_cast<AVGroupItemL*>(currentAnn->parent());
                bool f = treeWalker->isExpanded(currentAnn->annotation, parentGroup->getAnnotationGroup());
                if(f) { //If item is expanded, get first child
                    nextItem = new AVQualifierItemL(currentAnn, currentAnn->annotation->getQualifiers()[0]);
                    return nextItem;
                }
            }
            currentGroup = static_cast<AVGroupItemL*>(currentAnn->parent());
            index = currentGroup->getAnnotationGroup()->getAnnotations().indexOf(currentAnn->annotation);//next item is annotation
            if(index < currentGroup->getAnnotationGroup()->getAnnotations().size() - 1) {
                nextItem = new AVAnnotationItemL(currentGroup, currentGroup->getAnnotationGroup()->getAnnotations()[index + 1]);
                return nextItem;
            } //If not go to higher level
        case AVItemType_Group:
            if(currentGroup == NULL) {
                currentGroup = static_cast<AVGroupItemL*>(bottom);
                if(treeWalker->isExpanded(currentGroup->getAnnotationGroup())) {//If item is expanded get first child
                    if(currentGroup->getAnnotationGroup()->getAnnotations().isEmpty()) {
                        if(!currentGroup->getAnnotationGroup()->getSubgroups().isEmpty()) {
                            nextItem = new AVGroupItemL(currentGroup->getAnnotationTreeView(), currentGroup, currentGroup->getAnnotationGroup()->getSubgroups()[0]);
                        } else {
                            return NULL;
                        }
                    } else {
                        nextItem = new AVAnnotationItemL(currentGroup, currentGroup->getAnnotationGroup()->getAnnotations()[0]);
                    }
                    return nextItem;
                }
            } //Or find next group 
            parentGroup = static_cast<AVGroupItemL*>(currentGroup->parent()); //Need custom case when currentGroup is top-level item
            while(parentGroup->parent()) {
                index = parentGroup->getAnnotationGroup()->getSubgroups().indexOf(currentGroup->getAnnotationGroup());
                if(index < parentGroup->getAnnotationGroup()->getSubgroups().size() - 1) {
                    nextItem = new AVGroupItemL(parentGroup->getAnnotationTreeView(), parentGroup, parentGroup->getAnnotationGroup()->getSubgroups()[index + 1]);
                    return nextItem;
                }
                currentGroup = parentGroup;
                parentGroup = static_cast<AVGroupItemL*>(currentGroup->parent());
            }
            index = parentGroup->indexOfChild(currentGroup); // Reach root item
            if(++index < parentGroup->childCount()) {
                return static_cast<AVItemL*>(parentGroup->child(index));
            }
            break;
        default:
            assert(false);
    }
    return NULL; //reached end of tree
}

AVItemL * LazyTreeView::getLastItemInSubtree(AnnotationGroup *gr, AnnotationsTreeViewL *view) {
    if(gr->getSubgroups().isEmpty()) {
        Annotation *a = gr->getAnnotations().last();
        if(treeWalker->isExpanded(a, gr)) {
            AVAnnotationItemL *annItem = view->findAnnotationItem(gr, a);
            AVGroupItemL *upperGroup = view->findGroupItem(gr);
            if(upperGroup == NULL) {
                AVGroupItemL *pGroup = view->findGroupItem(gr->getParentGroup());
                assert(pGroup);
                upperGroup = new AVGroupItemL(view, pGroup, gr);
                insertItemBehindView(0, upperGroup);
            }
            if(annItem == NULL) {
                annItem = new AVAnnotationItemL(upperGroup, a);
                insertItemBehindView(upperGroup->childCount() - 1, annItem);
                if(treeWalker->isExpanded(a, gr)) {
                    emptyExpand = true;
                    expand(guessIndex(annItem));
                    emptyExpand = true;
                    expand(guessIndex(upperGroup));
                    emptyExpand = false;
                }
            }
            return new AVQualifierItemL(annItem, a->getQualifiers().last());
        } else {
            AVGroupItemL *parentGroup = view->findGroupItem(gr);
            AVGroupItemL *upperGroup = view->findGroupItem(gr->getParentGroup());
            if(parentGroup == NULL) {
                parentGroup = new AVGroupItemL(view, upperGroup, gr);
                insertItemBehindView(/*upperGroup->childCount() - 1*/0, parentGroup);
                if(treeWalker->isExpanded(gr)) {
                    emptyExpand = true;
                    expand(guessIndex(parentGroup));
                    emptyExpand = false;
                }
            }
            return new AVAnnotationItemL(parentGroup, a);
        }
    } else {
        AnnotationGroup *childGroup = gr->getSubgroups().last();
        if(treeWalker->isExpanded(childGroup)) {
            return getLastItemInSubtree(childGroup, view);
        } else {
            AVGroupItemL *parentGroup = view->findGroupItem(gr); //FIXME: what if group don't exist
            AVGroupItemL *upperGroup = view->findGroupItem(gr->getParentGroup());
            if(parentGroup == NULL) {
                parentGroup = new AVGroupItemL(view, upperGroup, gr);
                insertItemBehindView(/*upperGroup->childCount() - 1 */0, parentGroup);
                if(treeWalker->isExpanded(gr)) {
                    emptyExpand = true;
                    expand(guessIndex(parentGroup));
                    emptyExpand = false;
                }
            }
            return new AVGroupItemL(view, parentGroup, childGroup);
        }
    }
}

AVItemL * LazyTreeView::getNextItemUp(){ //Get item upper than first on screen
    AVItemL *bottom = static_cast<AVItemL*>(onScreen.first());
    AVItemL *nextItem = NULL;
    AVQualifierItemL *currentQual = NULL;
    AVAnnotationItemL *currentAnn = NULL;
    AVGroupItemL *currentGroup = NULL;
    AVGroupItemL *parentGroup = NULL;
    U2Qualifier qual;
    int index;
    switch(bottom->type) {
        case AVItemType_Qualifier: //If top item is qualifier
            currentQual = static_cast<AVQualifierItemL*>(bottom);
            currentAnn = static_cast<AVAnnotationItemL*>(currentQual->parent());
            qual = U2Qualifier(currentQual->qName, currentQual->qValue);
            index = currentAnn->annotation->getQualifiers().indexOf(qual);
            if(index == 0) { //Next item parent annotation
                return currentAnn;
            } else { //Or nearby qualifier if current qualifier not first
                nextItem = new AVQualifierItemL(currentAnn, currentAnn->annotation->getQualifiers()[index - 1]);
                return nextItem;
            }
            break; 
        case AVItemType_Annotation: //If top item is annotation
             currentAnn = static_cast<AVAnnotationItemL*>(bottom);
             currentGroup = static_cast<AVGroupItemL*>(currentAnn->parent());
             index = currentGroup->getAnnotationGroup()->getAnnotations().indexOf(currentAnn->annotation);
             if(index == 0) { //Next item is parent group
                 return currentGroup;
             }  else { //If not
                 //If previous annotation is expanded
                 if(treeWalker->isExpanded(currentGroup->getAnnotationGroup()->getAnnotations()[index - 1], currentGroup->getAnnotationGroup())) {
                     AVAnnotationItemL * annItem = currentGroup->getAnnotationTreeView()->findAnnotationItem(currentGroup->getAnnotationGroup(), 
                         currentGroup->getAnnotationGroup()->getAnnotations()[index - 1]); //Previous annotation in group
                     if(!annItem) { //If annotation not in tree, insert it in tree
                        annItem = new AVAnnotationItemL(currentGroup, currentGroup->getAnnotationGroup()->getAnnotations()[index - 1]);
                        insertItemBehindView(0, annItem);
                        if(treeWalker->isExpanded(currentGroup->getAnnotationGroup()->getAnnotations()[index - 1], currentGroup->getAnnotationGroup())) {
                            emptyExpand = true;
                            expand(guessIndex(annItem));
                            emptyExpand = false;
                        }
                     }
                     //Last qualifier of previous annotation
                     nextItem = new AVQualifierItemL(annItem, annItem->annotation->getQualifiers().last());
                     return nextItem;
                 }
                 //Or annotation itself
                 nextItem = new AVAnnotationItemL(currentGroup, currentGroup->getAnnotationGroup()->getAnnotations()[index - 1]);
                 return nextItem;
             }
             break;
        case AVItemType_Group: //If top item is group
            currentGroup = static_cast<AVGroupItemL*>(bottom);
            parentGroup = static_cast<AVGroupItemL*>(currentGroup->parent());
            if(parentGroup->parent() == NULL) { //If it top group
                index = parentGroup->indexOfChild(currentGroup);
                if(index == 0) {
                    return NULL; //reached top of tree
                } else {
                    AVGroupItemL *rootGroup = static_cast<AVGroupItemL*>(parentGroup->child(index - 1)); //Another top group
                    if(treeWalker->isExpanded(rootGroup->getAnnotationGroup())) {//If top group is expanded, get last item in it
                        return getLastItemInSubtree(rootGroup->getAnnotationGroup(), rootGroup->getAnnotationTreeView());
                    } else { //or return top group
                        return rootGroup;
                    }
                }
            }
            index = parentGroup->getAnnotationGroup()->getSubgroups().indexOf(currentGroup->getAnnotationGroup());
            if(index == 0) {
                return parentGroup;
            } else {
                if(treeWalker->isExpanded(parentGroup->getAnnotationGroup()->getSubgroups()[index - 1])) {
                    return getLastItemInSubtree(parentGroup->getAnnotationGroup()->getSubgroups()[index - 1], parentGroup->getAnnotationTreeView());
                }
                nextItem = new AVGroupItemL(parentGroup->getAnnotationTreeView(), parentGroup, parentGroup->getAnnotationGroup()->getSubgroups()[index - 1]);
                return nextItem;
            }
            break;
        default:
            assert(0);
    }
    return NULL;
}

void LazyTreeView::resizeModel(){
    int dif = onScreen.size() - numOnScreen;
    if(dif == 0 || onScreen.isEmpty()) {
        return;
    }
    if(dif < 0) { //Expand view
        AVItemL *item = getNextItemDown(static_cast<AVItemL*>(onScreen.last()));
        while(item && dif < 0) { //Add items to bottom
            if(item->parent()->parent() == NULL) {
                insertItem(item->parent()->indexOfChild(item), item, false);
            } else {
                insertItem(item->parent()->childCount() - 1, item, false);
            }
            if(++dif < 0) {
                item = getNextItemDown(static_cast<AVItemL*>(onScreen.last()));
            }
        }

        if(dif < 0) {
            item = getNextItemUp();
        }
        while(item && dif < 0) { //Add items to top
            flag = true;
            QTreeWidgetItem *last = onScreen.first();
            if(item == last->parent()) {
                onScreen.insert(0, item);
            } else {
                insertItem(0,  item);
            }
            if(++dif < 0) {
                item = getNextItemUp();
            }
        }
    } else { //Narrow view
        while(dif > 0) {
            QTreeWidgetItem *item = onScreen.takeLast(); //delete all excess annotations
            removeItem(item);
            dif--;
        }
    }
    if(indexAt(QPoint(0,0)) != guessIndex(onScreen.first())) {
        flag = true;
        emptyExpand = true;
        scrollTo(guessIndex(onScreen.first()), QAbstractItemView::PositionAtTop);
        emptyExpand = false;

    }
    updateSlider();
}

void LazyTreeView::calculateIndex( AnnotationTableObject *obj ){
    treeWalker->recalculate(obj);
}

void LazyTreeView::updateSlider() {
    int maxVal;
    if(realNumberOfItems - numOnScreen < 0) {
        maxVal = 0;
    } else {
        maxVal = realNumberOfItems - numOnScreen;
    }
    CustomSlider *slider = static_cast<CustomSlider*>(verticalScrollBar());
    slider->setCustomPageStep(numOnScreen);
    slider->setPageStep(numOnScreen);
    slider->setMaxVal(maxVal);
    slider->setMaximum(slider->getMaxVal());
}

int LazyTreeView::getPositionInTree(QTreeWidgetItem *item) {
    QList<int> indexes;
    while(item->parent()) {
        indexes.prepend(item->parent()->indexOfChild(item));
        item = item->parent();
    }

    int pos = 0;
    foreach(int i, indexes) {
        for(int j = 0; j < i; j++) {
            pos += getExpandedNumber((static_cast<AVItemL*>(item->child(j))));
        }
        item = item->child(i);
    }
    return pos + indexes.last();
}

LazyTreeView::LazyTreeView( QWidget * parent /*= 0 */ ) : QTreeView(parent) {
    connect(this, SIGNAL(expanded(const QModelIndex &)), SLOT(sl_expanded(const QModelIndex &)));
    connect(this, SIGNAL(entered(const QModelIndex &)), SLOT(sl_entered(const QModelIndex &)));
    connect(this, SIGNAL(collapsed(const QModelIndex&)), SLOT(sl_collapsed(const QModelIndex&)));
    //connect(verticalScrollBar(), SIGNAL(rangeChanged ( int, int)), SLOT(sl_rangeChanged(int, int)));
    numOnScreen = 14;
    setVerticalScrollBar(new CustomSlider(this));
    verticalScrollBar()->setMouseTracking(true);
    flag = false;
    treeWalker = new TreeIndex();
    realNumberOfItems = 0;
    emptyExpand = false;
    lineHeight = 1;
}

void LazyTreeView::selectionChanged( const QItemSelection & selected, const QItemSelection & deselected ) {
    QTreeView::selectionChanged(selected, deselected);
    emit itemSelectionChanged();
}

void LazyTreeView::mouseDoubleClickEvent( QMouseEvent * event ) {
    LazyAnnotationTreeViewModel *modell = static_cast<LazyAnnotationTreeViewModel*>(model());
    QPoint pos = event->pos();
    QModelIndex index = indexAt(pos);
    QTreeWidgetItem *item = modell->getItem(index);
    //selectionModel()->select(index, QItemSelectionModel::Select);

    QTreeView::mouseDoubleClickEvent(event);
    emit itemDoubleClicked(item, index.column());
}

void LazyTreeView::mousePressEvent( QMouseEvent *event ) {
    LazyAnnotationTreeViewModel *modell = static_cast<LazyAnnotationTreeViewModel*>(model());
    QPoint pos = event->pos();
    QModelIndex index = indexAt(pos);
    QTreeWidgetItem *item = modell->getItem(index);

    QTreeView::mousePressEvent(event);
    emit itemClicked(item, index.column());
}

void LazyTreeView::resizeEvent( QResizeEvent *event ) {
    QTreeView::resizeEvent(event);
    numOnScreen = viewport()->height() / lineHeight;
    if(numOnScreen == 0) {
        numOnScreen = 1;
    }
    resizeModel();
}

void LazyTreeView::sl_entered( const QModelIndex &index ) {
    LazyAnnotationTreeViewModel *modell = static_cast<LazyAnnotationTreeViewModel*>(model());
    QTreeWidgetItem *item = (QTreeWidgetItem *)modell->getItem(index);
    emit itemEntered(item, index.column());
}

void LazyTreeView::updateItem( QTreeWidgetItem *item ) {
    update(guessIndex(item));
    setDirtyRegion(QRegion(visualRect(guessIndex(item))));
    viewport()->repaint();
}

void LazyTreeView::setLineHeight( int height ) {
    lineHeight = height;
    CustomSlider * slider = static_cast<CustomSlider*>(verticalScrollBar());
    slider->setRowHeight(height);
}

QModelIndex LazyTreeView::guessIndex( QTreeWidgetItem *item ) {
    LazyAnnotationTreeViewModel *lm = static_cast<LazyAnnotationTreeViewModel*>(model());
    return lm->guessIndex(item);
}

void AnnotationsTreeViewL::focusOnItem(Annotation *a) {
    QList<int> indexes;
    AnnotationGroup *topGroup = a->getGroups().first();
    indexes << topGroup->getAnnotations().indexOf(a);

    /************************************************************************/
    /*Create indexes of items from annotation to top group                  */
    /************************************************************************/
    while(topGroup->getParentGroup()) { 
        indexes.insert(0, topGroup->getParentGroup()->getSubgroups().indexOf(topGroup));
        topGroup = topGroup->getParentGroup();
    }

    AVGroupItemL *topGroupItem = /*NULL*/findGroupItem(topGroup);
    LazyAnnotationTreeViewModel *lm = static_cast<LazyAnnotationTreeViewModel*>(tree->model());
    AVItemL *root = lm->getItem(QModelIndex());

    /************************************************************************/
    /*Destroy old tree                  */
    /************************************************************************/
    assert(topGroupItem);
    for(int i = 0; i< root->childCount(); i++) {
        destroyTree(root->child(i));
    }
    

    /************************************************************************/
    /*Create all parent groups for given annotation                  */
    /************************************************************************/
    for(int i = 0; i < indexes.size() - 1; i++) {
        if(!findGroupItem(topGroupItem->group->getSubgroups()[indexes[i]])) {
            topGroupItem = createGroupItem(topGroupItem, topGroupItem->group->getSubgroups()[indexes[i]]);
        } else {
            topGroupItem = findGroupItem(topGroupItem->group->getSubgroups()[indexes[i]]);
            tree->onScreen.insert(0, topGroupItem);
        }
    }
    
    int num = tree->numOnScreen;
    QList<QTreeWidgetItem*> oldList = tree->onScreen;
    tree->onScreen.clear();
    
    int index = indexes.last();
    createAnnotationItem(topGroupItem, topGroupItem->group->getAnnotations()[index], false);
    num--;

    AVItemL *item = tree->getNextItemDown(static_cast<AVItemL*>(tree->onScreen.last()));
    while(num > 0 && item) {
        tree->insertItem(item->parent()->childCount() - 1, item, false);
        num--;
        if(num > 0) {
            item = tree->getNextItemDown(static_cast<AVItemL*>(tree->onScreen.last()));
        }
    }

    if(num > 0) {
        item = tree->getNextItemUp();
    }
    while(num > 0 && item) {
        if(!oldList.contains(item)) {
            tree->insertItem(0, item, false);
            if(tree->treeWalker->isExpanded(item)) {
                tree->expand(tree->guessIndex(item));
            }
        } else {
            tree->onScreen.insert(0, item);
        }
        num--;
        if(num > 0) {
            item = tree->getNextItemUp();
        }
    } 

    /************************************************************************
    Scroll to upper item on screen
    ************************************************************************/
    if(tree->indexAt(QPoint(0,0)) != tree->guessIndex(tree->onScreen.first())) {
        tree->flag = true;
        CustomSlider *slider = static_cast<CustomSlider *>(tree->verticalScrollBar());
        slider->setPosition(tree->getPositionInTree(tree->onScreen.first()));
        tree->scrollTo(tree->guessIndex(tree->onScreen.first()), QAbstractItemView::PositionAtTop);
    }
}


CustomSlider::CustomSlider( QWidget *parent /*= NULL*/ ) : QScrollBar(parent), minVal(0), maxVal(0), sliderPos(0), rowHeight(1), dif(0){
    setPageStep(14); //random value, will be changed by tree view
    setMouseTracking(true);
    sliderPressed = false;
}

void CustomSlider::setMaxVal( int _maxVal ){
    maxVal = _maxVal;
}

int CustomSlider::getMaxVal() const {
    return maxVal;
}

void CustomSlider::setPosition( int pos ) {
    sliderPos = qBound(minVal,pos, maxVal);
}

void CustomSlider::setCustomPageStep( int /*ps*/ ) {
    //pageStep = ps;
}

void CustomSlider::sliderChange( SliderChange sc ) {
    if(sc == QAbstractSlider::SliderRangeChange) {
        if(!(maximum() == maxVal /*&& minimum() == minVal*/)) {
            setMaximum(maxVal);
            setMinimum(minVal);
            return;
        }
    } 
    QScrollBar::sliderChange(sc);
}

void CustomSlider::paintEvent( QPaintEvent * ) {
    //QStyleOptionSlider options;
    options.initFrom(this);
    options.sliderPosition = sliderPos;
    options.maximum = maxVal;
    options.minimum = minVal;
    options.singleStep = 1;
    options.pageStep = pageStep();
    options.orientation = Qt::Vertical;
    options.sliderValue = sliderPos;
    options.upsideDown = false;
    options.state = QStyle::State_Sunken | QStyle::State_Enabled;

    QPainter painter(this);
    style()->drawComplexControl(QStyle::CC_ScrollBar, &options, &painter);
}

int CustomSlider::sliderCustomPosition() const {
    return sliderPos;
}

void CustomSlider::mouseMoveEvent( QMouseEvent *me ) {
    if(me->buttons() & Qt::LeftButton && sliderPressed) {
        int newPos = QStyle::sliderValueFromPosition(minVal, maxVal, me->pos().y(), height());
        int sDif = newPos - sliderPos;
        dif = qAbs(sDif);
        setSliderPosition(sliderPosition() + sDif);
    }
}

void CustomSlider::mousePressEvent(QMouseEvent *me) {
    if(me->button() == Qt::LeftButton) {
        QRect sliderRect = style()->subControlRect(QStyle::CC_ScrollBar, &options, QStyle::SC_ScrollBarSlider, this); //Slider
        QRect grooveRect = style()->subControlRect(QStyle::CC_ScrollBar, &options, QStyle::SC_ScrollBarGroove, this); //Free space where slider can move
        if(sliderRect.contains(me->pos())) { //mouse was pressed on slider -> scroll according to mouse move
            sliderPressed = true;
        } else if(grooveRect.contains(me->pos())) { //mouse was pressed on free space of scroll bar -> scroll on page step
            if(me->pos().y() < sliderRect.topLeft().y()) {
                dif = pageStep();
                setSliderPosition(sliderPosition() - pageStep());
            } else if(me->pos().y() > sliderRect.bottomLeft().y()) {
                dif = pageStep();
                setSliderPosition(sliderPosition() + pageStep());
            }
        } else{ //mouse was pressed on arrows
            if(me->pos().y() < sliderRect.topLeft().y()) {
                dif = 1;
                setSliderPosition(sliderPosition() - 1);
            } else if(me->pos().y() > sliderRect.bottomLeft().y()) {
                dif = 1;
                setSliderPosition(sliderPosition() + 1);
            }
        }
        return;
    }
    QScrollBar::mousePressEvent(me); //mouse was pressed on arrows or right button was pressed
}

void CustomSlider::mouseReleaseEvent(QMouseEvent *me) {
    if(me->button() == Qt::LeftButton) {
        sliderPressed = false;
    }
}

void CustomSlider::setRowHeight( int height ) {
    rowHeight = height;
}

int CustomSlider::numToScroll() const {
    return dif;
}

void CustomSlider::resetNumToScroll() {
    dif = 0;
}
}//namespace
