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

#include <U2Misc/RangeSelector.h>
#include <U2Misc/PositionSelector.h>
#include <U2Gui/GUIUtils.h>
#include <U2Misc/DialogUtils.h>
#include <U2Misc/HBar.h>

#include <QtGui/QApplication>
#include <QtGui/QToolButton>
#include <QtGui/QWidgetAction>
#include <QtGui/QDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

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
    connect(selectRangeAction2, SIGNAL(triggered()), SLOT(sl_onSelectRange()));

    selectInAnnotationRangeAction= new QAction(tr("Sequence between selected annotations"), this);
    connect(selectInAnnotationRangeAction, SIGNAL(triggered()), SLOT(sl_onSelectInRange()));
    
    selectOutAnnotationRangeAction= new QAction(tr("Sequence around selected annotations"), this);
    connect(selectOutAnnotationRangeAction, SIGNAL(triggered()), SLOT(sl_onSelectOutRange()));

    zoomToRangeAction = new QAction(QIcon(":/core/images/zoom_reg.png"), tr("Zoom to range.."), this);
    connect(zoomToRangeAction, SIGNAL(triggered()), SLOT(sl_zoomToRange()));

    createNewRulerAction = new QAction(tr("Create new ruler..."), this);
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
    detView->setObjectName("det_view");
    addSequenceView(detView);

    panView = new PanView(this, seqCtx);
    panView->setObjectName("pan_view");
    connect(panView, SIGNAL(si_centerPosition(int)), SLOT(sl_onLocalCenteringRequest(int)));

    zoomUseObject.setPanView(panView);

    addSequenceView(panView, headerWidget);

    panView->setFrameView(detView);

    overview = new Overview(this, seqCtx);
    overview->setObjectName("overview");
    overview->setMouseTracking(true);
    addSequenceView(overview, headerWidget);
    
    setFixedHeight(linesLayout->minimumSize().height());

    QToolBar* hBar = headerWidget->getToolBar();
    
    hBar->addAction(selectRangeAction1);
    hBar->addSeparator();

    if (seqCtx->getComplementTT() != NULL) {
        QAction* showComplementAction = detView->getShowComplementAction();
        hBar->addAction(showComplementAction);
    }
    if (seqCtx->getAminoTT() != NULL) {
        QAction* showTranslationAction = detView->getShowTranslationAction();
        hBar->addAction(showTranslationAction);
    }

    if (seqCtx->getAminoTT() != NULL) {        
        ttButton = new QToolButton(hBar);
        QMenu* ttMenu = seqCtx->createTranslationsMenu();
        ttButton->setDefaultAction(ttMenu->menuAction());
        ttButton->setPopupMode(QToolButton::InstantPopup);
        hBar->addWidget(ttButton);
        tbMenues.append(ttMenu);
        hBar->addSeparator();
    } else {
        ttButton = NULL;
    }

    QAction* shotScreenAction = new QAction(QIcon(":/core/images/cam2.png"), tr("Capture screen"), this);
    shotScreenAction->setObjectName("capture_screen");
    connect(shotScreenAction, SIGNAL(triggered()), this, SLOT(sl_saveScreenshot()));
    hBar->addAction(shotScreenAction);

    hBar->addAction(panView->getZoomInAction());
    hBar->addAction(panView->getZoomOutAction());
    //hBar->addAction(panView->getZoomToSelectionAction());
    hBar->addAction(zoomToRangeAction);
    hBar->addAction(panView->getZoomToSequenceAction());

    updateSelectionActions();

#define MIN_SEQUENCE_LEN_TO_USE_FULL_MODE 100
    if (seqCtx->getSequenceLen() < MIN_SEQUENCE_LEN_TO_USE_FULL_MODE) {
        //sequence is rather small -> show panview only by default
        setOverviewCollapsed(true);
        setDetViewCollapsed(true);
    }
}


ADVSingleSequenceWidget::~ADVSingleSequenceWidget() {
    foreach(QMenu* m, tbMenues) {
        delete m;
    }
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
    foreach(GSequenceLineView* seqView, lineViews) {
        seqView->setHidden(v);
    }
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
    detView->setHidden(v);
    if (ttButton != NULL) {
        ttButton->setDisabled(v);
    }
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

void ADVSingleSequenceWidget::setNumBasesVisible(int n) {
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

int ADVSingleSequenceWidget::getSequenceLen() const {
    return getSequenceContext()->getSequenceLen();
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

DNASequenceObject* ADVSingleSequenceWidget::getSequenceObject() const {
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
    return false;
}

void ADVSingleSequenceWidget::sl_onLocalCenteringRequest(int pos) {
    detView->setCenterPos(pos);
}

void ADVSingleSequenceWidget::addADVSequenceWidgetAction(ADVSequenceWidgetAction* a) {
    ADVSequenceWidget::addADVSequenceWidgetAction(a);
    if (a->addToBar) {
        QToolBar* tb = headerWidget->getToolBar();
        if (a->menu()!=NULL) {
            QToolButton* tt = new QToolButton(tb);
            tt->setDefaultAction(a);
            tt->setPopupMode(QToolButton::InstantPopup);
            tb->insertWidget(tb->actions().first(), tt);
        } else {
            tb->insertAction(tb->actions().first(), a);
        }
        
    }
}

void ADVSingleSequenceWidget::sl_onSelectRange() {
    QDialog dlg(this);
    dlg.setModal(true);
    dlg.setWindowTitle(tr("Select range"));
    ADVSequenceObjectContext* ctx = getSequenceContext();
    RangeSelector rs(&dlg, 1, ctx->getSequenceLen(), ctx->getSequenceLen(), true);
    int rc = dlg.exec();
    if (rc == QDialog::Accepted) {
        U2Region r(rs.getStart() - 1, rs.getEnd() - rs.getStart() + 1);
        ctx->getSequenceSelection()->clear();
        getSequenceSelection()->addRegion(r);
        if (!detView->getVisibleRange().intersects(r)) {
            detView->setCenterPos(r.startPos);
        }
    }
}

QVector<U2Region> ADVSingleSequenceWidget::getSelectedAnnotationRegions(int max) {
    ADVSequenceObjectContext* seqCtx = getSequenceContext();
    const QList<AnnotationSelectionData> selection = seqCtx->getAnnotatedDNAView()->getAnnotationsSelection()->getSelection();
    const QSet<AnnotationTableObject*> myAnns = seqCtx->getAnnotationObjects(true);

    QVector<U2Region> res;
    foreach(const AnnotationSelectionData& sd, selection) {
        AnnotationTableObject* aObj = sd.annotation->getGObject();
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
    
    getSequenceContext()->getSequenceSelection()->clear();
    getSequenceContext()->getSequenceSelection()->addRegion(r);
}

void ADVSingleSequenceWidget::sl_onSelectOutRange() {
    QVector<U2Region> selRegs = getSelectedAnnotationRegions(0);
    assert(!selRegs.isEmpty());
    U2Region r = U2Region::containingRegion(selRegs);

    getSequenceContext()->getSequenceSelection()->clear();
    getSequenceContext()->getSequenceSelection()->addRegion(r);
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
    
    RangeSelector* rs = new RangeSelector(&dlg, start, end, getSequenceLen(), true);

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

void ADVSingleSequenceWidget::updateState(const QVariantMap& m) {
    QVariantMap map = m.value(SPLITTER_STATE_MAP_NAME).toMap();
    QVariantMap myData = map.value(getActiveSequenceContext()->getSequenceObject()->getGObjectName()).toMap();
    U2Region panReg = myData.value(PAN_REG_NAME).value<U2Region>();
    int detPos = myData.value(DET_POS_NAME).toInt();
    
    U2Region seqRange(0, getActiveSequenceContext()->getSequenceLen());
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
    
    map[getActiveSequenceContext()->getSequenceObject()->getGObjectName()] = myData;
    m[SPLITTER_STATE_MAP_NAME] = map;
}

// QT 4.5.0 bug workaround
void ADVSingleSequenceWidget::sl_closeView()
{
    closeView();
}
void ADVSingleSequenceWidget::sl_saveScreenshot()
{
    LastOpenDirHelper lod(IMAGE_DIR);
    QRect* screenRect = new QRect(linesLayout->itemAt(1)->geometry().topLeft().x(),
                                  linesLayout->itemAt(1)->geometry().topLeft().y(),
                                  this->geometry().bottomRight().x(),
                                  this->geometry().bottomRight().y());
    QPixmap curPixMap = QPixmap::grabWidget(this, *screenRect);
    QMap<QString, QString> filters;
    filters[ "PNG - Portable Network Graphics (*.png)" ] = "png";
    filters[ "JPG/JPEG format (*.jpg)" ] = "jpg";
    filters[ "TIF - Tagged Image File format (*.tiff)" ] = "tiff";

    QString initialPath = lod.dir + "/untitled";
    QString selectedFormat;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
        initialPath, QStringList(filters.keys()).join(";;"), &selectedFormat);
    lod.url = fileName;
    
    if (!fileName.isEmpty()) {
        bool formatAppend = true;
        foreach( const QString & format, filters.values() ) {
            if( fileName.endsWith("." + format) ){
                formatAppend = false;
                break;
            }
        }
        if( formatAppend ) {
            assert( filters.contains(selectedFormat) );
            fileName.append( "." + filters[selectedFormat] );
            
            if( QFile::exists(fileName) ) {
                QMessageBox::StandardButtons b = 
                        QMessageBox::warning( 
                                0, tr("Replace file"), 
                                tr("%1 already exists.\nDo you want to replace it?").arg(fileName),
                                QMessageBox::Yes|QMessageBox::No);
                if( QMessageBox::Yes != b ) {
                    return;
                }
            }
        }
        bool result = curPixMap.save(fileName);
        if (!result) {
            QMessageBox::critical(this, L10N::errorTitle(), L10N::errorImageSave(fileName, selectedFormat));
            return;
        }
    }
}

void ADVSingleSequenceWidget::closeView() {
    DNASequenceObject* dnaObj = getSequenceObject();
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
    DNASequenceObject * seqObj = getSequenceObject();
    int annOffset = INT_MAX;
    foreach(const AnnotationSelectionData & selectionData, annSelection->getSelection()) {
        Annotation * ann = selectionData.annotation;
        AnnotationTableObject * annObj = ann->getGObject();
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
    
    CreateRulerDialogController d(namesToFilter, getSequenceObject()->getSequenceRange(), offset);
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
    togglePanViewAction->setText(isPanViewCollapsed() ? tr("Show zoom view") : tr("Hide zoom view"));
    toggleDetViewAction->setText(isDetViewCollapsed() ? tr("Show details view") : tr("Hide details view"));
    toggleOverviewAction->setText(isOverviewCollapsed() ? tr("Show overview") : tr("Hide overview"));
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

void ADVSingleSequenceWidget::setTitle(const QString& title) {
    headerWidget->setTitle(title);
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

    DNASequenceObject* seqObj = ctx->getSequenceObject();
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
        + "<br>" + tr(" Sequence size: <b>%1</b>").arg(seqObj->getSequenceLen())
        + "<br>" + tr(" File:&nbsp;<b>%1</b>").arg(seqObj->getDocument()->getURLString());
    pixLabel->setToolTip(objInfoTip);
    pixLabel->installEventFilter(this);
    
    int labelWidth = 50;
    QFontMetrics fm(f);
    QString nameText = objName + " [" + getShortAlphabetName(seqObj->getAlphabet()) +"]";
    nameLabel = new QLabel(nameText, this);
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    nameLabel->setMinimumWidth(labelWidth);
    nameLabel->setMaximumWidth(fm.width(nameText));
    nameLabel->setFont(f);
    nameLabel->setToolTip(objInfoTip);
    nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    toolBar = new HBar(this);
    toolBar->layout()->setSpacing(0);
    toolBar->layout()->setMargin(0);

    /// close bar
    closeBar = new HBar(this);
    closeBar->layout()->setSpacing(0);
    closeBar->layout()->setMargin(0);

        
    setLayout(l);

    l->addWidget(pixLabel);
    l->addWidget(nameLabel);
    l->addStretch();
    l->addWidget(toolBar);
    l->addWidget(closeBar);


    connect(toolBar, SIGNAL(actionTriggered(QAction*)), SLOT(sl_actionTriggered(QAction*)));
    connect(closeBar, SIGNAL(actionTriggered(QAction*)), SLOT(sl_actionTriggered(QAction*)));

    populateToolBars();
    updateActiveState();

}

void ADVSingleSequenceHeaderWidget::populateToolBars() {
    // close bar
    widgetStateMenuButton = new QToolButton(this);
    widgetStateMenuButton->setIcon(QIcon(":core/images/adv_widget_menu.png"));
    widgetStateMenuButton->setFixedWidth(20);
    widgetStateMenuButton->setToolTip(tr("Toggle view"));
    connect(widgetStateMenuButton, SIGNAL(pressed()), SLOT(sl_showStateMenu()));


    closeViewAction = new QAction(tr("Remove sequence"), ctx);
    connect(closeViewAction, SIGNAL(triggered()), SLOT(sl_closeView()));

    closeBar->addWidget(widgetStateMenuButton);
}

bool ADVSingleSequenceHeaderWidget::eventFilter(QObject *o, QEvent *e) {
    if (o == pixLabel && e->type() == QEvent::MouseButtonPress) {
        sl_showStateMenu();
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

void ADVSingleSequenceHeaderWidget::sl_showStateMenu() {
    QPointer<QToolButton> widgetStateMenuButtonPtr(widgetStateMenuButton);

    QMenu m;
    ctx->addStateActions(m);
    m.addAction(closeViewAction);
    m.exec(QCursor::pos());

    if (!widgetStateMenuButtonPtr.isNull()) { //if not self closed
        widgetStateMenuButtonPtr->setDown(false);
    }
}


void ADVSingleSequenceHeaderWidget::sl_closeView() {
#ifdef Q_OS_LINUX
    if(QString("4.5.0") == qVersion())
    {
        QTimer::singleShot(0,ctx,SLOT(sl_closeView()));
    }
    else
    {
#endif // Q_OS_LINUX
        ctx->closeView();
#ifdef Q_OS_LINUX
    }
#endif // Q_OS_LINUX
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

QString ADVSingleSequenceHeaderWidget::getShortAlphabetName(DNAAlphabet* al) {
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
        return tr("dna ext.");
    } else if (id == BaseDNAAlphabetIds::NUCL_RNA_DEFAULT()) {
        return tr("rna");
    } else if (id == BaseDNAAlphabetIds::NUCL_RNA_EXTENDED()) {
        return tr("rna ext.");
    }
    return "?";
}



}//namespace

