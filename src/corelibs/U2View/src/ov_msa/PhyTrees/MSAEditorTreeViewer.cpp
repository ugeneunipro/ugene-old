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
#include "MSAEditorTreeViewer.h"
#include <U2View/GraphicsRectangularBranchItem.h>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QCursor>
#include <QtCore/QStack>
#include <U2Core/PhyTree.h>
#include <U2Core/U2SafePoints.h>
#include <QDateTime>


namespace U2 {

MSAEditorTreeViewer::MSAEditorTreeViewer(const QString& viewName, GObject* obj, GraphicsRectangularBranchItem* _root, qreal s)
    : TreeViewer(viewName, obj, _root, s) {
}

QWidget* MSAEditorTreeViewer::createWidget() {
    SAFE_POINT(ui == NULL, QString("MSAEditorTreeViewer::createWidget error"), NULL);
    QWidget* view = new QWidget();
    QVBoxLayout* vLayout = new QVBoxLayout();
    ui = new MSAEditorTreeViewerUI(this);
    QToolBar* toolBar = new QToolBar(tr("MSAEditor tree toolbar"));
    buildStaticToolbar(toolBar);

    sortSeqAction = new QAction(QIcon(":core/images/sort_ascending.png"), tr("Sort alignment by tree"), ui);
    sortSeqAction->setObjectName("Sort Alignment");
    connect(sortSeqAction, SIGNAL(triggered()), ui, SLOT(sl_sortAlignment()));

    refreshTreeAction = new QAction(QIcon(":core/images/arrow_rotate_clockwise.png"), tr("Refresh tree"), ui);
    refreshTreeAction->setObjectName("Refresh tree");
    refreshTreeAction->setEnabled(false);
    connect(refreshTreeAction, SIGNAL(triggered()), SLOT(sl_refreshTree()));


    toolBar->addSeparator();
    toolBar->addAction(refreshTreeAction);
    toolBar->addAction(sortSeqAction);

    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->addWidget(toolBar);
    vLayout->addWidget(ui);
    view->setLayout(vLayout);

    return view;
}

void MSAEditorTreeViewer::setTreeVerticalSize(int size) {
    MSAEditorTreeViewerUI* msaUI = dynamic_cast<MSAEditorTreeViewerUI*>(ui);
    CHECK(NULL != msaUI, );
    msaUI->setTreeVerticalSize(size);
} 

void MSAEditorTreeViewer::setCreatePhyTreeSettings(const CreatePhyTreeSettings& _buildSettings) {
    buildSettings = _buildSettings;
    refreshTreeAction->setEnabled(true);
}

void MSAEditorTreeViewer::sl_refreshTree() {
    emit si_refreshTree(*this);
}

MSAEditorTreeViewerUI::MSAEditorTreeViewerUI(MSAEditorTreeViewer* treeViewer) 
    : TreeViewerUI(treeViewer), subgroupSelectorPos(0.0), isSinchronized(true), curLayoutIsRectangular(true), curMSATreeViewer(treeViewer){
    connect(scene(), SIGNAL(selectionChanged()), SLOT(sl_onSelectionChanged()));
    connect(scene(), SIGNAL(sceneRectChanged(const QRectF&)), SLOT(sl_onSceneRectChanged(const QRectF&)));

    subgroupSelector = scene()->addLine(0.0, 0.0, 0.0, scene()->height(), QPen(Qt::blue));
}

void MSAEditorTreeViewerUI::setTreeVerticalSize(int size) {
    sl_onHeightChanged(size, false, false);
}

GroupColors::GroupColors() {
    colors.append(Qt::cyan);
    colors.append(Qt::yellow);
    colors.append(Qt::green);
    colors.append(Qt::magenta);
    colors.append(Qt::darkYellow);
    colors.append(Qt::gray);
}
QColor GroupColors::getColor(int index) {
    if(index >= 0 && index < colors.size()) {
        return colors.at(index);
    }
    else {
        srand(QDateTime::currentDateTime().toTime_t());
        QColor randomColor(qrand() % 255, qrand() % 255, qrand() % 255);
        return randomColor;
    }
}
void MSAEditorTreeViewerUI::mouseDoubleClickEvent(QMouseEvent *e) {
    QGraphicsView::mouseDoubleClickEvent(e);
    emit si_collapseModelChangedInTree(getVisibleSeqsList());
}


void MSAEditorTreeViewerUI::mousePressEvent(QMouseEvent *e) {
    if(abs(mapFromScene(subgroupSelectorPos, 0.0).x() - e->pos().x()) < 5) {
        subgroupSelectionMode = true;
    }
    else {
        TreeViewerUI::mousePressEvent(e);
    }
}

void MSAEditorTreeViewerUI::mouseReleaseEvent(QMouseEvent *e) {
    subgroupSelectionMode = false;
    TreeViewerUI::mouseReleaseEvent(e);
}

void MSAEditorTreeViewerUI::wheelEvent(QWheelEvent *we ) {
    if(!curLayoutIsRectangular || !isSinchronized) {
        TreeViewerUI::wheelEvent(we);
        return;
    }
    if(0 < we->delta()) {
        emit si_treeZoomedIn();
    }
    else {
        emit si_treeZoomedOut();
    }
    we->accept();
}

void MSAEditorTreeViewerUI::mouseMoveEvent(QMouseEvent *me) {
    if(!subgroupSelector->isVisible()) {
        TreeViewerUI::mouseMoveEvent(me);
        return;
    }
    QRectF boundingRect = scene()->itemsBoundingRect();
    QPointF sceneMousePos = mapToScene(me->pos());
    if(abs(mapFromScene(subgroupSelectorPos, 0.0).x() - me->pos().x()) < 5 && boundingRect.contains(sceneMousePos)) {
        setCursor(Qt::SplitHCursor);
    }
    else {
        setCursor(Qt::ArrowCursor);
    }
    if(true == subgroupSelectionMode) {
        qreal xPos = sceneMousePos.x();

        if(boundingRect.contains(sceneMousePos)) {
            subgroupSelectorPos = mapToScene(me->pos()).x();
        }
        else {
            if(xPos < boundingRect.left()) {
                subgroupSelectorPos = boundingRect.left() + 1;
            }
            if(xPos > boundingRect.right()) {
                subgroupSelectorPos = boundingRect.right() - 1;
            }
        }

        subgroupSelector->setLine(subgroupSelectorPos, 0, subgroupSelectorPos, scene()->height());
        highlightBranches();
    }
    else {
        TreeViewerUI::mouseMoveEvent(me);
    }
    me->accept();
}

void MSAEditorTreeViewerUI::sl_onSceneRectChanged( const QRectF& ) {
    subgroupSelector->setLine(subgroupSelectorPos, 0, subgroupSelectorPos, scene()->height());
}
void MSAEditorTreeViewerUI::sl_zoomToSel() {
    emit si_treeZoomedIn();
}

void MSAEditorTreeViewerUI::sl_zoomOut() {
    emit si_treeZoomedOut();
}

void MSAEditorTreeViewerUI::sl_collapseTriggered() {
    TreeViewerUI::sl_collapseTriggered();
    emit si_collapseModelChangedInTree(getVisibleSeqsList());
}

void MSAEditorTreeViewerUI::sl_selectionChanged(const QStringList& selection) {
    bool cleanSelection = true;
    QList<QGraphicsItem*> items = scene()->items();
    foreach(QGraphicsItem* item, items) {
        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem *>(item);
        if(NULL == branchItem) {
            continue;
        }
        QGraphicsSimpleTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem) {
            continue;
        }
        QString str = QString(nameItem->text());
        foreach(const QString& seqname, selection) {
            bool res = seqname == str;
            res = res ? false : res;
        }
        if(selection.contains(nameItem->text(), Qt::CaseInsensitive)) {
            if(true == cleanSelection) {
                cleanSelection = false;
                getRoot()->setSelectedRecurs(false, true);
            }
            branchItem->setSelectedRecurs(true, false);
        }
        else
            branchItem->setSelectedRecurs(false, false);
    }
    scene()->update();
}

void MSAEditorTreeViewerUI::sl_sequenceNameChanged(QString prevName, QString newName) {
    QList<QGraphicsItem*> items = scene()->items();
    foreach(QGraphicsItem* item, items) {
        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem *>(item);
        if(NULL == branchItem) {
            continue;
        }
        QGraphicsSimpleTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem) {
            continue;
        }
        if(prevName == nameItem->text()) {
            nameItem->setText(newName);
        }
    }
    scene()->update();
}

typedef QPair<qreal, QString> seqNameWithPos;

QStringList* MSAEditorTreeViewerUI::getOrderedSeqNames() {
    QList<QGraphicsItem*> items = scene()->items();
    QList<seqNameWithPos> namesAndHeights;

    foreach(QGraphicsItem* item, items) {
        GraphicsRectangularBranchItem *branchItem = dynamic_cast<GraphicsRectangularBranchItem*>(item);
        if(NULL == branchItem) {
            continue;
        }
        QGraphicsSimpleTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem) {
            continue;
        }
        qreal y = branchItem->scenePos().y();
        QString str = nameItem->text();
        namesAndHeights.append(QPair<qreal, QString>(y, str));
    }
    qSort(namesAndHeights.begin(), namesAndHeights.end());

    QStringList *seqNames = new QStringList;
    foreach(seqNameWithPos pair, namesAndHeights) {
        seqNames->append(pair.second);
    }

    getTreeSize();

    return seqNames;
}
U2Region MSAEditorTreeViewerUI::getTreeSize() {
    QList<QGraphicsItem*> items = scene()->items();
    QList<seqNameWithPos> namesAndHeights;

    foreach(QGraphicsItem* item, items) {
        GraphicsRectangularBranchItem *branchItem = dynamic_cast<GraphicsRectangularBranchItem*>(item);
        if(NULL == branchItem) {
            continue;
        }
        QGraphicsSimpleTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem) {
            continue;
        }
        qreal y = branchItem->scenePos().y();
        QString str = nameItem->text();
        namesAndHeights.append(QPair<qreal, QString>(y, str));
    }
    qSort(namesAndHeights.begin(), namesAndHeights.end());

    int minH = mapFromScene(0, namesAndHeights.first().first).y();
    int maxH = mapFromScene(0, namesAndHeights.last().first).y();

    return U2Region(minH, maxH - minH);
}

void MSAEditorTreeViewerUI::setTreeLayout(TreeLayout newLayout) {
    onLayoutChanged(newLayout);
    TreeViewerUI::setTreeLayout(newLayout);
}

void MSAEditorTreeViewerUI::onLayoutChanged(const TreeLayout& layout) {
    curLayoutIsRectangular = (TreeViewerUI::TreeLayout_Rectangular == layout);
    curMSATreeViewer->getSortSeqsAction()->setEnabled(false);
    if(curLayoutIsRectangular) {
        subgroupSelector->show();
        if(isSinchronized) {
            curMSATreeViewer->getSortSeqsAction()->setEnabled(true);
        }
    }
    else {
        subgroupSelector->hide();
    }
}

const QStringList* MSAEditorTreeViewerUI::getVisibleSeqsList() {
    QList<QGraphicsItem*> items = scene()->items();
    QStringList* result = new QStringList();

    foreach(QGraphicsItem* item, items) {
        GraphicsRectangularBranchItem *branchItem = dynamic_cast<GraphicsRectangularBranchItem*>(item);
        if(NULL == branchItem) {
            continue;
        }
        QGraphicsSimpleTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem) {
            continue;
        }
        if(branchItem->isVisible()) {
            QString str = branchItem->getNameText()->text();
            result->append(branchItem->getNameText()->text());
        }
    }
    return result;
}

void MSAEditorTreeViewerUI::sl_onHeightChanged(int height, bool isMinimumSize, bool isMaximumSize) {
    CHECK(curLayoutIsRectangular && isSinchronized, );
    qreal zoomCoef = (qreal)(height - 5) / getTreeSize().length;
    zooming(zoomCoef, zoomCoef);
    updateSceneRect(sceneRect());
    curMSATreeViewer->getZoomOutAction()->setEnabled(!isMinimumSize);
    curMSATreeViewer->getZoomToSelAction()->setEnabled(!isMaximumSize);
}

void MSAEditorTreeViewerUI::sl_onReferenceSeqChanged(const QString &) {
    QList<QGraphicsItem*> items = scene()->items();
    foreach(QGraphicsItem* item, items) {
        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem *>(item);
        if(NULL == branchItem) {
            continue;
        }
        QGraphicsSimpleTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem) {
            continue;
        }
        QPen brush(Qt::white);
    }
    scene()->update();
}

void MSAEditorTreeViewerUI::sl_sortAlignment() {
    emit si_seqOrderChanged(getOrderedSeqNames());
}

void MSAEditorTreeViewerUI::highlightBranches() {
    BranchSettings settings = rectRoot->getBranchSettings();
    settings.branchColor = Qt::black;
    if(rectRoot) {
        rectRoot->updateSettings(settings);
        rectRoot->updateChildSettings(settings);
    }

    QStack<GraphicsRectangularBranchItem*> graphicsItems;
    QList<GraphicsRectangularBranchItem*> groupRoots;
    graphicsItems.push(getRectRoot());
    
    do {
        GraphicsRectangularBranchItem* node = graphicsItems.pop();
        qreal nodePos = node->sceneBoundingRect().left();
        if(nodePos > subgroupSelectorPos) {
            groupRoots.append(node);
            continue;
        }
        const QList<QGraphicsItem*>& childs = node->childItems();
        foreach(QGraphicsItem* curItem, childs) {
            GraphicsRectangularBranchItem *branchItem = dynamic_cast<GraphicsRectangularBranchItem*>(curItem);
            if(NULL == branchItem) {
                continue;
            }
            graphicsItems.append(branchItem);
        }
    } while(!graphicsItems.isEmpty());

    int colorIndex = 0;
    QMap<PhyNode*, QColor> colorSchema;
    foreach(GraphicsRectangularBranchItem* branchItem, groupRoots) {
        PhyNode* firstNode = NULL;
        if(NULL != branchItem->getPhyBranch()) {
            firstNode = branchItem->getPhyBranch()->node1;
        }
        else {
            continue;
        }
        if(colorSchema.contains(firstNode)) {
            BranchSettings settings = branchItem->getBranchSettings();
            settings.branchColor = colorSchema[firstNode];
            branchItem->updateSettings(settings);
            branchItem->updateChildSettings(settings);
        }
        else {
            colorSchema[firstNode] = groupColors.getColor(colorIndex);
            BranchSettings settings = branchItem->getBranchSettings();
            settings.branchColor = colorSchema[firstNode];
            branchItem->updateSettings(settings);
            branchItem->updateChildSettings(settings);
            colorIndex++;
        }
    }

    GroupColorSchema groupColorSchema;
    QList<QGraphicsItem*> items = scene()->items();
    foreach(QGraphicsItem* item, items) {
        GraphicsRectangularBranchItem *branchItem = dynamic_cast<GraphicsRectangularBranchItem*>(item);
        if(NULL == branchItem) {
            continue;
        }
        QGraphicsSimpleTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem) {
            continue;
        }
        QString name = nameItem->text();
        groupColorSchema[nameItem->text()] = branchItem->getBranchSettings().branchColor;
    }
    emit si_groupColorsChanged(groupColorSchema);
}
}//namespace

