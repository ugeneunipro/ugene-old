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

#include "QueryViewController.h"
#include "QueryViewItems.h"
#include "QueryPalette.h"
#include "QDSamples.h"
#include "QueryEditor.h"
#include "QDSceneIOTasks.h"
#include "QDRunDialog.h"
#include "QDGroupsEditor.h"

#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/Counter.h>
#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/TaskSignalMapper.h>

#include <U2Misc/GlassView.h>
#include <U2Misc/DialogUtils.h>

#include <U2Lang/QueryDesignerRegistry.h>

#include <U2Designer/DelegateEditors.h>

#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QTabWidget>
#include <QtGui/QSplitter>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>


namespace U2 {
    
/************************************************************************/
/* Scene                                                                */
/************************************************************************/

#define QS 1000
#define MAX_ITEM_SIZE 10000 // max point size for any visual item and whole scene
#define MIN_ROWS_NUMBER 3
#define LABEL_HEIGHT 40
#define LABEL_PIXEL_SIZE 15
#define LABEL_LEFT_PAD 200
QueryScene::QueryScene(QueryViewController* parent/* =0 */)
: QGraphicsScene(parent), dropCandidateLeft(NULL), dropCandidateRight(NULL),
view(parent), rowsNum(MIN_ROWS_NUMBER), showSchemeLbl(false), showSchemeDesc(false),
showDesc(true), showOrder(true), modified(false) {
    setSceneRect(0, 0, QS, QS);
    setItemIndexMethod(NoIndex);
    scheme = new QDScheme;

    labelTxtItem = new QDLabelItem("NewSchema");
    if (parent) {
        connect(labelTxtItem, SIGNAL(si_editingFinished()), parent, SLOT(sl_updateTitle()));
    }
    labelTxtItem->setPos(LABEL_LEFT_PAD,0);
    QFont lblFont;
    lblFont.setItalic(true);
    lblFont.setPixelSize(LABEL_PIXEL_SIZE);
    labelTxtItem->setFont(lblFont);

    descTxtItem = new QDDescriptionItem("<Write description here>");
    descTxtItem->setTextWidth(200);

    ruler = new QDRulerItem;
    connect(this, SIGNAL(si_schemeChanged()), ruler, SLOT(sl_updateText()));
    addItem(ruler);
    ruler->setPos(0,0);
}

QueryScene::~QueryScene() {
    clearScene();
    delete scheme;
    delete labelTxtItem;
    delete descTxtItem;
    delete ruler;
}

int QueryScene::getRow(QDElement* const uv) const {
    const QPointF& pos = uv->scenePos();
    qreal top=annotationsArea().top();
    int row = (pos.y()-top)/GRID_STEP;
    return row;
}

bool yPosLessThan(QGraphicsItem* uv1, QGraphicsItem* uv2) {
    const QPointF& pos1 = uv1->scenePos();
    const QPointF& pos2 = uv2->scenePos();
    if (pos1.y()<pos2.y()) {
        return false;
    }
    return true;
}

void QueryScene::insertRow(int idx) {
    assert(idx>=0);
    if (idx>=rowsNum) {
        rowsNum = idx+1;
        return;
    }
    qreal rowTop = idx*GRID_STEP + annotationsArea().top();
    //sort items by y-pos
    QList<QGraphicsItem*> units;
    foreach(QGraphicsItem* it, items()) {
        if (it->type()==QDElementType) {
            if(it->scenePos().y() >= rowTop) {
                units.append(it);
            }
        }
    }
    qSort(units.begin(), units.end(), yPosLessThan);
    foreach(QGraphicsItem* it, units) {
        QPointF itPos = it->scenePos();
        itPos.ry()+=GRID_STEP;
        it->setPos(itPos);
    }
}

void QueryScene::sl_adaptRowsNumber() {
    int adaptedNum = rowsNum;
    for (int i=rowsNum-1;i>=MIN_ROWS_NUMBER; i--) {
        if (unitsIntersectedByRow(i).isEmpty()) {
            --adaptedNum;
        }
        else {
            break;
        }
    }
    setRowsNumber(adaptedNum);
}

void QueryScene::sl_updateRulerText() {
    ruler->sl_updateText();
}

QString QueryScene::getLabel() const {
    return labelTxtItem->toPlainText();
}

QString QueryScene::getDescription() const {
    return descTxtItem->toPlainText();
}

void QueryScene::setLabel(const QString& lbl) {
    labelTxtItem->setPlainText(lbl);
}

void QueryScene::setDescription(const QString& dsc) {
    descTxtItem->setPlainText(dsc);
}

void QueryScene::sl_showLabel(bool show) {
    if (showSchemeLbl==show) {
        return;
    }
    showSchemeLbl = show;
    int dy;
    if (showSchemeLbl) {
        dy=LABEL_HEIGHT;
        ruler->setPos(0, LABEL_HEIGHT);
    }
    else {
        dy = -LABEL_HEIGHT;
        removeItem(labelTxtItem);
        ruler->setPos(0,0);
    }
    foreach(QGraphicsItem* it, items()) {
        if (it->type()==QDElementType) {
            it->moveBy(0.0, dy);
        }
    }
    update();
    if (showSchemeLbl) {
        addItem(labelTxtItem);
    }
    updateDescriptionPos();
}

void QueryScene::sl_showSchemeDesc(bool show) {
    if (showSchemeDesc==show) {
        return;
    }
    showSchemeDesc = show;
    if (!showSchemeDesc) {
        removeItem(descTxtItem);
    }
    else {
        addItem(descTxtItem);
        updateDescriptionPos();
    }
}

void QueryScene::sl_showItemDesc(bool show) {
    showDesc = show;
    foreach(QGraphicsItem* it, items()) {
        if (it->type()==QDElementType) {
            QDElement* uv = qgraphicsitem_cast<QDElement*>(it);
            uv->sl_refresh();
            uv->rememberSize();
            uv->adaptSize();
            sl_adaptRowsNumber();
        }
    }
}

void QueryScene::sl_showOrder(bool show) {
    showOrder = show;
    foreach(QGraphicsItem* it, items()) {
        if (it->type()==QDElementType) {
            QDElement* uv = qgraphicsitem_cast<QDElement*>(it);
            uv->sl_refresh();
        }
    }
}

#define DESCRIPTION_TOP_PAD 40
void QueryScene::updateDescriptionPos() {
    qreal y = footnotesArea().bottom() + DESCRIPTION_TOP_PAD;
    descTxtItem->setPos(descTxtItem->x(),y);
}

QRectF QueryScene::rulerArea() const {
    QRectF area = ruler->boundingRect();
    area.moveTopLeft(ruler->scenePos());
    assert(area.width() < MAX_ITEM_SIZE && area.height() < MAX_ITEM_SIZE);
    return area;
}

QRectF QueryScene::annotationsArea() const {
    const QRectF& rect = sceneRect();
    assert(rect.width() < MAX_ITEM_SIZE && rect.height() < MAX_ITEM_SIZE);
    //qreal top = round(rect.top() + dy, GRID_STEP);
    qreal top = rect.top() + ruler->boundingRect().height();
    if (showSchemeLbl) {
        top += LABEL_HEIGHT;
    }
    qreal areaHeight = rowsNum*GRID_STEP;
    return QRectF(rect.left(), top, rect.width(), areaHeight);
}

#define FOOTNOTES_AREA_TOP_MARGIN 20
QRectF QueryScene::footnotesArea() const {
    qreal tlX = sceneRect().left();
    qreal tlY = annotationsArea().bottom() + FOOTNOTES_AREA_TOP_MARGIN;
    qreal brX = sceneRect().right();
    qreal brY=tlY;
    foreach(QGraphicsItem* it, items()) {
        if (it->type()==FootnoteItemType) {
            qreal itBottomEdge = it->scenePos().y() + it->boundingRect().height();
            if (itBottomEdge>brY) {
                brY = itBottomEdge;
            }
        }
    }
    QRectF rect(QPointF(tlX,tlY), QPointF(brX,brY));
    assert(rect.width() < MAX_ITEM_SIZE && rect.height() < MAX_ITEM_SIZE);
    return rect;
}

QList<QDElement*> QueryScene::getElements() const {
    QList<QDElement*> res;
    foreach(QGraphicsItem* item, items()) {
        if (item->type()==QDElementType) {
            QDElement* el = qgraphicsitem_cast<QDElement*>(item);
            res.append(el);
        }
    }
    return res;
}

QList<QGraphicsItem*> QueryScene::getFootnotes() const {
    QList<QGraphicsItem*> res;
    foreach(QGraphicsItem* item, items()) {
        if (item->type()==FootnoteItemType) {
            res.append(item);
        }
    }
    return res;
}

QDElement* QueryScene::getUnitView(QDSchemeUnit* su) const {
    foreach(QDElement* el, getElements()) {
        if (el->getSchemeUnit()==su) {
            return el;
        }
    }
    return NULL;
}

void QueryScene::setRowsNumber(int count) {
    rowsNum = count;
    QList<Footnote*> footnotes;
    foreach(QGraphicsItem* item, items()) {
        if(item->type()==FootnoteItemType) {
            footnotes << qgraphicsitem_cast<Footnote*>(item);
        }
    }
    foreach(Footnote* footnote, footnotes) {
        footnote->updatePos();
    }
    //to do: evaluate row area and update it instead of whole scene
    updateDescriptionPos();
    update();
}

void QueryScene::drawBackground(QPainter *painter, const QRectF &rect) {
    Q_UNUSED(rect);
    int step = GRID_STEP;
    painter->setPen(QPen(QColor(200, 200, 255, 125)));
    // draw horizontal grid
    const QRectF& area = annotationsArea();
    qreal start = area.top();
    for (qreal y = start; y < start + (rowsNum+1)*step; y += step) {
        painter->drawLine(area.left(), y, area.right(), y);
    }
}

QList<QGraphicsItem*> QueryScene::getElements(const QRectF& area) {
    QList<QGraphicsItem*> items = QGraphicsScene::items(area, Qt::IntersectsItemShape);
    foreach(QGraphicsItem* item, items) {
        if(item->type()!=QDElementType) {
            items.removeAll(item);
        }
    }
    return items;
}

#define BINDING_AREA 50
void QueryScene::dragEnterEvent(QGraphicsSceneDragDropEvent *) {}

void QueryScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event) {
    const QString& mimeStr = event->mimeData()->text();
    if (mimeStr==QDDistanceIds::E2S || mimeStr==QDDistanceIds::S2E ||
        mimeStr==QDDistanceIds::S2S || mimeStr==QDDistanceIds::E2E) {
        const QList<QGraphicsItem*>& sceneUnitViews = getElements(sceneRect());
        if (sceneUnitViews.size()<2) {
            event->setDropAction(Qt::IgnoreAction);
            return;
        }

        if(dropCandidateLeft && dropCandidateRight) {
            dropCandidateLeft->highlighted = false;
            dropCandidateRight->highlighted = false;
        }
        QPointF mousePos = event->scenePos();

        QRectF leftArea = sceneRect();
        leftArea.setRight(mousePos.x());
        const QList<QGraphicsItem*>& annItemsToLeft = getElements(leftArea);

        QRectF rightArea = sceneRect();
        rightArea.setLeft(mousePos.x());
        const QList<QGraphicsItem*>& annItemsToRight = getElements(rightArea);
        
        qreal delta = sceneRect().width()*sceneRect().width() + sceneRect().height()*sceneRect().height();
        QDElement *src = NULL, *dst = NULL;
        foreach(QGraphicsItem* itLeft, annItemsToLeft) {
            QDElement* leftAnn = qgraphicsitem_cast<QDElement*>(itLeft);
            assert(leftAnn);
            foreach(QGraphicsItem* itRight, annItemsToRight) {
                QDElement* rightAnn = qgraphicsitem_cast<QDElement*>(itRight);
                assert(rightAnn);
                QLineF srcToPos(leftAnn->getRightConnector(), mousePos);
                QLineF dstToPos(rightAnn->getLeftConnector(), mousePos);
                QLineF srcToDst(leftAnn->getRightConnector(), rightAnn->getLeftConnector());
                qreal curDelta = srcToPos.length() + dstToPos.length() - srcToDst.length();
                if(curDelta < delta) {
                    delta = curDelta;
                    src=leftAnn;
                    dst=rightAnn;
                }
            }
        }

        if(delta < BINDING_AREA) {
            dropCandidateLeft = src;
            dropCandidateRight = dst;
            dropCandidateLeft->highlighted = true;
            dropCandidateRight->highlighted = true;
            update();
        }
        event->acceptProposedAction();
    } else if(AppContext::getQDActorProtoRegistry()->getAllIds().contains(mimeStr)) {
        event->acceptProposedAction();
    } else {
        event->acceptProposedAction();
    }
}

void QueryScene::dropEvent(QGraphicsSceneDragDropEvent *event) {
    if(!event->mimeData()->hasText()) {
        return;
    }
    const QString& mimeStr = event->mimeData()->text();
    if (AppContext::getQDActorProtoRegistry()->getAllIds().contains(mimeStr)) {
        QDActorPrototype* proto = AppContext::getQDActorProtoRegistry()->getProto(mimeStr);
        QDActor* actor = proto->createInstance();
        addActor(actor, event->scenePos());
        return;
    } else if (mimeStr==QDDistanceIds::E2E) {
        setupDistanceDialog(E2E);
    } else if (mimeStr==QDDistanceIds::S2S) {
        setupDistanceDialog(S2S);
    } else if (mimeStr==QDDistanceIds::E2S) {
        setupDistanceDialog(E2S);
    } else if (mimeStr==QDDistanceIds::S2E) {
        setupDistanceDialog(S2E);
    }
    if (dropCandidateLeft) {
        dropCandidateLeft->highlighted = false;
    }
    if (dropCandidateRight) {
        dropCandidateRight->highlighted = false;
    }
    dropCandidateLeft = NULL;
    dropCandidateRight = NULL;
}

void QueryScene::setupDistanceDialog(QDDistanceType kind) {
    if(dropCandidateLeft && dropCandidateRight) {
        AddConstraintDialog dlg(this, kind, dropCandidateLeft, dropCandidateRight);
        dlg.exec();
    }
}

#define UNIT_PADDING 30
void QueryScene::addActor(QDActor* actor, const QPointF& pos) {
    int count = 0;
    foreach(QDActor* a, scheme->getActors()) {
        if (a->getActorType() == actor->getActorType()) {
            ++count;
        }
    }
    QDActorParameters* actorCfg = actor->getParameters();
    QString defaultName = actor->getProto()->getDisplayName();
    if (count>0) {
        actorCfg->setLabel(QString("%1%2").arg(defaultName).arg(count));
    }
    else {
        actorCfg->setLabel(defaultName);
    }

    qreal top = annotationsArea().top();
    int rowNum = (pos.y() - top)/GRID_STEP;
    assert(rowNum>=0);
    qreal y = rowNum*GRID_STEP + top;
    scheme->addActor(actor);
    int dx = 0;
    QMap<QDSchemeUnit*, QDElement*> unit2view;
    foreach(QDSchemeUnit* su, actor->getSchemeUnits()) {
        QDElement* uv = new QDElement(su);
        unit2view[su] = uv;
        addItem(uv);
        uv->setPos(pos.x() + dx, y);
        dx += UNIT_PADDING + uv->boundingRect().width();
    }
    foreach(QDConstraint* c, actor->getParamConstraints()) {
        QDDistanceConstraint* dc = static_cast<QDDistanceConstraint*>(c);
        if (dc) {
            QueryViewController::setupConstraintEditor(dc);
            Footnote* fn = new Footnote(unit2view.value(dc->getSource()),
                unit2view.value(dc->getDestination()), dc->distanceType(), c
                );
            addItem(fn);
            fn->updatePos();
        }
    }
    connect(actor->getParameters(), SIGNAL(si_modified()), ruler, SLOT(sl_updateText()));
    emit_schemeChanged();
    setModified(true);
}

void QueryScene::addDistanceConstraint(QDElement* src, QDElement* dst, QDDistanceType distType, int min, int max) {
    if(src!=dst) {
        QList<QDSchemeUnit*> units;
        units << src->getSchemeUnit() << dst->getSchemeUnit();
        QDConstraint* c = new QDDistanceConstraint(units, distType, min, max);
        QueryViewController::setupConstraintEditor(c);
        scheme->addConstraint(c);
        connect(c->getParameters(), SIGNAL(si_modified()), ruler, SLOT(sl_updateText()));
        Footnote* fn = new Footnote(src, dst, distType, c);
        addItem(fn);
        fn->updatePos();
        emit_schemeChanged();
    }
    setModified(true);
}

void QueryScene::removeActor(QDActor* actor) {    
    foreach(QGraphicsItem* it, getElements()) {
        QDElement* uv = qgraphicsitem_cast<QDElement*>(it);
        assert(uv);
        if (uv->getActor()==actor) {
            removeItem(uv);
            delete uv;
        }
    }
    const QList<QDActor*>& actors = scheme->getActors();
    int removedIdx = actors.indexOf(actor);
    scheme->removeActor(actor);
    int actorsNumber = actors.size();
    for (int idx=removedIdx; idx<actorsNumber; idx++) {
        QDActor* a = actors.at(idx);
        scheme->setOrder(a, idx);
        foreach(QDElement* el, getElements()) {
            if (el->getActor()==a) {
                el->sl_refresh();
                break;
            }
        }
    }
    emit_schemeChanged();
    setModified(true);
}

void QueryScene::removeActors(const QList<QDActor*>& actors) {
    foreach(QDActor* a, actors) {
        removeActor(a);
    }
}

void QueryScene::removeConstraint(QDConstraint* constraint) {
    QDSchemeUnit* su = constraint->getSchemeUnits().at(0);
    QDActor* actor = su->getActor();
    assert(su->getConstraints().contains(constraint));
    foreach(QGraphicsItem* it, getFootnotes()) {
        Footnote* fn = qgraphicsitem_cast<Footnote*>(it);
        assert(fn);
        if (fn->getConstraint()==constraint) {
            removeItem(fn);
            delete fn;
        }
    }
    scheme->removeConstraint(constraint);
    emit_schemeChanged();
    setModified(true);
}

void QueryScene::clearScene() {
    removeActors(scheme->getActors());
    scheme->clear();
}

QList<QGraphicsItem*> QueryScene::unitsIntersectedByRow(int idx) const {
    const QRectF& area = annotationsArea();
    QRectF currRow(area.left(), idx*GRID_STEP, area.width(), GRID_STEP);
    currRow.moveTop(currRow.top()+annotationsArea().top());
    QList<QGraphicsItem*> rowItems = items(currRow, Qt::IntersectsItemShape);
    foreach(QGraphicsItem* it, rowItems) {
        if (it->type()!=QDElementType) {
            rowItems.removeAll(it);
        }
    }
    return rowItems;
}

void QueryScene::setModified( bool b ) {
    modified=b;
    if (view) {
        view->enableSaveAction(b);
    }
}

/************************************************************************/
/* View Controller                                                      */
/************************************************************************/
enum {ElementsTab,GroupsTab,SamplesTab};

#define PALETTE_STATE "query_palette_settings"

#define DESCRIPTION_HEIGHT 200
QueryViewController::QueryViewController() : MWMDIWindow(tr("Query Designer")) {
    GCOUNTER(cvar, tvar, "OpenQDWindow");
    scene = new QueryScene(this);

    sceneView = new GlassView(scene);
    sceneView->setDragMode(QGraphicsView::RubberBandDrag);

    palette = new QueryPalette(this);
    groupsEditor = new QDGroupsEditor(this);
    QDSamplesWidget* samples = new QDSamplesWidget(scene, this);

    tabs = new QTabWidget(this);
    tabs->insertTab(ElementsTab, palette, tr("Elements"));
    tabs->insertTab(GroupsTab, groupsEditor, tr("Groups"));
    tabs->insertTab(SamplesTab, samples, tr("Samples"));

    editor = new QueryEditor(this);

    connect(scene, SIGNAL(selectionChanged()), SLOT(sl_editItem()));
    connect(palette, SIGNAL(processSelected(QDActorPrototype*)), SLOT(sl_elementSelected(QDActorPrototype*)));
    connect(samples, SIGNAL(setupGlass(GlassPane*)), sceneView, SLOT(setGlass(GlassPane*)));
    connect(samples, SIGNAL(itemActivated(QDDocument*)), SLOT(sl_pasteSample(QDDocument*)));
    connect(tabs, SIGNAL(currentChanged(int)), samples, SLOT(sl_cancel()));
    connect(editor, SIGNAL(modified()), scene, SLOT(sl_setModified()));

    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(tabs);
    splitter->addWidget(sceneView);
    splitter->addWidget(editor);

    Settings* settings = AppContext::getSettings();
    if (settings->contains(PALETTE_STATE)) {
        palette->restoreState(settings->getValue(PALETTE_STATE));
    }

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(splitter);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    createActions();
    sl_updateTitle();
    
    sl_scrollUp();
    qreal viewportW = sceneView->viewport()->width();
    qreal xp = (viewportW - scene->descTxtItem->boundingRect().width())/2.0;
    scene->descTxtItem->setPos(xp, 0);
}

void QueryViewController::loadScene(const QString& content) {
    QDDocument doc;
    doc.setContent(content);
    QList<QDDocument*> docs;
    docs << &doc;
    QDSceneSerializer::doc2scene(scene, docs);
    scene->setModified(false);
    sl_updateTitle();
}

void QueryViewController::createActions() {
    runAction = new QAction(tr("Run Schema..."), this);
    runAction->setIcon(QIcon(":query_designer/images/run.png"));
    connect(runAction, SIGNAL(triggered()), SLOT(sl_run()));

    newAction = new QAction(tr("New Schema"), this);
    newAction->setShortcuts(QKeySequence::New);
    newAction->setIcon(QIcon(":query_designer/images/filenew.png"));
    connect(newAction, SIGNAL(triggered()), SLOT(sl_newScene()));
    
    loadAction = new QAction(tr("Load Schema..."), this);
    loadAction->setShortcut(QKeySequence("Ctrl+L"));
    loadAction->setIcon(QIcon(":query_designer/images/fileopen.png"));
    connect(loadAction, SIGNAL(triggered()), SLOT(sl_loadScene()));

    saveAction = new QAction(tr("Save Schema"), this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setIcon(QIcon(":query_designer/images/filesave.png"));
    saveAction->setDisabled(true);
    connect(saveAction, SIGNAL(triggered()), SLOT(sl_saveScene()));
    
    saveAsAction = new QAction(tr("Save Schema As..."), this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setIcon(QIcon(":query_designer/images/filesave.png"));
    connect(saveAsAction, SIGNAL(triggered()), SLOT(sl_saveSceneAs()));

    deleteAction = new QAction(tr("Delete"), this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setIcon(QIcon(":query_designer/images/delete.png"));
    connect(deleteAction, SIGNAL(triggered()), SLOT(sl_deleteItem()));

    showLabelAction = new QAction(tr("Show title"), this);
    showLabelAction->setCheckable(true);
    showLabelAction->setChecked(false);
    connect(showLabelAction, SIGNAL(toggled(bool)), scene, SLOT(sl_showLabel(bool)));

    showDescAction = new QAction(tr("Show description"), this);
    showDescAction->setCheckable(true);
    showDescAction->setChecked(false);
    connect(showDescAction, SIGNAL(toggled(bool)), scene, SLOT(sl_showSchemeDesc(bool)));

    showItemDescAction = new QAction(tr("Show element info"), this);
    showItemDescAction->setCheckable(true);
    showItemDescAction->setChecked(true);
    connect(showItemDescAction, SIGNAL(toggled(bool)), scene, SLOT(sl_showItemDesc(bool)));

    showOrderAction = new QAction(tr("Show order"), this);
    showOrderAction->setCheckable(true);
    showOrderAction->setChecked(true);
    connect(showOrderAction, SIGNAL(toggled(bool)), scene, SLOT(sl_showOrder(bool)));

    strandActions = new QActionGroup(this);

    directStrandAction = new QAction(tr("Direct strand"), strandActions);
    directStrandAction->setCheckable(true);

    complementStrandAction = new QAction(tr("Reverse complementary strand"), strandActions);
    complementStrandAction->setCheckable(true);

    bothStrandsAction = new QAction(tr("Both strands"), strandActions);
    bothStrandsAction->setCheckable(true);

    QDStrandOption strand = scene->getScheme()->getStrand();
    switch (strand)
    {
    case QDStrand_Both:
        bothStrandsAction->setChecked(true);
        break;
    case QDStrand_DirectOnly:
        directStrandAction->setChecked(true);
        break;
    case QDStrand_ComplementOnly:
        complementStrandAction->setChecked(true);
        break;
    default:
        assert(0);
        break;
    }
    connect(strandActions, SIGNAL(triggered(QAction*)), SLOT(sl_setGlobalStrand(QAction*)));
}

void QueryViewController::setupViewModeMenu(QMenu* m) {
    m->addAction(showLabelAction);
    m->addAction(showDescAction);
    m->addAction(showItemDescAction);
    m->addAction(showOrderAction);
}

void QueryViewController::setupQuerySequenceModeMenu(QMenu* m) {
    m->addAction(directStrandAction);
    m->addAction(complementStrandAction);
    m->addAction(bothStrandsAction);
}

void QueryViewController::setupStrandMenu(QMenu* m) {
    m->addActions(strandActions->actions());
}

void QueryViewController::setupMDIToolbar(QToolBar* tb) {
    tb->addAction(newAction);
    tb->addAction(loadAction);
    tb->addAction(saveAction);
    tb->addAction(saveAsAction);
    tb->addSeparator();

    tb->addAction(runAction);
    tb->addSeparator();

    QToolButton* tt = new QToolButton(tb);
    QMenu* viewModeMenu = new QMenu(tr("View Mode"), this);
    setupViewModeMenu(viewModeMenu);
    QAction* a = viewModeMenu->menuAction();
    tt->setDefaultAction(a);
    tt->setPopupMode(QToolButton::InstantPopup);
    tt->setIcon(QIcon(":query_designer/images/eye.png"));
    tb->addWidget(tt);

    QToolButton* st = new QToolButton(tb);
    QMenu* strandMenu = new QMenu(tr("Query Sequence Mode"), this);
    setupStrandMenu(strandMenu);
    QAction* sa = strandMenu->menuAction();
    st->setDefaultAction(sa);
    st->setPopupMode(QToolButton::InstantPopup);
    st->setIcon(QIcon(":query_designer/images/strands.png"));
    tb->addWidget(st);

    tb->addSeparator();
    tb->addAction(deleteAction);
}

void QueryViewController::setupViewMenu(QMenu* m) {
    m->addAction(newAction);
    m->addAction(loadAction);
    m->addAction(saveAction);
    m->addAction(saveAsAction);
    m->addSeparator();
    m->addAction(runAction);
    m->addSeparator();

    QMenu* viewModeMenu = new QMenu(tr("View Mode"), this);
    viewModeMenu->setIcon(QIcon(":query_designer/images/eye.png"));
    setupViewModeMenu(viewModeMenu);
    m->addMenu(viewModeMenu);

    QMenu* querySequenceModeMenu = new QMenu(tr("Query Sequence Mode"), this);
    querySequenceModeMenu->setIcon((QIcon(":query_designer/images/strands.png")));
    setupQuerySequenceModeMenu(querySequenceModeMenu);
    m->addMenu(querySequenceModeMenu);

    m->addSeparator();
    m->addAction(deleteAction);
    m->addSeparator();
}

void QueryViewController::switchToGroupsTab() {
    tabs->setCurrentIndex(GroupsTab);
}

bool QueryViewController::onCloseEvent() {
    saveState();
    return confirmModified();
}

void QueryViewController::saveState() {
    AppContext::getSettings()->setValue(PALETTE_STATE, palette->saveState());
}

void QueryViewController::sl_run() {
    QDScheme* scheme = scene->getScheme();
    if (scheme->isEmpty()) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("The schema is empty!"));
    } else if (!scheme->isValid()) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("The schema is invalid! Please see the log for details."));
    } else {
        QDRunDialog runDlg(scene->getScheme(), this, inFile_, outFile_);
        runDlg.exec();
    }
}

void QueryViewController::sl_newScene() {
    if(!scene->getScheme()->getActors().isEmpty()) {
        if (!confirmModified()) {
            return;
        }
    }
    schemeUri.clear();
    scene->setLabel("NewSchema");
    scene->setDescription("<Insert description here>");
    scene->clearScene();
    sl_updateTitle();

}

void QueryViewController::sl_loadScene() {
    if(!scene->getScheme()->getActors().isEmpty()) {
        if (!confirmModified()) {
            return;
        }
    }
    LastOpenDirHelper dir(QUERY_DESIGNER_ID);
    dir.url = QFileDialog::getOpenFileName(this, tr("Load Schema"), dir, QString("*.%1").arg(QUERY_SCHEME_EXTENSION));
    if (!dir.url.isEmpty()) {
        QDLoadSceneTask* t = new QDLoadSceneTask(scene, dir.url);
        connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_updateTitle()));
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
        scene->setModified(false);
        schemeUri = dir.url;
    }
}

void QueryViewController::sl_saveScene() {
    if (schemeUri.isEmpty()) {
        sl_saveSceneAs();
    } else {
        QDSceneInfo info;
        info.path = schemeUri;
        info.schemeName = scene->getLabel();
        info.description = scene->getDescription();
        QDSaveSceneTask* t = new QDSaveSceneTask(scene, info);
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
        scene->setModified(false);
    }
}

void QueryViewController::sl_saveSceneAs() {
    LastOpenDirHelper dir(QUERY_DESIGNER_ID);
    dir.url = QFileDialog::getSaveFileName(this, tr("Save Schema"), dir, QString("*.%1").arg(QUERY_SCHEME_EXTENSION));
    if (!dir.url.isEmpty()) {
        schemeUri = dir.url;
        sl_saveScene();
    }
}

void QueryViewController::sl_deleteItem() {
    QList<QDActor*> actors2remove;
    QList<QDConstraint*> constraints2remove;
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    foreach(QGraphicsItem* item, selectedItems) {
        switch (item->type()) {
            case QDElementType:
            {
                QDElement* uv = qgraphicsitem_cast<QDElement*>(item);
                assert(uv);
                QDActor* a = uv->getActor();
                if(!actors2remove.contains(a)) {
                    actors2remove.append(a);
                }
            }
            break;
            case FootnoteItemType:
            {
                Footnote* fn = qgraphicsitem_cast<Footnote*>(item);
                assert(fn);
                QDConstraint* c = fn->getConstraint();
                if (!constraints2remove.contains(c)) {
                    constraints2remove.append(c);
                }
            }
            break;
            default:
                break;
        }
    }

    QList<QDConstraint*> removedConstraints;
    foreach (QDConstraint* c, constraints2remove) {
        if (removedConstraints.contains(c)) {
            continue;
        }
        QDSchemeUnit* su = c->getSchemeUnits().at(0);
        QDActor* actor = su->getActor();
        if (!su->getConstraints().contains(c)) { //param constraint
            actors2remove.removeAll(actor);
            removedConstraints << actor->getConstraints();
            scene->removeActor(actor);
            continue;
        }
        scene->removeConstraint(c);
    }
    scene->removeActors(actors2remove);
    scene->setModified(true);
}

void QueryViewController::sl_editItem() {
    const QList<QGraphicsItem*>& selectedItems = scene->selectedItems();
    if(1==selectedItems.size()) {
        QGraphicsItem* selectedItem = selectedItems.at(0);
        if(selectedItem->type()==QDElementType) {
            QDElement* unitView = qgraphicsitem_cast<QDElement*>(selectedItem);
            QDActor* a = unitView->getSchemeUnit()->getActor();
            editor->edit(a);
        }
        if(selectedItem->type()==FootnoteItemType) {
            Footnote* fn = qgraphicsitem_cast<Footnote*>(selectedItem);
            QDConstraint* con = fn->getConstraint();
            editor->edit(con);
        }
    }
    else {
        editor->reset();
    }
}

void QueryViewController::sl_elementSelected(QDActorPrototype* proto) {
    scene->clearSelection();
    editor->showProto(proto);
}

void QueryViewController::sl_pasteSample(QDDocument* content) {
    if(!scene->getScheme()->getActors().isEmpty()) {
        if (!confirmModified()) {
            return;
        }
    }
    tabs->setCurrentIndex(ElementsTab);
    scene->clearScene();
    QList<QDDocument*> docList = ( QList<QDDocument*>() << content );
    QDSceneSerializer::doc2scene(scene, docList);
    sl_updateTitle();
    scene->setModified(false);
    schemeUri.clear();
}

void QueryViewController::sl_selectEditorCell(const QString& link) {
    editor->setCurrentAttribute(link);
}

void QueryViewController::sl_updateTitle() {
    setWindowTitle(tr("Query Designer - %1").arg(scene->getLabel()));
}

void QueryViewController::sl_setGlobalStrand(QAction* a) {
    QDScheme* scheme = scene->getScheme();
    QDStrandOption old = scheme->getStrand();
    if (a==bothStrandsAction) {
        scheme->setStrand(QDStrand_Both);
    } else if (a==directStrandAction) {
        scheme->setStrand(QDStrand_DirectOnly);
    } else {
        assert(a==complementStrandAction);
        scheme->setStrand(QDStrand_ComplementOnly);
    }
    if (scheme->getStrand()!=old) {
        scene->setModified(true);
    }
}

void QueryViewController::sl_scrollUp() {
    QPointF topLeft = scene->sceneRect().topLeft();
    QSize s = sceneView->viewport()->rect().size();
    QRectF topRect(topLeft, s);
    sceneView->ensureVisible(topRect);
}

void QueryViewController::setupConstraintEditor(QDConstraint* c) {
    if (c->constraintType()==QDConstraintTypes::DISTANCE) {
        QMap<QString,PropertyDelegate*> delegates;
        {
            QVariantMap lenMap;
            lenMap["minimum"] = QVariant(0);
            lenMap["maximum"] = QVariant(INT_MAX);
            lenMap["suffix"] = L10N::suffixBp();
            delegates[QDConstraintController::MIN_LEN_ATTR] = new SpinBoxDelegate(lenMap);
            delegates[QDConstraintController::MAX_LEN_ATTR] = new SpinBoxDelegate(lenMap);
        }
        c->setUIEditor(new DelegateEditor(delegates));
    }
}

bool QueryViewController::confirmModified() {
    if (scene->isModified()) {
        AppContext::getMainWindow()->getMDIManager()->activateWindow(this);
        int ret = QMessageBox::question(this, tr("Query Designer"),
            tr("The schema has been modified.\n"
            "Do you want to save changes?"),
            QMessageBox::Save | QMessageBox::Discard
            | QMessageBox::Cancel,
            QMessageBox::Save);
        if (QMessageBox::Cancel == ret) {
            return false;
        } else if (QMessageBox::Save == ret){
            sl_saveScene();
        }
    }
    return true;
}

void QueryViewController::enableSaveAction( bool enable ) {
    if (saveAction) {
        saveAction->setEnabled(enable);
    }
}

/************************************************************************/
/* AddConstraintDialog                                                  */
/************************************************************************/

AddConstraintDialog::AddConstraintDialog(QueryScene* _scene, QDDistanceType _kind,
                                         QDElement* defSrc, QDElement* defDst)
: scene(_scene), kind(_kind) {
    setupUi(this);

    QString title = "Add %1 Constraint";
    switch (kind)
    {
    case E2S:
        setWindowTitle(title.arg("'" + tr("End-Start") + "'"));
        break;
    case E2E:
        setWindowTitle(title.arg("'" + tr("End-End") + "'"));
        break;
    case S2E:
        setWindowTitle(title.arg("'" + tr("Start-End") + "'"));
        break;
    case S2S:
        setWindowTitle(title.arg("'" + tr("Start-Start") + "'"));
        break;
    default:
        break;
    }

    maxSpin->setMaximum(INT_MAX);
    minSpin->setMaximum(INT_MAX);
    const QList<QDElement*>& elements = scene->getElements();
    int index = 0;
    foreach(QDElement* el, elements) {
        const QVariant& data = qVariantFromValue(el);
        QDActor* a = el->getActor();
        QString name = a->getParameters()->getLabel();
        if (a->getSchemeUnits().size()>1) {
            name += QString(".%1").arg(a->getUnitId(el->getSchemeUnit()));
        }

        fromCBox->insertItem(index, name);
        fromCBox->setItemData(index, data);
        toCBox->insertItem(index, name);
        toCBox->setItemData(index, data);
        ++index;
    }

    assert(elements.contains(defSrc));
    assert(elements.contains(defDst));
    fromCBox->setCurrentIndex(elements.indexOf(defSrc));
    toCBox->setCurrentIndex(elements.indexOf(defDst));
}

void AddConstraintDialog::accept() {
    int min = minSpin->text().toInt();
    int max = maxSpin->text().toInt();
    QDElement* src = qVariantValue<QDElement*>(fromCBox->itemData(fromCBox->currentIndex()));
    QDElement* dst = qVariantValue<QDElement*>(toCBox->itemData(toCBox->currentIndex()));
    scene->addDistanceConstraint(src, dst, kind, min, max);
    QDialog::accept();
}

/************************************************************************/
/* GUIUtils                                                             */
/************************************************************************/

QPixmap QDUtils::generateSnapShot(QDDocument* doc, const QRect& rect) {
    QueryScene scene;
    QList<QDDocument*> docs = (QList<QDDocument*>() << doc);
    QDSceneSerializer::doc2scene(&scene, docs);
    return generateSnapShot(&scene, rect);
}

QPixmap QDUtils::generateSnapShot( QueryScene* scene, const QRect& rect) {
    //assert(!rect.isNull());
    QRectF bounds; 
    foreach(QGraphicsItem* item, scene->items()) {
        if (item->type()==QDElementType || item->type()==FootnoteItemType) {
            QRectF itemBound = item->boundingRect();
            QPointF pos = item->scenePos();
            itemBound.moveTopLeft(pos);
            bounds |= itemBound;
            if (bounds.width() > MAX_ITEM_SIZE || bounds.height() > MAX_ITEM_SIZE) {
                uiLog.trace(QString("Illegal QD item size, stop rendering preview!"));
                break;
            }
        }
    }

    QPixmap pixmap(bounds.size().toSize());
    if (pixmap.isNull()) { // failed to allocate 
        uiLog.trace(QString("Failed to allocate pixmap for the QD scene, bounds: x:%1 y:%2 w:%3 h:%4")
            .arg(bounds.x()).arg(bounds.y()).arg(bounds.width()).arg(bounds.height()));
        QPixmap naPixmap = QPixmap(rect.size());
        naPixmap.fill();
        QPainter p(&naPixmap);
        p.drawText(naPixmap.rect(), Qt::AlignHCenter | Qt::AlignTop, QueryScene::tr("Preview is not available."));
        return naPixmap;
    }
    pixmap.fill();
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter, rect, bounds);
    return pixmap;
}

}//namespace
