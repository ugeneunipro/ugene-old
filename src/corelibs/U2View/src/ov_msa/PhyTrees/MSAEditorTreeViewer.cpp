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
#include "MSAEditorTreeViewer.h"
#include <U2View/GraphicsRectangularBranchItem.h>
#include <U2View/GraphicsButtonItem.h>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QCursor>
#include <QtCore/QStack>
#include <U2Core/PhyTree.h>
#include <U2Core/U2SafePoints.h>
#include <QDateTime>
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>
#include <U2View/MSAEditorNameList.h>
#include <U2View/MSAEditorDataList.h>

namespace U2 {

MSAEditorTreeViewer::MSAEditorTreeViewer(const QString& viewName, GObject* obj, GraphicsRectangularBranchItem* _root, qreal s)
    : TreeViewer(viewName, obj, _root, s), msa(NULL), syncMode(WithoutSynchronization), slotsAreConnected(false) {
}

QWidget* MSAEditorTreeViewer::createWidget() {
    SAFE_POINT(ui == NULL, QString("MSAEditorTreeViewer::createWidget error"), NULL);
    QWidget* view = new QWidget();
    QVBoxLayout* vLayout = new QVBoxLayout();
    ui = new MSAEditorTreeViewerUI(this);
    QToolBar* toolBar = new QToolBar(tr("MSAEditor tree toolbar"));
    buildMSAEditorStaticToolbar(toolBar);

    sortSeqAction = new QAction(QIcon(":core/images/sort_ascending.png"), tr("Sort alignment by tree"), ui);
    sortSeqAction->setObjectName("Sort Alignment");
    connect(sortSeqAction, SIGNAL(triggered()), ui, SLOT(sl_sortAlignment()));

    refreshTreeAction = new QAction(QIcon(":core/images/refresh.png"), tr("Refresh tree"), ui);
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
    //msaUI->setTreeVerticalSize(size);
    //msa->getUI()->getSequenceArea()->onVisibleRangeChanged();
} 

void MSAEditorTreeViewer::setCreatePhyTreeSettings(const CreatePhyTreeSettings& _buildSettings) {
    buildSettings = _buildSettings;
    refreshTreeAction->setEnabled(true);
}

void MSAEditorTreeViewer::sl_refreshTree() {
    emit si_refreshTree(*this);
}
void MSAEditorTreeViewer::setSynchronizationMode(SynchronizationMode newSyncMode) {
    CHECK(NULL != msa,);
    syncMode = newSyncMode;
    MSAEditorUI* msaUI = msa->getUI();
    MSAEditorTreeViewerUI* treeViewerUI = dynamic_cast<MSAEditorTreeViewerUI*>(ui);
    CHECK(NULL != treeViewerUI, );
    treeViewerUI->setSynchronizeMode(syncMode);
    if(!treeViewerUI->canSynchronizeWithMSA(msa)) {
        syncMode = WithoutSynchronization;
        sortSeqAction->setEnabled(false);
        return;
    }
    if(FullSynchronization == syncMode) {
        if(false == slotsAreConnected) {
            connect(treeViewerUI, SIGNAL(si_collapseModelChangedInTree(const QStringList*)), msaUI->getSequenceArea(), SLOT(sl_setCollapsingRegions(const QStringList*)));
            connect(treeViewerUI, SIGNAL(si_seqOrderChanged(QStringList*)), msa, SLOT(sl_onSeqOrderChanged(QStringList*)));
            connect(treeViewerUI, SIGNAL(si_groupColorsChanged(const GroupColorSchema&)), msaUI->getEditorNameList(), SLOT(sl_onGroupColorsChanged(const GroupColorSchema&)));
            //connect(msa, SIGNAL(si_sizeChanged(int, bool, bool)), treeViewerUI, SLOT(sl_onHeightChanged(int, bool, bool)));

            connect(treeViewerUI,               SIGNAL(si_treeZoomedIn()),
                    msa,                        SLOT(sl_zoomIn()));
            connect(msa,                        SIGNAL(si_referenceSeqChanged(qint64)),
                    treeViewerUI,               SLOT(sl_onReferenceSeqChanged(qint64)));
            connect(treeViewerUI,               SIGNAL(si_treeZoomedOut()),
                    msa,                        SLOT(sl_zoomOut()));
            connect(msaUI->getSequenceArea(),   SIGNAL(si_visibleRangeChanged(QStringList, int)),
                    treeViewerUI,               SLOT(sl_onVisibleRangeChanged(QStringList, int)));

            slotsAreConnected = true;
        }
        sortSeqAction->setEnabled(true);
        treeViewerUI->sl_sortAlignment();
        msa->getUI()->getSequenceArea()->onVisibleRangeChanged();
    }
    connect(msaUI->getSequenceArea(),   SIGNAL(si_selectionChanged(const QStringList&)), treeViewerUI,  SLOT(sl_selectionChanged(const QStringList&)));
    connect(msaUI->getEditorNameList(), SIGNAL(si_sequenceNameChanged(QString, QString)), treeViewerUI, SLOT(sl_sequenceNameChanged(QString, QString)));
}

MSAEditorTreeViewerUI::MSAEditorTreeViewerUI(MSAEditorTreeViewer* treeViewer) 
    : TreeViewerUI(treeViewer), subgroupSelectorPos(0.0), groupColors(1, 0.86), isSinchronized(true), curLayoutIsRectangular(true), 
 curMSATreeViewer(treeViewer), syncMode(WithoutSynchronization), hasMinSize(false), hasMaxSize(false){
    connect(scene(), SIGNAL(sceneRectChanged(const QRectF&)), SLOT(sl_onSceneRectChanged(const QRectF&)));

    QRectF rect = scene()->sceneRect();
    subgroupSelector = scene()->addLine(0.0, rect.bottom(), 0.0, rect.top(), QPen(Qt::blue));
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
}

void MSAEditorTreeViewerUI::setTreeVerticalSize(int size) {
    //sl_onHeightChanged(size, false, false);
}

ColorGenerator::ColorGenerator(int _countOfColors, qreal _lightness): countOfColors(_countOfColors), delta(0.1), hue(0.0), lightness(_lightness){
    satValues << 0.4 << 0.8 << 0.6 << 1.0;
    SAFE_POINT(lightness >= 0 && lightness <= 1.0, "ColorGenerator::ColorGenerator(int _countOfColors, qreal _lightness) _lightness must be in range (0.0, 1.0)",);
    CHECK(0 < countOfColors, );
    generateColors();
}
void ColorGenerator::setCountOfColors(int counts) {
    CHECK(0 < countOfColors, );
    countOfColors = counts;
    delta = (1.0 / countOfColors);
    generateColors();
}
QColor ColorGenerator::getColor(int index) {
    if(index >= 0 && index < colors.size()) {
        return colors.at(index);
    }
    else {
        return Qt::white;
    }
}
void ColorGenerator::generateColors() {
    srand(QDateTime::currentDateTime().toTime_t());
    int countOfAddedColors = countOfColors - colors.size();
    for(int i = 0; i < countOfAddedColors; i++) {
        QColor color;
        qreal saturation = satValues.at(i%satValues.size());
        color.setHslF(hue, saturation , lightness);
        int size = colors.size();
        if(size > 2) {
            colors.insert(qrand()%size, color);
        }
        else {
            colors.append(color);
        }
        hue += delta;
        hue = (hue > 1) ? hue - 1 : hue;
    }
}
void MSAEditorTreeViewerUI::mouseDoubleClickEvent(QMouseEvent *e) {
    QGraphicsView::mouseDoubleClickEvent(e);
    emit si_collapseModelChangedInTree(getVisibleSeqsList());
}


void MSAEditorTreeViewerUI::mousePressEvent(QMouseEvent *e) {
    bool isLeftButton = e->button() == Qt::LeftButton;
    if(isLeftButton && abs(mapFromScene(subgroupSelectorPos, 0.0).x() - e->pos().x()) < 5) {
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
    if(!curLayoutIsRectangular || curMSATreeViewer->getSynchronizationMode() != FullSynchronization) {
        TreeViewerUI::wheelEvent(we);
        return;
    }
    we->accept();
    if(0 < we->delta() && !hasMaxSize) {
        emit si_treeZoomedIn();
    }
    if(0 > we->delta() && !hasMinSize) {
        emit si_treeZoomedOut();
    }
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
        QRectF rect = scene()->sceneRect();
        subgroupSelector->setLine(subgroupSelectorPos, rect.bottom(), subgroupSelectorPos, rect.top());
        highlightBranches();
        //scene()->update();
    }
    else {
        TreeViewerUI::mouseMoveEvent(me);
    }
    me->accept();
}

void MSAEditorTreeViewerUI::sl_onSceneRectChanged( const QRectF& ) {
    QRectF rect = scene()->sceneRect();
    subgroupSelector->setLine(subgroupSelectorPos, rect.bottom(), subgroupSelectorPos, rect.top());
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

bool MSAEditorTreeViewerUI::canSynchronizeWithMSA(MSAEditor* msa) {
    if(!curLayoutIsRectangular) {
        return false;
    }
    QStringList seqsNames = msa->getMSAObject()->getMAlignment().getRowNames();
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
        QString nodeName = QString(nameItem->text());
        if(!seqsNames.contains(nodeName)) {
            return false;
        }
    }
    return true;
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
    TreeViewerUI::setTreeLayout(newLayout);
}

void MSAEditorTreeViewerUI::onLayoutChanged(const TreeLayout& layout) {
    curLayoutIsRectangular = (TreeViewerUI::TreeLayout_Rectangular == layout);
    curMSATreeViewer->getSortSeqsAction()->setEnabled(false);
    if(curLayoutIsRectangular) {
        subgroupSelector->show();
        if(FullSynchronization == syncMode) {
            curMSATreeViewer->getSortSeqsAction()->setEnabled(true);
            MSAEditor* msa = curMSATreeViewer->getMsaEditor();
            CHECK(NULL != msa, );
            //setTreeVerticalSize(msa->getUI()->getSequenceArea()->getHeight());
            msa->getUI()->getSequenceArea()->onVisibleRangeChanged();
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
    int treeHeight = getTreeSize().length;
    CHECK(0 != treeHeight  && 0 != height, );
    qreal zoomCoef = (qreal)(height - 5) / treeHeight;
    //zooming(1.0, zoomCoef);
    //updateSceneRect(sceneRect());
    hasMaxSize = isMaximumSize;
    hasMinSize = isMinimumSize;
}

void MSAEditorTreeViewerUI::sl_onReferenceSeqChanged(qint64) {
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

    int countOfListNodes = getListNodesOfTree().size();
    if(groupColors.getCountOfColors() < countOfListNodes) {
        groupColors.setCountOfColors(countOfListNodes);
    }
    do {
        GraphicsRectangularBranchItem* node = graphicsItems.pop();
        qreal node1Pos = node->sceneBoundingRect().left();
        qreal node2Pos = node->sceneBoundingRect().right();
        if(node2Pos > subgroupSelectorPos && node1Pos < subgroupSelectorPos) {
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
        PhyNode* secondNode = NULL;
        if(NULL != branchItem->getPhyBranch()) {
            secondNode = branchItem->getPhyBranch()->node2;
        }
        else {
            continue;
        }
        if(colorSchema.contains(secondNode)) {
            BranchSettings settings = branchItem->getBranchSettings();
            settings.branchColor = colorSchema[secondNode];
            branchItem->updateSettings(settings);
            branchItem->updateChildSettings(settings);
        }
        else {
            colorSchema[secondNode] = groupColors.getColor(colorIndex);
            BranchSettings settings = branchItem->getBranchSettings();
            settings.branchColor = colorSchema[secondNode];
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

void MSAEditorTreeViewerUI::resizeEvent(QResizeEvent *e) {
    CHECK(curLayoutIsRectangular && isSinchronized, );

    QGraphicsView::resizeEvent(e);
    e->accept();
}

void MSAEditorTreeViewerUI::setSynchronizeMode(SynchronizationMode newSyncMode) {
    syncMode = newSyncMode;
    bool fullSync = (FullSynchronization == syncMode);
    QList<QGraphicsItem*> items = scene()->items();
    foreach(QGraphicsItem* item, items) {
        GraphicsButtonItem* buttonItem = dynamic_cast<GraphicsButtonItem*>(item);
        if(NULL != buttonItem) {
            buttonItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        }
        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(item);
        if(NULL == branchItem) {
            continue;
        }
        QGraphicsSimpleTextItem* nameItem = branchItem->getNameText();
        if(NULL != nameItem) {
            nameItem->setFlag(QGraphicsItem::ItemIgnoresTransformations, fullSync);
        }
        QGraphicsSimpleTextItem* distanceText = branchItem->getDistanceText();
        if(NULL != distanceText) {
            distanceText->setFlag(QGraphicsItem::ItemIgnoresTransformations, fullSync);
        }
    }
}

QList<GraphicsBranchItem*> MSAEditorTreeViewerUI::getListNodesOfTree() {
    QList<QGraphicsItem*> items = scene()->items();
    QList<GraphicsBranchItem*> result;
    foreach(QGraphicsItem* item, items) {
        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(item);
        if(NULL == branchItem) {
            continue;
        }
        QGraphicsSimpleTextItem* nameItem = branchItem->getNameText();
        if(NULL != nameItem) {
            result.append(branchItem);
        }
    }
    return result;
}

void MSAEditorTreeViewerUI::updateSettings(const TreeSettings &settings) {
    bool isSizeSynchronized = (FullSynchronization == syncMode && curLayoutIsRectangular);
    if(!isSizeSynchronized) {
        TreeViewerUI::updateSettings(settings);
        return;
    }

    bool widthChanged = treeSettings.width_coef == settings.width_coef;
    treeSettings = settings;
    updateTreeSettings(widthChanged);

    MSAEditor* msa = curMSATreeViewer->getMsaEditor();
    CHECK(NULL != msa, );
    msa->getUI()->getSequenceArea()->onVisibleRangeChanged();
}

void MSAEditorTreeViewerUI::sl_rectLayoutRecomputed() {
    QMatrix curMatrix = matrix();
    TreeViewerUI::sl_rectLayoutRecomputed();
    curMSATreeViewer->setSynchronizationMode(syncMode);
    setMatrix(curMatrix);
}

void MSAEditorTreeViewerUI::sl_onVisibleRangeChanged(QStringList visibleSeqs, int height) {
    SAFE_POINT(height > 0, QString("Argument 'height' in function 'MSAEditorTreeViewerUI::sl_onVisibleRangeChanged' less then 1"),);
    QList<GraphicsBranchItem*> items = getListNodesOfTree();
    QRectF rect;
    zooming(1.0, 1.0/getVerticalZoom());
    foreach(const QString& str, visibleSeqs) {
        str == "";
    }
    int itemsNumber = 0;
    foreach(GraphicsBranchItem* item, items) {
        if(visibleSeqs.contains(item->getNameText()->text())) {
          if(rect.isNull()) {
                rect = item->getNameText()->sceneBoundingRect();
            }
            else {
                rect = rect.united(item->getNameText()->sceneBoundingRect());
                ++itemsNumber;
            }
        }
    }
    CHECK(rect.height() > 0, );
    QRectF sceneRect = transform().mapRect(rect); 

    MSAEditor* msa = curMSATreeViewer->getMsaEditor();
    CHECK(NULL != msa, );
    //setTreeVerticalSize(msa->getUI()->getSequenceArea()->getHeight());
    //msa->getUI()->getSequenceArea()->onVisibleRangeChanged();

    int viewportHeight = viewport()->rect().height(); 
    qreal zoom = qreal(height - msa->getRowHeight()/10) / (sceneRect.height() - sceneRect.height() / 1.5 / itemsNumber);
    
    rect.setHeight(rect.height() * (viewportHeight-35) / height);
    
    zooming(1.0, zoom);
    centerOn(rect.center());
    //translate(0, qreal(sceneRect.height()) / 1.5 / itemsNumber);
}


}//namespace

