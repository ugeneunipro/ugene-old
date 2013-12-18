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

#include "ADVSingleSequenceWidget.h"
#include "ADVSequenceObjectContext.h"
#include "AnnotatedDNAView.h"

#include "ADVConstants.h"
#include "DetView.h"
#include "Overview.h"
#include "CreateRulerDialogController.h"

#include <U2Core/L10n.h>
#include <U2Core/DocumentModel.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/AnnotationSelection.h>
#include <U2Core/GHints.h>
#include <U2Core/DNAAlphabet.h>

#include <U2Gui/RangeSelector.h>
#include <U2Gui/PositionSelector.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/ExportImageDialog.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/HBar.h>

#include <QtGui/QApplication>
#include <QtGui/QToolButton>
#include <QtGui/QWidgetAction>
#include <QtGui/QDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include "GSequenceGraphView.h"

namespace U2 {
#define ADV_HEADER_HEIGHT 24
#define IMAGE_DIR   "image"

ADVSingleSequenceWidget::ADVSingleSequenceWidget(ADVSequenceObjectContext* seqCtx, AnnotatedDNAView* ctx) : ADVSequenceWidget(ctx) {
    seqContexts.append(seqCtx);

    toggleViewAction = new QAction(this);
    connect(toggleViewAction, SIGNAL(triggered()), SLOT(sl_toggleView()));

    togglePanViewAction = new QAction(this);
    connect(togglePanViewAction, SIGNAL(triggered()), SLOT(sl_togglePanView()));

    toggleDetViewAction = new QAction(this);
    connect(toggleDetViewAction, SIGNAL(triggered()), SLOT(sl_toggleDetView()));

    toggleOverviewAction = new QAction(this);
    connect(toggleOverviewAction, SIGNAL(triggered()), SLOT(sl_toggleOverview()));

    connect(seqCtx->getAnnotatedDNAView()->getAnnotationsSelection(), 
        SIGNAL(si_selectionChanged(AnnotationSelection* , const QList<Annotation*>&, const QList<Annotation*>&)),
        SLOT(sl_onAnnotationSelectionChanged(AnnotationSelection* , const QList<Annotation*>&, const QList<Annotation*>&)));

    selectRangeAction1 = new QAction(QIcon(":/core/images/select_region.png"), tr("Select sequence region..."), this);
    selectRangeAction1->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
    selectRangeAction1->setObjectName("select_range_action");
    selectRangeAction1->setShortcutContext(Qt::WidgetShortcut);
    connect(selectRangeAction1, SIGNAL(triggered()), SLOT(sl_onSelectRange()));

    selectRangeAction2 = new QAction(QIcon(":/core/images/select_region.png"), tr("Sequence region..."), this);
    selectRangeAction2->setObjectName("Sequence region");
    connect(selectRangeAction2, SIGNAL(triggered()), SLOT(sl_onSelectRange()));

    selectInAnnotationRangeAction= new QAction(tr("Sequence between selected annotations"), this);
    selectInAnnotationRangeAction->setObjectName("Sequence between selected annotations");
    connect(selectInAnnotationRangeAction, SIGNAL(triggered()), SLOT(sl_onSelectInRange()));
    
    selectOutAnnotationRangeAction= new QAction(tr("Sequence around selected annotations"), this);
    selectOutAnnotationRangeAction->setObjectName("Sequence around selected annotations");
    connect(selectOutAnnotationRangeAction, SIGNAL(triggered()), SLOT(sl_onSelectOutRange()));

    zoomToRangeAction = new QAction(QIcon(":/core/images/zoom_reg.png"), tr("Zoom to range.."), this);
    zoomToRangeAction->setObjectName("zoom_to_range_" + getSequenceObject()->getGObjectName());
    connect(zoomToRangeAction, SIGNAL(triggered()), SLOT(sl_zoomToRange()));

    createNewRulerAction = new QAction(tr("Create new ruler..."), this);
    createNewRulerAction->setObjectName("Create new ruler");
    connect(createNewRulerAction, SIGNAL(triggered()), SLOT(sl_createCustomRuler()));

    linesLayout = new QVBoxLayout();
    linesLayout->setMargin(0);
    linesLayout->setSpacing(0);
    setLayout(linesLayout);
    headerWidget = new ADVSingleSequenceHeaderWidget(this);
    headerWidget->installEventFilter(this);
    linesLayout->addWidget(headerWidget);

    init();
}

void ADVSingleSequenceWidget::init() {
    ADVSequenceObjectContext* seqCtx = getSequenceContext();
    detView = new DetView(this, seqCtx);
    detView->setObjectName("det_view_" + getSequenceObject()->getGObjectName());
    detView->setMouseTracking(true);
    addSequenceView(detView, headerWidget);


    panView = new PanView(this, seqCtx);
    panView->setObjectName("pan_view_" + getSequenceObject()->getGObjectName());
    connect(panView, SIGNAL(si_centerPosition(qint64)), SLOT(sl_onLocalCenteringRequest(qint64)));

    zoomUseObject.setPanView(panView);

    addSequenceView(panView, headerWidget);

    panView->setFrameView(detView);

    overview = new Overview(this, seqCtx);
    overview->setObjectName("overview_" + getSequenceObject()->getGObjectName());
    overview->setMouseTracking(true);
    addSequenceView(overview, headerWidget);
    
    setFixedHeight(linesLayout->minimumSize().height());

    buttonTabOrederedNames = new QList<QString>;

    QToolBar* hBar = headerWidget->getToolBar();
    
    addButtonWithActionToToolbar(selectRangeAction1, hBar);
    buttonTabOrederedNames->append(selectRangeAction1->objectName());
    hBar->addSeparator();

    if (seqCtx->getComplementTT() != NULL) {
        addButtonWithActionToToolbar(detView->getShowComplementAction(), hBar);
        buttonTabOrederedNames->append(detView->getShowComplementAction()->objectName());
    }

    if (seqCtx->getAminoTT() != NULL) {
        addButtonWithActionToToolbar(detView->getShowTranslationAction(), hBar);
        buttonTabOrederedNames->append(detView->getShowTranslationAction()->objectName());
    }

    if (seqCtx->getAminoTT() != NULL) {        
        QMenu* ttMenu = seqCtx->createTranslationsMenu();
        tbMenues.append(ttMenu);
        QToolButton* button = addButtonWithActionToToolbar(ttMenu->menuAction(), hBar);
        SAFE_POINT(button, QString("ToolButton for %1 is NULL").arg(ttMenu->menuAction()->objectName()), );
        button->setPopupMode(QToolButton::InstantPopup);
        button->setObjectName("AminoToolbarButton");
        buttonTabOrederedNames->append(ttMenu->menuAction()->objectName());
        hBar->addSeparator();
    } else {
        ttButton = NULL;
    }

    QAction* shotScreenAction = new QAction(QIcon(":/core/images/cam2.png"), tr("Capture screen"), this);
    shotScreenAction->setObjectName("capture_screen");
    connect(shotScreenAction, SIGNAL(triggered()), this, SLOT(sl_saveScreenshot()));
    
    addButtonWithActionToToolbar(shotScreenAction, hBar);
    buttonTabOrederedNames->append(shotScreenAction->objectName());

    addButtonWithActionToToolbar(panView->getZoomInAction(), hBar);
    buttonTabOrederedNames->append(panView->getZoomInAction()->objectName());

    addButtonWithActionToToolbar(panView->getZoomOutAction(), hBar);
    buttonTabOrederedNames->append(panView->getZoomOutAction()->objectName());

    addButtonWithActionToToolbar(zoomToRangeAction, hBar);
    buttonTabOrederedNames->append(zoomToRangeAction->objectName());

    addButtonWithActionToToolbar(panView->getZoomToSequenceAction(), hBar);
    buttonTabOrederedNames->append(panView->getZoomToSequenceAction()->objectName());

    QToolButton* panViewToolButton = addButtonWithActionToToolbar(panView->getPanViewActions(), hBar);
    SAFE_POINT(panViewToolButton, "Pan view tool button is NULL", );
    panViewToolButton->setPopupMode(QToolButton::InstantPopup);
    buttonTabOrederedNames->append(panView->getPanViewActions()->objectName());

    widgetStateMenuAction = new QAction(QIcon(":core/images/adv_widget_menu.png"), tr("Toggle view"), this);
    widgetStateMenuAction->setObjectName("toggle_view_button_" + getSequenceObject()->getGObjectName());
    widgetStateMenuAction->setToolTip(tr("Toggle view"));
    connect(widgetStateMenuAction, SIGNAL(triggered()), SLOT(sl_showStateMenu()));
    widgetStateMenuButton = addButtonWithActionToToolbar(widgetStateMenuAction, hBar);
    SAFE_POINT(widgetStateMenuButton, "widgetStateMenuButton is NULL", );
    widgetStateMenuButton->setFixedWidth(20);
    buttonTabOrederedNames->append(widgetStateMenuAction->objectName());

    closeViewAction = new QAction(tr("Remove sequence"), this);
    closeViewAction->setObjectName("remove_sequence");
    connect(closeViewAction, SIGNAL(triggered()), SLOT(sl_closeView()));

    dynamic_cast<HBar *>(hBar)->setButtonTabOrderList(buttonTabOrederedNames);

    updateSelectionActions();

#define MIN_SEQUENCE_LEN_TO_USE_FULL_MODE 100
    if (seqCtx->getSequenceLength() < MIN_SEQUENCE_LEN_TO_USE_FULL_MODE) {
        //sequence is rather small -> show panview only by default
        setOverviewCollapsed(true);
        setDetViewCollapsed(true);
    }
}


ADVSingleSequenceWidget::~ADVSingleSequenceWidget() {
    foreach(QMenu* m, tbMenues) {
        delete m;
    }

    delete buttonTabOrederedNames;
}

QToolButton* ADVSingleSequenceWidget::addButtonWithActionToToolbar(QAction * buttonAction, QToolBar * toolBar) const {
    SAFE_POINT(NULL != buttonAction, "buttonAction is NULL", NULL);
    SAFE_POINT(NULL != toolBar, "toolBar is NULL", NULL);
    SAFE_POINT(!buttonAction->objectName().isEmpty(), "Action's object name is empty", NULL);

    toolBar->addAction(buttonAction);
    QToolButton* button = qobject_cast<QToolButton*>(toolBar->widgetForAction(buttonAction));

    SAFE_POINT(button, QString("ToolButton for %1 is NULL").arg(buttonAction->objectName()), NULL);
    button->setObjectName(buttonAction->objectName());

    return button;
}

bool ADVSingleSequenceWidget::isPanViewCollapsed() const {
    return panView->isHidden();
}

bool ADVSingleSequenceWidget::isDetViewCollapsed() const {
    return detView->isHidden();
}

bool ADVSingleSequenceWidget::isOverviewCollapsed() const {
    return overview->isHidden();
}

bool ADVSingleSequenceWidget::isViewCollapsed() const {
    bool collapsed = true;
    foreach(GSequenceLineView* seqView, lineViews) {
        if (seqView->isVisible()) {
            collapsed = false;
            break;
        }
    }
    return collapsed;
}

void ADVSingleSequenceWidget::setViewCollapsed(bool v) {   
    if (ttButton != NULL) {
        getSequenceContext()->setTranslationsVisible(!v);
    }
    foreach(GSequenceLineView* seqView, lineViews) {
        seqView->setHidden(v);
    }
    detView->setDisabledDetViewActions(v);
    updateMinMaxHeight();
}

void ADVSingleSequenceWidget::setPanViewCollapsed(bool v) {
    panView->setHidden(v);
    updateMinMaxHeight();

    if (isPanViewCollapsed()) {
        zoomUseObject.releaseZoom();
    }
    else {
        zoomUseObject.useZoom();
    }
    zoomToRangeAction->setDisabled( isPanViewCollapsed() );
}

void ADVSingleSequenceWidget::setDetViewCollapsed(bool v) {    
    if (ttButton != NULL) {
        getSequenceContext()->setTranslationsVisible(!v);
    }
    detView->setHidden(v);
    detView->setDisabledDetViewActions(v);

    updateMinMaxHeight();
}

void ADVSingleSequenceWidget::setOverviewCollapsed(bool v) {
    overview->setHidden(v);
    updateMinMaxHeight();
}

void ADVSingleSequenceWidget::addSequenceView(GSequenceLineView* v, QWidget* after) {
    assert(!lineViews.contains(v));
    lineViews.append(v);
    if (after==NULL) {
        if(lineViews.size()>1) {
            linesLayout->insertWidget(2, v);
        }
        else {
            linesLayout->insertWidget(1, v);
        }
    }
    else {
        int after_ = linesLayout->indexOf(after);
        assert(after_!=-1);
        linesLayout->insertWidget(after_+1, v);
    }
    v->setVisible(true);
    v->installEventFilter(this);
    updateMinMaxHeight();
    connect(v, SIGNAL(destroyed(QObject*)), SLOT(sl_onViewDestroyed(QObject*)));
}

void ADVSingleSequenceWidget::removeSequenceView(GSequenceLineView* v, bool deleteView) {
    assert(lineViews.contains(v));
    lineViews.removeOne(v);
    linesLayout->removeWidget(v);
    v->setVisible(false);
    v->disconnect(this);
    v->removeEventFilter(this);
    if (deleteView) {
        delete v;
    } 
    updateMinMaxHeight();
}

U2Region ADVSingleSequenceWidget::getVisibleRange() const {
    return panView->getVisibleRange();
}

void ADVSingleSequenceWidget::setVisibleRange(const U2Region& r) {
    return panView->setVisibleRange(r);
}

int ADVSingleSequenceWidget::getNumBasesVisible() const {
    return panView->getVisibleRange().length;
}

void ADVSingleSequenceWidget::setNumBasesVisible(qint64 n) {
    panView->setNumBasesVisible(n);
}


void ADVSingleSequenceWidget::sl_onViewDestroyed(QObject* o) {
    GSequenceLineView* v = static_cast<GSequenceLineView*>(o);
    bool r = lineViews.removeOne(v);
    linesLayout->removeWidget(v);//need here for updateMinMaxHeight
    assert(r);
    Q_UNUSED(r);
    updateMinMaxHeight();
}

void ADVSingleSequenceWidget::centerPosition(int pos, QWidget* skipView) {
    foreach(GSequenceLineView* v, lineViews) {
        if (v == skipView) {
            continue;
        }
        v->setCenterPos(pos);
    }
}

void ADVSingleSequenceWidget::updateMinMaxHeight() {
    int height = linesLayout->minimumSize().height();
    setFixedHeight(height);
}


void ADVSingleSequenceWidget::addZoomMenu(const QPoint& globalPos, QMenu* m) {
    GSequenceLineView* lineView = findSequenceViewByPos(globalPos);
    if (lineView == NULL) {
        return;
    }

    QAction* first = m->actions().isEmpty() ? NULL : m->actions().first();

    QAction * zoomInAction = lineView->getZoomInAction();
    QAction * zoomOutAction = lineView->getZoomOutAction();
    QAction * zoomToSelection = lineView->getZoomToSelectionAction();
    QAction * zoomToSequence = lineView->getZoomToSequenceAction();

    if (zoomInAction == NULL && zoomOutAction == NULL && zoomToSelection == NULL && zoomToSequence == NULL) {
        return;
    }

    QMenu* zm = m->addMenu(tr("Zoom"));

    if (zoomInAction!=NULL) {
        zm->insertAction(first, zoomInAction);
    }
    if (zoomOutAction!=NULL) {
        zm->insertAction(first, zoomOutAction);
    }
    if (zoomToSelection!=NULL) {
        zm->insertAction(first, zoomToSelection);
    }
    if (lineView == panView || lineView->getConherentRangeView() == panView) {
        zm->insertAction(first, zoomToRangeAction);
    }
    if (zoomToSequence!=NULL) {
        zm->insertAction(first, zoomToSequence);
    }
    zm->menuAction()->setObjectName(ADV_MENU_ZOOM);
    m->addSeparator();
}

GSequenceLineView* ADVSingleSequenceWidget::findSequenceViewByPos(const QPoint& globalPos) const {
    Q_UNUSED(globalPos);
    assert(0);
    return NULL;
}

int ADVSingleSequenceWidget::getSequenceLength() const {
    return getSequenceContext()->getSequenceLength();
}


DNATranslation* ADVSingleSequenceWidget::getComplementTT() const {
    ADVSequenceObjectContext* seqCtx = getSequenceContext();
    return seqCtx->getComplementTT();
}

DNATranslation* ADVSingleSequenceWidget::getAminoTT() const {
    ADVSequenceObjectContext* seqCtx = getSequenceContext();
    return seqCtx->getAminoTT();
}

DNASequenceSelection* ADVSingleSequenceWidget::getSequenceSelection() const {
    ADVSequenceObjectContext* seqCtx = getSequenceContext();
    return seqCtx->getSequenceSelection();
}

U2SequenceObject* ADVSingleSequenceWidget::getSequenceObject() const {
    ADVSequenceObjectContext* seqCtx = getSequenceContext();
    return seqCtx->getSequenceObject();
}

GSequenceLineView* ADVSingleSequenceWidget::getPanGSLView() const {
    return panView;
}

GSequenceLineView* ADVSingleSequenceWidget::getDetGSLView() const {
    return detView;
}

void ADVSingleSequenceWidget::buildPopupMenu(QMenu& m) {
    m.insertAction(GUIUtils::findActionAfter(m.actions(), ADV_GOTO_ACTION), getAnnotatedDNAView()->getCreateAnnotationAction());
    m.insertAction(GUIUtils::findActionAfter(m.actions(), ADV_GOTO_ACTION), selectRangeAction1);

    addSelectMenu(m);

    if (panView->isVisible()) {
        addRulersMenu(m);
    }

    ADVSequenceWidget::buildPopupMenu(m);
    foreach(GSequenceLineView* v, lineViews) {
        v->buildPopupMenu(m);
    }
}

void ADVSingleSequenceWidget::addSelectMenu(QMenu& m) {
    QMenu* selectMenu = new QMenu(tr("Select"),  &m);
    selectMenu->menuAction()->setObjectName("Select");
    
    selectMenu->addAction(selectRangeAction2);
    selectMenu->addAction(selectInAnnotationRangeAction);
    selectMenu->addAction(selectOutAnnotationRangeAction);

    QAction* aBefore = GUIUtils::findActionAfter(m.actions(), ADV_MENU_COPY);
    m.insertMenu(aBefore, selectMenu);
}

void ADVSingleSequenceWidget::addRulersMenu(QMenu& m) {
    qDeleteAll(rulerActions.qlist);
    rulerActions.qlist.clear();

    QMenu* rulersM = new QMenu(tr("Rulers..."),  &m);
    rulersM->menuAction()->setObjectName("Rulers");
    rulersM->setIcon(QIcon(":core/images/ruler.png"));
    
    rulersM->addAction(createNewRulerAction);
    rulersM->addSeparator();
    rulersM->addAction(panView->getToggleMainRulerAction());
    rulersM->addAction(panView->getToggleCustomRulersAction());
    rulersM->addSeparator();

    foreach(const RulerInfo& ri, panView->getCustomRulers()) {
        QAction* rulerAction = new QAction(tr("Remove '%1'").arg(ri.name), this);
        rulerAction->setData(ri.name);
        connect(rulerAction, SIGNAL(triggered()), SLOT(sl_removeCustomRuler()));
        rulerActions.qlist.append(rulerAction);
    }
    rulersM->addActions(rulerActions.qlist);

    QAction* aBefore = GUIUtils::findActionAfter(m.actions(), ADV_MENU_SECTION2_SEP);
    m.insertMenu(aBefore, rulersM);
    m.insertSeparator(aBefore)->setObjectName("SECOND_SEP");
}

bool ADVSingleSequenceWidget::isWidgetOnlyObject(GObject* o) const {
    foreach(GSequenceLineView* v, lineViews) {
        ADVSequenceObjectContext *ctx = v->getSequenceContext();
        if(ctx->getSequenceGObject() == o) {
            return true;
        }
    }
    return false;
}


bool ADVSingleSequenceWidget::eventFilter(QObject* o, QEvent* e) {
    QEvent::Type t = e->type();
    if (t == QEvent::Resize) {
        GSequenceLineView* v = qobject_cast<GSequenceLineView*>(o);
        if (lineViews.contains(v)) {
            updateMinMaxHeight();
        }
    } else if (t == QEvent::FocusIn || t == QEvent::MouseButtonPress || t == QEvent::MouseButtonRelease) {
        ctx->setFocusedSequenceWidget(this);
    } 

    if (o == headerWidget && t == QEvent::MouseButtonPress) {
        QMouseEvent* event = dynamic_cast<QMouseEvent*>(e);
        CHECK(event, false);
        if (event->buttons() == Qt::LeftButton) {
            emit si_titleClicked(this);
        }
    }
    return false;
}

void ADVSingleSequenceWidget::sl_onLocalCenteringRequest(qint64 pos) {
    detView->setCenterPos(pos);
}

void ADVSingleSequenceWidget::addADVSequenceWidgetAction(ADVSequenceWidgetAction* a) {
    ADVSequenceWidget::addADVSequenceWidgetAction(a);
    if (a->addToBar) {
        QToolBar* tb = headerWidget->getToolBar();
        tb->insertAction(tb->actions().first(), a);
        QToolButton* tt = qobject_cast<QToolButton*>(tb->widgetForAction(a));
        SAFE_POINT(tt, QString("ToolButton for %1 is NULL").arg(a->objectName()), );
        tt->setObjectName(a->objectName());
        
        if (a->menu() != NULL) {
            tt->setPopupMode(QToolButton::InstantPopup);
        }

        buttonTabOrederedNames->prepend(a->objectName());
    }
}

void ADVSingleSequenceWidget::sl_onSelectRange() {

    ADVSequenceObjectContext* ctx = getSequenceContext();
    DNASequenceSelection* selection = ctx->getSequenceSelection();

    qint64 wholeSeqLen = ctx->getSequenceLength();
    const QVector<U2Region>& seqRegions = selection->getSelectedRegions();
    MultipleRangeSelector mrs(this, seqRegions, wholeSeqLen);
    
    if(mrs.exec() == QDialog::Accepted){
        QVector<U2Region> curRegions = mrs.getSelectedRegions();
        if(curRegions.isEmpty()){
            return;
        }
        if(curRegions.size() == 1){
            U2Region r = curRegions.first();
            setSelectedRegion(r);
            if (!detView->getVisibleRange().intersects(r)) {
                detView->setCenterPos(r.startPos);
            }
        }else{
            getSequenceContext()->getSequenceSelection()->setSelectedRegions(curRegions);
        }
    }


}

QVector<U2Region> ADVSingleSequenceWidget::getSelectedAnnotationRegions(int max) {
    ADVSequenceObjectContext* seqCtx = getSequenceContext();
    const QList<AnnotationSelectionData> selection = seqCtx->getAnnotatedDNAView()->getAnnotationsSelection()->getSelection();
    const QSet<AnnotationTableObject *> myAnns = seqCtx->getAnnotationObjects(true);

    QVector<U2Region> res;
    foreach(const AnnotationSelectionData& sd, selection) {
        AnnotationTableObject *aObj = sd.annotation.getGObject();
        if (myAnns.contains(aObj)) {
            res << sd.getSelectedRegions();
            if (max > 0 && res.size() >= max) {
                break;
            }
        }
    }
    return res;
}

void ADVSingleSequenceWidget::sl_onSelectInRange() {
    QVector<U2Region> selRegs = getSelectedAnnotationRegions(3);
    assert(selRegs.size() == 2);

    const U2Region& r1 = selRegs.at(0);
    const U2Region& r2 = selRegs.at(1);
    assert(!r1.intersects(r2));

    U2Region r;
    r.startPos = qMin(r1.endPos(), r2.endPos());
    r.length = qMax(r1.startPos, r2.startPos) - r.startPos;

    setSelectedRegion(r);
}

void ADVSingleSequenceWidget::sl_onSelectOutRange() {
    QVector<U2Region> selRegs = getSelectedAnnotationRegions(0);
    assert(!selRegs.isEmpty());
    U2Region r = U2Region::containingRegion(selRegs);

    setSelectedRegion(r);
}

void ADVSingleSequenceWidget::setSelectedRegion(const U2Region& region)
{
    getSequenceContext()->getSequenceSelection()->setRegion(region);
    
}

void ADVSingleSequenceWidget::sl_zoomToRange() {
    QDialog dlg(this);
    dlg.setModal(true);
    dlg.setWindowTitle(tr("Zoom to range"));
    DNASequenceSelection* sel = getSequenceSelection();
    int start=getVisibleRange().startPos + 1, end=getVisibleRange().endPos();
    if (!sel->isEmpty()) {
        const QVector<U2Region>& regions = sel->getSelectedRegions();
        start=regions.first().startPos+1;
        end=regions.first().endPos();
    }
    
    RangeSelector* rs = new RangeSelector(&dlg, start, end, getSequenceLength(), true);

    int rc = dlg.exec();
    if (rc == QDialog::Accepted) {
        U2Region r(rs->getStart() - 1, rs->getEnd() - rs->getStart() + 1);
        panView->setVisibleRange(r);
        detView->setStartPos(r.startPos);
    }
    delete rs;
}

#define SPLITTER_STATE_MAP_NAME "ADVSI_MAP"
#define PAN_REG_NAME            "PAN_REG"
#define DET_POS_NAME            "DET_POS"
#define OVERVIEW_VISIBLE        "OVERVIEW_VISIBLE"
#define PAN_VISIBLE             "PAN_VISIBLE"
#define DET_VISIBLE             "DET_VISIBLE"
#define MAIN_RULER_VISIBLE      "MAINR_VISIBLE"
#define CUSTOM_RULERS_VISIBLE   "CUSTOMR_VISIBLE"
#define CUSTOM_R_NAMES          "CUSTOMR_NAMES"
#define CUSTOM_R_COLORS         "CUSTOMR_COLORS"
#define CUSTOM_R_OFFSETS        "CUSTOMR_OFFSETS"
#define SEQUENCE_GRAPH_NAME     "GRAPH_NAME"
#define GRAPH_LABELS_POSITIONS  "LABELS_POSITIONS"

void ADVSingleSequenceWidget::updateState(const QVariantMap& m) {
    QVariantMap map = m.value(SPLITTER_STATE_MAP_NAME).toMap();
    QString sequenceInProjectId = getActiveSequenceContext()->getSequenceObject()->getGHints()->get(GObjectHint_InProjectId).toString();
    QVariantMap myData = map.value(sequenceInProjectId).toMap();
    U2Region panReg = myData.value(PAN_REG_NAME).value<U2Region>();
    int detPos = myData.value(DET_POS_NAME).toInt();
    
    U2Region seqRange(0, getActiveSequenceContext()->getSequenceLength());
    if (seqRange.contains(detPos)) {
        detView->setStartPos(detPos);
    }
    if (!panReg.isEmpty() && seqRange.contains(panReg)) {
        panView->setVisibleRange(panReg);
    }
    
    bool overIsVisible = myData.value(OVERVIEW_VISIBLE, true).toBool();
    setOverviewCollapsed(!overIsVisible);

    bool panIsVisible = myData.value(PAN_VISIBLE, true).toBool();
    setPanViewCollapsed(!panIsVisible);

    bool detIsVisible = myData.value(DET_VISIBLE, true).toBool();
    setDetViewCollapsed(!detIsVisible);

    bool mainRulerVisible = myData.value(MAIN_RULER_VISIBLE, true).toBool();
    panView->getToggleMainRulerAction()->setChecked(mainRulerVisible);

    bool customRulersVisible = myData.value(CUSTOM_RULERS_VISIBLE, true).toBool();
    panView->getToggleCustomRulersAction()->setChecked(customRulersVisible);

    QStringList rnames = myData[CUSTOM_R_NAMES].toStringList();
    QList<QVariant> rcolors = myData[CUSTOM_R_COLORS].toList();
    QList<QVariant> roffsets = myData[CUSTOM_R_OFFSETS].toList();
    if (rnames.count() == rcolors.count() && rnames.count() == roffsets.count()) {
        panView->removeAllCustomRulers();
        for (int i=0; i< rnames.count(); i++) {
            QString name = rnames[i];
            int offset = roffsets[i].toInt();
            QColor color = rcolors[i].value<QColor>();
            panView->addCustomRuler(RulerInfo(name, offset, color));
        }
    }
    QStringList graphNames = myData[SEQUENCE_GRAPH_NAME].toStringList();

    emit si_updateGraphView(graphNames, myData);
}

void ADVSingleSequenceWidget::saveState(QVariantMap& m) {
    QVariantMap map = m.value(SPLITTER_STATE_MAP_NAME).toMap();
    
    QVariantMap myData;
    myData[PAN_REG_NAME] = QVariant::fromValue<U2Region>(panView->getVisibleRange());
    myData[DET_POS_NAME] = QVariant::fromValue<int>(detView->getVisibleRange().startPos);
    myData[OVERVIEW_VISIBLE] = !isOverviewCollapsed();
    myData[PAN_VISIBLE] = !isPanViewCollapsed();
    myData[DET_VISIBLE] = !isDetViewCollapsed();
    myData[MAIN_RULER_VISIBLE] = panView->getToggleMainRulerAction()->isChecked();
    myData[CUSTOM_RULERS_VISIBLE] = panView->getToggleCustomRulersAction()->isChecked();
    
    QStringList rnames;
    QList<QVariant> roffsets;
    QList<QVariant> rcolors;
    foreach(const RulerInfo& ri, panView->getCustomRulers()) {
        rnames.append(ri.name);
        roffsets.append(ri.offset);
        rcolors.append(ri.color);
    }
    myData[CUSTOM_R_NAMES] = rnames;
    myData[CUSTOM_R_OFFSETS] = roffsets;
    myData[CUSTOM_R_COLORS] = rcolors;

    QStringList graphNames;
    QList<QVariant> positions;
    foreach(GSequenceLineView *view, lineViews) {
        QList<QVariant> positions;
        GSequenceGraphView *graphView = dynamic_cast<GSequenceGraphView *>(view);
        if(NULL != graphView) {
            graphNames.append(graphView->getGraphViewName());
            graphView->getLabelPositions(positions);
            myData[graphView->getGraphViewName()] = positions;
        }
    }
    myData[SEQUENCE_GRAPH_NAME] = graphNames;
    myData[GRAPH_LABELS_POSITIONS] = positions;

    
    QString sequenceInProjectId = getActiveSequenceContext()->getSequenceObject()->getGHints()->get(GObjectHint_InProjectId).toString();
    map[sequenceInProjectId] = myData;
    m[SPLITTER_STATE_MAP_NAME] = map;

}

// QT 4.5.0 bug workaround
void ADVSingleSequenceWidget::sl_closeView()
{
    closeView();
}

void ADVSingleSequenceWidget::sl_saveScreenshot() {
    if (linesLayout->count() < 2) {
        return;
    }
    QRect screenRect = rect();
    screenRect.setTopLeft(linesLayout->itemAt(1)->geometry().topLeft());
    ExportImageDialog dialog(this, screenRect);
    dialog.exec();

}

void ADVSingleSequenceWidget::closeView() {
    U2SequenceObject* dnaObj = getSequenceObject();
    AnnotatedDNAView* v = getAnnotatedDNAView();
    v->removeObject(dnaObj);
}

void ADVSingleSequenceWidget::sl_createCustomRuler() {
    QSet<QString> namesToFilter;
    foreach(const RulerInfo& ri, panView->getCustomRulers()) {
        namesToFilter.insert(ri.name);
    }
    
    int offset = panView->getVisibleRange().center();
    
    AnnotationSelection * annSelection = getDetGSLView()->getSequenceContext()->getAnnotationsSelection();
    U2SequenceObject * seqObj = getSequenceObject();
    int annOffset = INT_MAX;
    foreach(const AnnotationSelectionData & selectionData, annSelection->getSelection()) {
        const Annotation &ann = selectionData.annotation;
        AnnotationTableObject *annObj = ann.getGObject();
        if( !annObj->hasObjectRelation(seqObj, GObjectRelationRole::SEQUENCE) ) {
            continue;
        }
        
        // find minimum of start positions of selected annotations
        foreach( const U2Region & region, selectionData.getSelectedRegions() ) {
            annOffset = annOffset > region.startPos ? region.startPos : annOffset;
        }
    }
    if( annOffset != INT_MAX ) {
        offset = annOffset;
    }
    
    QVector<U2Region> selection = getSequenceSelection()->getSelectedRegions();
    if(!selection.isEmpty()) {
        offset = selection.first().startPos;
    }
    
    CreateRulerDialogController d(namesToFilter, U2Region(0, getSequenceObject()->getSequenceLength()), offset);
    int rc = d.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    RulerInfo ri(d.name, d.offset, d.color);
    panView->addCustomRuler(ri);
}

void ADVSingleSequenceWidget::sl_removeCustomRuler() {
    QString rulerName = qobject_cast<QAction*>(sender())->data().toString();
    panView->removeCustomRuler(rulerName);
}

void ADVSingleSequenceWidget::sl_onAnnotationSelectionChanged(AnnotationSelection* s, const QList<Annotation*>&, const QList<Annotation*>&) {
    // make sequence selection to match external annotation bounds
    QVector<U2Region> annotatedRegions = s->getSelectedLocations(getSequenceContext()->getAnnotationObjects(true));
    if (!annotatedRegions.isEmpty()) {
        QVector<U2Region> joinedRegions = U2Region::join(annotatedRegions);
        getSequenceContext()->getSequenceSelection()->setSelectedRegions(joinedRegions);
    }
    updateSelectionActions();
}

void ADVSingleSequenceWidget::updateSelectionActions() {
    QVector<U2Region> selRegs = getSelectedAnnotationRegions(3);

    selectInAnnotationRangeAction->setEnabled(selRegs.size() == 2 && !selRegs[0].intersects(selRegs[1]));
    selectOutAnnotationRangeAction->setEnabled(!selRegs.isEmpty());
}

void ADVSingleSequenceWidget::addStateActions( QMenu& m ) {
    toggleViewAction->setText(isViewCollapsed() ? tr("Show all views") : tr("Hide all views"));
    toggleViewAction->setObjectName("show_hide_all_views");
    togglePanViewAction->setText(isPanViewCollapsed() ? tr("Show zoom view") : tr("Hide zoom view"));
    togglePanViewAction->setObjectName("show_hide_zoom_view");
    toggleDetViewAction->setText(isDetViewCollapsed() ? tr("Show details view") : tr("Hide details view"));
    toggleDetViewAction->setObjectName("show_hide_details_view");
    toggleOverviewAction->setText(isOverviewCollapsed() ? tr("Show overview") : tr("Hide overview"));
    toggleOverviewAction->setObjectName("show_hide_overview");
    m.addAction(toggleViewAction);
    m.addAction(toggleDetViewAction);
    m.addAction(togglePanViewAction);
    m.addAction(toggleOverviewAction);
}

void ADVSingleSequenceWidget::sl_toggleView(){
    bool newStateCollapsed = !isViewCollapsed();
    setViewCollapsed(newStateCollapsed);
}

void ADVSingleSequenceWidget::sl_togglePanView(){
    setPanViewCollapsed(!isPanViewCollapsed());
}

void ADVSingleSequenceWidget::onSequenceObjectRenamed(const QString&) {
    headerWidget->updateTitle();
}

void ADVSingleSequenceWidget::sl_showStateMenu() {
    QPointer<QToolButton> widgetStateMenuButtonPtr(widgetStateMenuButton);

    QMenu m;
    addStateActions(m);
    m.addAction(closeViewAction);
    m.exec(QCursor::pos());

    if (!widgetStateMenuButtonPtr.isNull()) { //if not self closed
        widgetStateMenuButtonPtr->setDown(false);
    }
}

//////////////////////////////////////////////////////////////////////////
// header

ADVSingleSequenceHeaderWidget::ADVSingleSequenceHeaderWidget(ADVSingleSequenceWidget* p) : QWidget(p), ctx(p) {
    setFixedHeight(ADV_HEADER_HEIGHT);
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);

    connect(ctx->getAnnotatedDNAView(), SIGNAL(si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*)), 
                                        SLOT(sl_advFocusChanged(ADVSequenceWidget*, ADVSequenceWidget*)));

    //TODO: track focus events (mouse clicks) on toolbar in disabled state and on disabled buttons !!!

    QHBoxLayout* l = new QHBoxLayout();
    l->setSpacing(4);
    l->setContentsMargins(5, 1, 0, 2);

    U2SequenceObject* seqObj = ctx->getSequenceObject();
    QString objName = seqObj->getGObjectName();
    pixLabel= new QLabel(this);
    QFont f = pixLabel->font();
    if (f.pixelSize() > ADV_HEADER_HEIGHT) {
        f.setPixelSize(ADV_HEADER_HEIGHT-8);
    }
    QIcon objIcon(":/core/images/gobject.png");
    QPixmap pix = objIcon.pixmap(QSize(16, 16), QIcon::Active);
    pixLabel->setPixmap(pix);
    pixLabel->setFont(f);
    QString objInfoTip = "<i>" + objName  + "</i>"
        + "<br>" + tr("Alphabet: <b>%1</b>").arg(seqObj->getAlphabet()->getName()) 
        + "<br>" + tr(" Sequence size: <b>%1</b>").arg(seqObj->getSequenceLength())
        + "<br>" + tr(" File:&nbsp;<b>%1</b>").arg(seqObj->getDocument()->getURLString());
    pixLabel->setToolTip(objInfoTip);
    pixLabel->installEventFilter(this);
    
    int labelWidth = 50;
    QFontMetrics fm(f);
    nameLabel = new QLabel("", this);
    updateTitle();
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    nameLabel->setMinimumWidth(labelWidth);
    nameLabel->setMaximumWidth(fm.width(nameLabel->text()));
    nameLabel->setFont(f);
    nameLabel->setToolTip(objInfoTip);
    nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    toolBar = new HBar(this);
    toolBar->setObjectName("tool_bar_" + ctx->getSequenceObject()->getGObjectName());
    toolBar->layout()->setSpacing(0);
    toolBar->layout()->setMargin(0);
    toolBar->setFixedHeight(ADV_HEADER_HEIGHT);
   
    setLayout(l);

    l->addWidget(pixLabel);
    l->addWidget(nameLabel);
    l->addStretch();
    l->addWidget(toolBar);


    connect(toolBar, SIGNAL(actionTriggered(QAction*)), SLOT(sl_actionTriggered(QAction*)));

    updateActiveState();

}

void ADVSingleSequenceHeaderWidget::updateTitle() {
    U2SequenceObject* seqObj = ctx->getSequenceObject();
    QString newTitle = seqObj->getGObjectName() + " [" + getShortAlphabetName(seqObj->getAlphabet()) +"]";
    setTitle(newTitle);
}

bool ADVSingleSequenceHeaderWidget::eventFilter(QObject *o, QEvent *e) {
    if (o == pixLabel && e->type() == QEvent::MouseButtonPress) {
        ctx->sl_showStateMenu();
        return true;
    }
    return false;
}

void ADVSingleSequenceHeaderWidget::sl_actionTriggered(QAction* a){
    Q_UNUSED(a);
    ctx->getAnnotatedDNAView()->setFocusedSequenceWidget(ctx);
}

void ADVSingleSequenceHeaderWidget::sl_advFocusChanged(ADVSequenceWidget* prevFocus, ADVSequenceWidget* newFocus) {
    if (prevFocus == ctx || newFocus == ctx) {
        update();
        updateActiveState();
    } 
}
void ADVSingleSequenceHeaderWidget::updateActiveState() {
    bool focused = ctx->getAnnotatedDNAView()->getSequenceWidgetInFocus() == ctx;
    nameLabel->setEnabled(focused);
    pixLabel->setEnabled(focused);
    ctx->getSelectRangeAction()->setShortcutContext(focused ? Qt::WindowShortcut : Qt::WidgetShortcut);
    //toolBar->setEnabled(focused); TODO: click on disabled buttons does not switch focus!
}

void ADVSingleSequenceHeaderWidget::mouseDoubleClickEvent(QMouseEvent *e) {
    ctx->toggleViewAction->trigger();
    QWidget::mouseDoubleClickEvent(e);
}

void ADVSingleSequenceHeaderWidget::paintEvent(QPaintEvent *e) {
    QWidget::paintEvent(e);

    QPainter p(this);
    p.setPen(QApplication::palette().color(QPalette::Dark));
    p.drawLine(0, height()-1, width(), height()-1);
}

QString ADVSingleSequenceHeaderWidget::getShortAlphabetName(const DNAAlphabet* al) {
    DNAAlphabetType type = al->getType();
    if (type == DNAAlphabet_RAW) {
        return tr("raw");
    }
    if (type == DNAAlphabet_AMINO) {
        return tr("amino");
    }
    assert(type == DNAAlphabet_NUCL);
    QString id = al->getId();
    if (id == BaseDNAAlphabetIds::NUCL_DNA_DEFAULT()) {
        return tr("dna");
    } else if (id == BaseDNAAlphabetIds::NUCL_DNA_EXTENDED()) {
        return tr("dna ext");
    } else if (id == BaseDNAAlphabetIds::NUCL_RNA_DEFAULT()) {
        return tr("rna");
    } else if (id == BaseDNAAlphabetIds::NUCL_RNA_EXTENDED()) {
        return tr("rna ext");
    }
    return "?";
}

}//namespace


