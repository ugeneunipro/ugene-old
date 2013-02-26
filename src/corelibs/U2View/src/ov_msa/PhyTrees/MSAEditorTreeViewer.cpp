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
#include <QDateTime>


namespace U2 {

MSAEditorTreeViewer::MSAEditorTreeViewer(const QString& viewName, GObject* obj, GraphicsRectangularBranchItem* _root, qreal s, const CreatePhyTreeSettings* _settings)
    : TreeViewer(viewName, obj, _root, s), settings(_settings) {
}

QWidget* MSAEditorTreeViewer::createWidget() {
    assert(ui == NULL);
    QWidget* view = new QWidget();
    QVBoxLayout* vLayout = new QVBoxLayout();
    ui = new MSAEditorTreeViewerUI(this);
    QToolBar* toolBar = new QToolBar(tr("MSAEditor tree toolbar"));
    buildStaticToolbar(toolBar);

    sortSeqAction = new QAction(QIcon(":core/images/sort_ascending.png"), tr("Sort alignment by tree"), ui);
    sortSeqAction->setObjectName("Sort Alignment");
    bool res = connect(sortSeqAction, SIGNAL(triggered()), ui, SLOT(sl_sortAlignment()));

    toolBar->addSeparator();
    toolBar->addAction(sortSeqAction);

    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->addWidget(toolBar);
    vLayout->addWidget(ui);
    view->setLayout(vLayout);

    return view;
}

MSAEditorTreeViewerUI::MSAEditorTreeViewerUI(MSAEditorTreeViewer* treeViewer) 
    : TreeViewerUI(treeViewer), sliderPos(0.0){
    connect(scene(), SIGNAL(selectionChanged()), this, SLOT(sl_onSelectionChanged()));

    treeLevelSlider = scene()->addLine(0.0, 0.0, 0.0, scene()->height(), QPen(Qt::blue));
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
    emit si_seqCollapsed(getCollapsingRegions());
}


void MSAEditorTreeViewerUI::mousePressEvent(QMouseEvent *e) {
    if(abs(mapFromScene(sliderPos, 0.0).x() - e->pos().x()) < 5) {
        choiceLevelIsActive = true;
    }
    else {
        TreeViewerUI::mousePressEvent(e);
    }
}

void MSAEditorTreeViewerUI::mouseReleaseEvent(QMouseEvent *e) {
    choiceLevelIsActive = false;
    TreeViewerUI::mouseReleaseEvent(e);
}

void MSAEditorTreeViewerUI::wheelEvent(QWheelEvent *we ) {
    if(0 < we->delta()) {
        emit si_treeZoomedIn();
    }
    else {
        emit si_treeZoomedOut();
    }
    we->accept();
}

void MSAEditorTreeViewerUI::mouseMoveEvent(QMouseEvent *me) {
    if(!treeLevelSlider->isVisible()) {
        TreeViewerUI::mouseMoveEvent(me);
        return;
    }
    QRectF boundingRect = scene()->itemsBoundingRect();
    QPointF sceneMousePos = mapToScene(me->pos());
    if(abs(mapFromScene(sliderPos, 0.0).x() - me->pos().x()) < 5 && boundingRect.contains(sceneMousePos)) {
        setCursor(Qt::SplitHCursor);
    }
    else {
        setCursor(Qt::ArrowCursor);
    }
    if(true == choiceLevelIsActive) {
        qreal xPos = sceneMousePos.x();

        if(boundingRect.contains(sceneMousePos)) {
            sliderPos = mapToScene(me->pos()).x();
        }
        else {
            if(xPos < boundingRect.left()) {
                sliderPos = boundingRect.left() + 1;
            }
            if(xPos > boundingRect.right()) {
                sliderPos = boundingRect.right() - 1;
            }
        }

        treeLevelSlider->setLine(sliderPos, 0, sliderPos, scene()->height());
        highlightBranches();
    }
    else {
        TreeViewerUI::mouseMoveEvent(me);
    }
    me->accept();
}
void MSAEditorTreeViewerUI::sl_zoomToSel() {
    emit si_treeZoomedIn();
}

void MSAEditorTreeViewerUI::sl_zoomOut() {
    emit si_treeZoomedOut();
}
void MSAEditorTreeViewerUI::sl_swapTriggered() {
    TreeViewerUI::sl_swapTriggered();
    emit si_seqOrderChanged(getOrderedSeqNames());
}

void MSAEditorTreeViewerUI::sl_collapseTriggered() {
    TreeViewerUI::sl_collapseTriggered();
    emit si_seqCollapsed(getCollapsingRegions());
}

void MSAEditorTreeViewerUI::sl_selectionChanged(const QStringList& selection) {
    bool cleanSelection = true;
    QList<QGraphicsItem*> items = scene()->items();
    foreach(QGraphicsItem* item, items) {
        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem *>(item);
        if(NULL == branchItem)
            continue;
        QGraphicsTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem)
            continue;
        QString str = QString(branchItem->getNameText()->toPlainText().unicode());
        foreach(const QString& seqname, selection) {
            bool res = seqname == str;
            if(res) {
                res = false;
            }
        }
        if(selection.contains(branchItem->getNameText()->toPlainText(), Qt::CaseInsensitive)) {
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
        if(NULL == branchItem)
            continue;
        QGraphicsTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem)
            continue;
        if(prevName == branchItem->getNameText()->toPlainText())
            branchItem->getNameText()->setPlainText(newName);
    }
    getOrderedSeqNames();
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
        QGraphicsTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem) {
            continue;
        }
        qreal y = branchItem->scenePos().y();
        QString str = nameItem->toPlainText();
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
        QGraphicsTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem) {
            continue;
        }
        qreal y = branchItem->scenePos().y();
        QString str = nameItem->toPlainText();
        namesAndHeights.append(QPair<qreal, QString>(y, str));
    }
    qSort(namesAndHeights.begin(), namesAndHeights.end());

    int minH = mapFromScene(0, namesAndHeights.first().first).y();
    int maxH = mapFromScene(0, namesAndHeights.last().first).y();

    return U2Region(minH, maxH - minH);
}

void MSAEditorTreeViewerUI::setTreeLayout(TreeLayout newLayout) {
    if(TreeViewerUI::TreeLayout_Rectangular == newLayout) {
        treeLevelSlider->show();
    }
    else {
        treeLevelSlider->hide();
    }
    TreeViewerUI::setTreeLayout(newLayout);
}

QVector<U2Region>* MSAEditorTreeViewerUI::getCollapsingRegions() {
    QList<QGraphicsItem*> items = scene()->items();
    QList<QPair<qreal, bool> > namesAndHeights;

    foreach(QGraphicsItem* item, items) {
        GraphicsRectangularBranchItem *branchItem = dynamic_cast<GraphicsRectangularBranchItem*>(item);
        if(NULL == branchItem) {
            continue;
        }
        QGraphicsTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem) {
            continue;
        }
        qreal y = branchItem->scenePos().y();
        if(branchItem->isCollapsed()) {
            bool res = branchItem->isCollapsed();
        }
        namesAndHeights.append(QPair<qreal, bool>(y, branchItem->isCollapsed()));
    }
    qSort(namesAndHeights.begin(), namesAndHeights.end());

    QVector<U2Region> *collapsedRegions = new QVector<U2Region>;
    int startPos = -1;
    int size = namesAndHeights.size();
    for(int i = 0; i < size; i++) {
        if(true == namesAndHeights.at(i).second) {
            if(-1 == startPos) {
                startPos = i;
            }
        }
        else {
            if(-1 == startPos) {
                continue;
            }
            if(i - startPos < 1) {
                startPos = -1;
            }
            else {
                U2Region collapsedReg(startPos, i-startPos);
                collapsedRegions->append(collapsedReg);
                startPos = -1;
            }
        }
    }

    return collapsedRegions;
}

void MSAEditorTreeViewerUI::sl_onHeightChanged(int height) {
    //zooming((float)(height - 5) / getTreeSize().length);
    //newzoom = zoom * newzoom;
    qreal zoomCoef = (qreal)(height - 5) / getTreeSize().length;
    zooming(1.0, zoomCoef);
    updateSceneRect(sceneRect());
}

void MSAEditorTreeViewerUI::sl_onReferenceSeqChanged(const QString &str) {
    QList<QGraphicsItem*> items = scene()->items();
    foreach(QGraphicsItem* item, items) {
        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem *>(item);
        if(NULL == branchItem)
            continue;
        QGraphicsTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem)
            continue;
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
        if(nodePos > sliderPos) {
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
        QGraphicsTextItem* nameItem = branchItem->getNameText();
        if(NULL == nameItem)
            continue;
        QString name = nameItem->toPlainText();
        groupColorSchema[nameItem->toPlainText()] = branchItem->getBranchSettings().branchColor;
    }
    emit si_groupColorsChanged(groupColorSchema);
}


}//namespace

