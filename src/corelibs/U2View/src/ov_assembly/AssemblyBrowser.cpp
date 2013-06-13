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

#include "AssemblyBrowser.h"

#include "AssemblyAnnotationsArea.h"
#include "AssemblyBrowserFactory.h"
#include "AssemblyBrowserState.h"
#include "AssemblyBrowserTasks.h"
#include "ZoomableAssemblyOverview.h"
#include "AssemblyReferenceArea.h"
#include "AssemblyConsensusArea.h"
#include "AssemblyCoverageGraph.h"
#include "AssemblyRuler.h"
#include "AssemblyReadsArea.h"
#include "AssemblyBrowserSettings.h"
#include "AssemblyCellRenderer.h"

#include <U2Core/U2Type.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/FormatUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/VariantTrackObject.h>

#include <QtGui/QVBoxLayout>
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QScrollBar>
#include <QtGui/QToolButton>
#include <QtCore/QEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#include <U2Core/AppContext.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/Log.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/Timer.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2CrossDatabaseReferenceDbi.h>

#include <U2Formats/ConvertAssemblyToSamTask.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/ExportImageDialog.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/OptionsPanel.h>
#include <U2Gui/OPWidgetFactoryRegistry.h>
#include <U2Gui/PositionSelector.h>

#include <U2View/ConvertAssemblyToSamDialog.h>

#include <memory>

namespace U2 {

//==============================================================================
// AssemblyBrowser
//==============================================================================
const double AssemblyBrowser::ZOOM_MULT = 1.25;
const double AssemblyBrowser::INITIAL_ZOOM_FACTOR= 1.;

AssemblyBrowser::AssemblyBrowser(QString viewName, AssemblyObject * o) :
GObjectView(AssemblyBrowserFactory::ID, viewName), ui(0),
gobject(o), model(0), zoomFactor(INITIAL_ZOOM_FACTOR), xOffsetInAssembly(0), yOffsetInAssembly(0), coverageReady(false),
cellRendererRegistry(new AssemblyCellRendererFactoryRegistry(this)),
zoomInAction(0), zoomOutAction(0), posSelectorAction(0), posSelector(0), showCoordsOnRulerAction(0), saveScreenShotAction(0),
showInfoAction(0), exportToSamAction(0)
{
    GCOUNTER( cvar, tvar, "AssemblyBrowser" );
    initFont();
    setupActions();

    if(gobject) {
        objects.append(o);
        requiredObjects.append(o);
        const U2EntityRef& ref= gobject->getEntityRef();
        model = QSharedPointer<AssemblyModel>(new AssemblyModel(DbiConnection(ref.dbiRef, dbiOpStatus)));
        sl_assemblyLoaded();
    }   
    onObjectAdded(gobject);
}

bool AssemblyBrowser::checkValid(U2OpStatus &os) {
    // before opening view, check for incorrect reference length attribute
    qint64 modelLen = model->getModelLength(os);
    CHECK_OP(os, false);
    if(modelLen == 0 && model->hasReads(os)) {
        os.setError(tr("Failed to open assembly browser for %1, assembly %2: model length should be > 0")
                    .arg(gobject->getDocument()->getURLString())
                    .arg(gobject->getGObjectName())) ;
        return false;
    }
    return true;
}

QWidget * AssemblyBrowser::createWidget() {
    optionsPanel = new OptionsPanel(this);
    ui = new AssemblyBrowserUi(this);
    U2OpStatusImpl os;
    if(model->hasReads(os)) {
        updateOverviewTypeActions();
        showCoordsOnRulerAction->setChecked(ui->getRuler()->getShowCoordsOnRuler());
        showCoverageOnRulerAction->setChecked(ui->getRuler()->getShowCoverageOnRuler());
        readHintEnabledAction->setChecked(ui->getReadsArea()->isReadHintEnabled());
        ui->installEventFilter(this);
        ui->setAcceptDrops(true);
    }
    return ui;
}

QVariantMap AssemblyBrowser::saveState() {
    return AssemblyBrowserState::buildStateMap(this);
}

Task * AssemblyBrowser::updateViewTask(const QString &stateName, const QVariantMap &stateData) {
    return new UpdateAssemblyBrowserTask(this, stateName, stateData);
}

OptionsPanel * AssemblyBrowser::getOptionsPanel() {
    return optionsPanel;
}

bool AssemblyBrowser::eventFilter(QObject* o, QEvent* e) {
    if(o == ui) {
        if (e->type() == QEvent::DragEnter || e->type() == QEvent::Drop) {
            QDropEvent* de = (QDropEvent*)e;
            const QMimeData* md = de->mimeData();
            const GObjectMimeData* gomd = qobject_cast<const GObjectMimeData*>(md);
            if (gomd != NULL) {
                if (e->type() == QEvent::DragEnter) {
                    de->acceptProposedAction();
                } else {
                    QString err = tryAddObject(gomd->objPtr.data());
                    if(!err.isEmpty()) {
                        QMessageBox::critical(ui, tr("Error!"), err);
                    }
                }
            }
        } 
    }
    return false;
}

QString AssemblyBrowser::tryAddObject(GObject * obj) {
    Document * objDoc = obj->getDocument();
    SAFE_POINT(NULL != objDoc, "", tr("Internal error: only object with document can be added to browser"));

    if (GObjectTypes::SEQUENCE == obj->getGObjectType()) {
        U2SequenceObject * seqObj = qobject_cast<U2SequenceObject*>(obj);
        CHECK(NULL != seqObj, tr("Internal error: broken sequence object"));
        SAFE_POINT(NULL != objDoc->getDocumentFormat(), "", tr("Internal error: empty document format"));
        
        U2OpStatus2Log os;
        qint64 seqLen = seqObj->getSequenceLength();    
        QStringList errs;
        qint64 modelLen = model->getModelLength(os);
        if (seqLen != modelLen) {
            errs << tr("- Reference sequence is %1 than assembly").arg(seqLen < modelLen ? tr("lesser") : tr("bigger"));
        }
        if (seqObj->getGObjectName() != gobject->getGObjectName()) {
            errs << tr("- Reference and assembly names not match");
        }
        
        // commented: waiting for fix
        //QByteArray refMd5 = model->getReferenceMd5();
        //if(!refMd5.isEmpty()) {
        //    //QByteArray data = QString(seqObj->getSequence()).remove("-").toUpper().toUtf8();
        //    QByteArray data = QString(seqObj->getSequence()).toUpper().toUtf8();
        //    QByteArray seqObjMd5 = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
        //    if(seqObjMd5 != refMd5) {
        //        errs << tr("- Reference MD5 not match with MD5 written in assembly");
        //    }
        //}
        
        bool setRef = true;
        if(!errs.isEmpty()) {
            errs << tr("\n  Continue?");
            QMessageBox::StandardButtons fl = QMessageBox::Ok | QMessageBox::Cancel;
            QMessageBox::StandardButton btn = QMessageBox::question(ui, tr("Errors"), errs.join("\n"), fl, QMessageBox::Ok);
            setRef = btn == QMessageBox::Ok;
        }
        if(setRef) {
            model->setReference(seqObj);
            U2CrossDatabaseReferenceDbi * crossDbi = model->getDbiConnection().dbi->getCrossDatabaseReferenceDbi();
            U2CrossDatabaseReference crossDbRef;
            // Cannot simply use seqObj->getSequenceRef(), since it points to a temporary dbi
            // TODO: make similar method seqObj->getPersistentSequenctRef()
            crossDbRef.dataRef.dbiRef.dbiId = objDoc->getURLString();
            crossDbRef.dataRef.dbiRef.dbiFactoryId = "document";
            crossDbRef.dataRef.entityId = seqObj->getGObjectName().toUtf8();
            crossDbRef.dataRef.version = 1;
            crossDbi->createCrossReference(crossDbRef, os);
            LOG_OP(os);
            model->associateWithReference(crossDbRef);
        }
    } else if (GObjectTypes::VARIANT_TRACK == obj->getGObjectType()) {
        VariantTrackObject *trackObj = qobject_cast<VariantTrackObject*>(obj);
        CHECK(NULL != trackObj, tr("Internal error: broken variant track object"));

        model->addTrackObject(trackObj);
    } else {
        return tr("Only sequence or variant track  objects can be added to assembly browser");
    }

    return "";
}

void AssemblyBrowser::buildStaticToolbar(QToolBar* tb) {
    U2OpStatusImpl os;
    if(model->hasReads(os)) {
        tb->addAction(zoomInAction);
        tb->addAction(zoomOutAction);

        U2OpStatusImpl st;
        posSelector = new PositionSelector(tb, 1, model->getModelLength(st));
        if(!st.hasError()) {
            connect(posSelector, SIGNAL(si_positionChanged(int)), SLOT(sl_onPosChangeRequest(int)));
            tb->addSeparator();
            tb->addWidget(posSelector);
            posSelector->getPosEdit()->setMinimumWidth(160); // For big numbers we need bigger text box
        }
        tb->addSeparator();
        updateZoomingActions();

        // commented because do not know if log scale is needed
        /*QToolButton * overviewScaleTypeToolButton = new QToolButton(tb);
        QMenu * scaleTypeMenu = new QMenu(tr("Scale type"), ui);
        foreach(QAction * a, overviewScaleTypeActions) {
            scaleTypeMenu->addAction(a);
        }
        overviewScaleTypeToolButton->setDefaultAction(scaleTypeMenu->menuAction());
        overviewScaleTypeToolButton->setPopupMode(QToolButton::InstantPopup);
        tb->addWidget(overviewScaleTypeToolButton);*/

        tb->addAction(showCoordsOnRulerAction);
        tb->addAction(showCoverageOnRulerAction);
        tb->addAction(readHintEnabledAction);
        tb->addSeparator();

        tb->addAction(saveScreenShotAction);
        tb->addAction(showInfoAction);
//        tb->addAction(exportToSamAction);
    }
    GObjectView::buildStaticToolbar(tb);
}

void AssemblyBrowser::sl_onPosChangeRequest(int pos) {
    setXOffsetInAssembly(normalizeXoffset(pos - 1));
    ui->getReadsArea()->setFocus();
}

void AssemblyBrowser::buildStaticMenu(QMenu* m) {
    U2OpStatusImpl os;
    if(model->hasReads(os)) {
        m->addAction(zoomInAction);
        m->addAction(zoomOutAction);
        m->addAction(saveScreenShotAction);
        m->addAction(showInfoAction);
        m->addAction(exportToSamAction);
    }
    GObjectView::buildStaticMenu(m);
    GUIUtils::disableEmptySubmenus(m);
}

void AssemblyBrowser::setGlobalCoverageInfo(CoverageInfo newInfo) {
    U2OpStatus2Log os;
    U2Region globalRegion(0, model->getModelLength(os));
    SAFE_POINT(newInfo.region == globalRegion, "coverage info is not global",);
    if(newInfo.coverageInfo.size() <= coveredRegionsManager.getSize()) {
        return;
    }
    // prefer model's coverage stat
    if(model->hasCachedCoverageStat()) {
        U2OpStatus2Log status;
        U2AssemblyCoverageStat coverageStat = model->getCoverageStat(status);
        if(!status.isCoR() && coverageStat.coverage.size() > newInfo.coverageInfo.size()) {
            newInfo.coverageInfo = U2AssemblyUtils::coverageStatToVector(coverageStat);
            newInfo.updateStats();
        }
    }
    coveredRegionsManager = CoveredRegionsManager(globalRegion, newInfo.coverageInfo);

    if(newInfo.coverageInfo.size() == newInfo.region.length) {
        setLocalCoverageCache(newInfo);
    }

    coverageReady = true;
    emit si_coverageReady();
}

QList<CoveredRegion> AssemblyBrowser::getCoveredRegions() const {
    if(!coveredRegionsManager.isEmpty()) {
        return coveredRegionsManager.getTopCoveredRegions(10, 1);
    }
    return QList<CoveredRegion>();
}

void AssemblyBrowser::setLocalCoverageCache(CoverageInfo coverage) {
    SAFE_POINT(coverage.region.length == coverage.coverageInfo.size(),
               "Coverage info with region not equal to coverage array size (not precise coverage) cannot be used as local coverage cache",);
    localCoverageCache = coverage;
}

bool AssemblyBrowser::isInLocalCoverageCache(qint64 position) {
    return localCoverageCache.region.contains(position);
}

qint64 AssemblyBrowser::getCoverageAtPos(qint64 pos) {
    if(isInLocalCoverageCache(pos)) {
        return localCoverageCache.coverageInfo.at(pos - localCoverageCache.region.startPos);
    } else {
        U2OpStatus2Log status;
        U2AssemblyCoverageStat coverageStat;
        coverageStat.coverage.resize(1);
        model->calculateCoverageStat(U2Region(pos, 1), coverageStat, status);
        return coverageStat.coverage.first().maxValue;
    }
}

bool AssemblyBrowser::intersectsLocalCoverageCache(U2Region region) {
    return !localCoverageCache.region.isEmpty() && localCoverageCache.region.intersects(region);
}

bool AssemblyBrowser::isInLocalCoverageCache(U2Region region) {
    return localCoverageCache.region.contains(region);
}

CoverageInfo AssemblyBrowser::extractFromLocalCoverageCache(U2Region region) {
    CoverageInfo ci;
    ci.region = region;
    ci.coverageInfo.resize(region.length);

    if(intersectsLocalCoverageCache(region)) {
        U2Region intersection = localCoverageCache.region.intersect(region);
        SAFE_POINT(!intersection.isEmpty(), "intersection cannot be empty", ci);

        int offsetInCache = intersection.startPos - localCoverageCache.region.startPos;
        int offsetInResult = intersection.startPos - region.startPos;
        memcpy(ci.coverageInfo.data() + offsetInResult, localCoverageCache.coverageInfo.constData() + offsetInCache, intersection.length*sizeof(ci.coverageInfo[0]));
        ci.updateStats();
    }
    return ci;
}

int AssemblyBrowser::getCellWidth() const {
    return calcPixelCoord(1);
}

qint64 AssemblyBrowser::calcAsmCoordX(qint64 xPixCoord) const {
    U2OpStatusImpl status;
    qint64 modelLen = model->getModelLength(status);
    qint64 width = ui->getReadsArea()->width();
    qint64 xAsmCoord = (double(modelLen) / width * zoomFactor * double(xPixCoord)) + 0.5;
    return xAsmCoord;
}

qint64 AssemblyBrowser::calcAsmCoordY(qint64 pixCoord)const {
    U2OpStatusImpl status;
    qint64 modelHeight = model->getModelHeight(status);
    LOG_OP(status);
    qint64 h = ui->getReadsArea()->height();
    return (double(modelHeight) / h * zoomFactor * double(pixCoord)) + 0.5;
}

qint64 AssemblyBrowser::calcAsmPosX(qint64 pixPosX) const {
    int cellWidth = getCellWidth();
    if(cellWidth == 0) {
        return xOffsetInAssembly + calcAsmCoordX(pixPosX);
    }
    return xOffsetInAssembly + (double)pixPosX / cellWidth;
}

qint64 AssemblyBrowser::calcAsmPosY(qint64 pixPosY) const {
    int cellWidth = getCellWidth();
    if(cellWidth == 0) {
        return yOffsetInAssembly + calcAsmCoordY(pixPosY);
    }
    return yOffsetInAssembly + (double)pixPosY / cellWidth;
}

qint64 AssemblyBrowser::calcPixelCoord(qint64 xAsmCoord) const {
    U2OpStatusImpl status;
    qint64 modelLen = model->getModelLength(status);
    qint64 width = ui->getReadsArea()->width();

    SAFE_POINT(modelLen != 0, "modelLen == 0, cannot divide to find pixel coordinate", 0);
    qint64 xPixelCoord = (double(width) / modelLen * double(xAsmCoord)) / zoomFactor + 0.5;
    return xPixelCoord;
}

qint64 AssemblyBrowser::calcPainterOffset(qint64 xAsmCoord) const {
    qint64 letterWidth = getCellWidth();
    if(!(letterWidth > 0)) {
        return calcPixelCoord(xAsmCoord);
    }
    qint64 result = letterWidth * xAsmCoord;
    return result;
}

qint64 AssemblyBrowser::basesCanBeVisible() const {
    int width = ui->getReadsArea()->width();
    qint64 letterWidth = getCellWidth();
    if(0 == letterWidth) {
        return calcAsmCoordX(width);
    }
    qint64 result = width / letterWidth + !!(width % letterWidth);
    return result;
}

qint64 AssemblyBrowser::basesVisible() const {
    U2OpStatusImpl status;
    qint64 modelLength = model->getModelLength(status);
    LOG_OP(status);
    qint64 maxLength = modelLength - getXOffsetInAssembly();
    return qMin(maxLength, basesCanBeVisible());
}

qint64 AssemblyBrowser::rowsCanBeVisible() const {
    int height = ui->getReadsArea()->height();
    qint64 letterWidth = getCellWidth();
    if(0 == letterWidth) {
        return calcAsmCoordX(height);
    }
    qint64 result = height / letterWidth + !!(height % letterWidth);
    return result;
}

qint64 AssemblyBrowser::rowsVisible() const {
    U2OpStatusImpl status;
    qint64 modelHeight = model->getModelHeight(status);
    return qMin(rowsCanBeVisible(), modelHeight);
}

bool AssemblyBrowser::areReadsVisible() const {
    int readWidthPix = calcPixelCoord(1); // TODO: average read length ? 
    return readWidthPix >= 1;
}

bool AssemblyBrowser::areCellsVisible() const {
    return getCellWidth() >= CELL_VISIBLE_WIDTH;
}

bool AssemblyBrowser::areLettersVisible() const {
    return getCellWidth() >= LETTER_VISIBLE_WIDTH;
}

qint64 AssemblyBrowser::normalizeXoffset(qint64 x) const {
    if(x < 0) {
        return 0;
    }
    U2OpStatusImpl st;
    // hack: first case is needed to show last column
    qint64 maxWindowSize = areCellsVisible() ? basesCanBeVisible() - 1 : basesCanBeVisible();
    qint64 xMax = model->getModelLength(st) - qMax((qint64)1, maxWindowSize);
    LOG_OP(st);
    if(xMax < 0) {
        return 0;
    }
    if(x > xMax && xMax >= 0) {
        return xMax;
    }
    return x;
}

qint64 AssemblyBrowser::normalizeYoffset(qint64 y) const {
    if(y < 0) {
        return 0;
    }
    U2OpStatusImpl st;
    qint64 yMax = model->getModelHeight(st) - qMax((qint64)1, rowsCanBeVisible() - 2);
    LOG_OP(st);
    if(yMax < 0) {
        return 0;
    }
    if(y > yMax && yMax >= 0) {
        return yMax;
    }
    return y;
}

void AssemblyBrowser::setXOffsetInAssembly(qint64 x) {
    U2OpStatusImpl st;
    qint64 len = model->getModelLength(st); Q_UNUSED(len);
    assert(x >= 0 && (x < len || len == 0)); // len == 0 in case of empty model
    xOffsetInAssembly = x;
    emit si_offsetsChanged();
}

void AssemblyBrowser::setYOffsetInAssembly(qint64 y) {
    U2OpStatusImpl st;
    qint64 height = model->getModelHeight(st); Q_UNUSED(height);
    assert(y >= 0 && (y < height || height == 0));
    yOffsetInAssembly = y;
    emit si_offsetsChanged();
}

void AssemblyBrowser::setOffsetsInAssembly(qint64 x, qint64 y) {
    U2OpStatusImpl st; Q_UNUSED(st);
    qint64 len = model->getModelLength(st); Q_UNUSED(len);
    qint64 height = model->getModelHeight(st); Q_UNUSED(height);
    assert(x >= 0 && (x < len || len == 0));
    assert(y >= 0 && (y < height || height == 0));
    xOffsetInAssembly = x;
    yOffsetInAssembly = y;
    emit si_offsetsChanged();
}

void AssemblyBrowser::adjustOffsets(qint64 dx, qint64 dy) {
    //U2OpStatusImpl status;
    //qint64 modelLen = model->getModelLength(status);
    //qint64 modelHeight = model->getModelHeight(status);

    xOffsetInAssembly = normalizeXoffset(xOffsetInAssembly + dx);
    yOffsetInAssembly = normalizeYoffset(yOffsetInAssembly + dy);
    emit si_offsetsChanged();
}

void AssemblyBrowser::setFocusToPosSelector() {
    posSelector->getPosEdit()->setFocus();
}

void AssemblyBrowser::sl_assemblyLoaded() {
    assert(model);
    GTIMER(c1, t1, "AssemblyBrowser::sl_assemblyLoaded");
    LOG_OP(dbiOpStatus);
    U2Dbi * dbi = model->getDbiConnection().dbi;
    assert(U2DbiState_Ready == dbi->getState());

    U2AssemblyDbi * assmDbi = dbi->getAssemblyDbi();

    U2DataId objectId = gobject->getEntityRef().entityId;
    U2Assembly assm = dbi->getAssemblyDbi()->getAssemblyObject(objectId, dbiOpStatus);
    LOG_OP(dbiOpStatus);

    model->setAssembly(assmDbi, assm);
}


void AssemblyBrowser::navigateToRegion(const U2Region & region) {
    int requiredCellSize = qMax(1, qRound((double)ui->getReadsArea()->width()/region.length));
    zoomToSize(requiredCellSize);

    //if cells are not visible -> make them visible
    if(!areCellsVisible()) {
        while(!areCellsVisible()) {
            sl_zoomIn();
        }
    }
    
    //if visible area does not contain reads area -> shift reads area
    if(!getVisibleBasesRegion().contains(region)) {
        setXOffsetInAssembly(region.startPos);
    }
}

void AssemblyBrowser::initFont() {
    font.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
}

void AssemblyBrowser::setupActions() {
    zoomInAction = new QAction(QIcon(":core/images/zoom_in.png"), tr("Zoom in"), this);
    connect(zoomInAction, SIGNAL(triggered()), SLOT(sl_zoomIn()));

    zoomOutAction = new QAction(QIcon(":core/images/zoom_out.png"), tr("Zoom out"), this);
    connect(zoomOutAction, SIGNAL(triggered()), SLOT(sl_zoomOut()));
    
    QAction * linearScaleAction = new QAction(tr("Linear"), this);
    linearScaleAction->setCheckable(true);
    QAction * logScaleAction = new QAction(tr("Logarithmic"), this);
    logScaleAction->setCheckable(true);
    connect(linearScaleAction, SIGNAL(triggered()), SLOT(sl_changeOverviewType()));
    connect(logScaleAction, SIGNAL(triggered()), SLOT(sl_changeOverviewType()));
    overviewScaleTypeActions << linearScaleAction << logScaleAction;
    
    showCoordsOnRulerAction = new QAction(QIcon(":core/images/notch.png"), tr("Show coordinates on ruler"), this);
    showCoordsOnRulerAction->setCheckable(true);
    connect(showCoordsOnRulerAction, SIGNAL(toggled(bool)), SLOT(sl_onShowCoordsOnRulerChanged(bool)));

    showCoverageOnRulerAction = new QAction(QIcon(":core/images/ruler_coverage.png"), tr("Show coverage under ruler cursor"), this);
    showCoverageOnRulerAction->setCheckable(true);
    connect(showCoverageOnRulerAction, SIGNAL(toggled(bool)), SLOT(sl_onShowCoverageOnRulerChanged(bool)));

    readHintEnabledAction = new QAction(QIcon(":core/images/tooltip.png"), tr("Show information about read under cursor in pop-up hint"), this);
    readHintEnabledAction->setCheckable(true);
    connect(readHintEnabledAction, SIGNAL(toggled(bool)), SLOT(sl_onReadHintEnabledChanged(bool)));
    
    saveScreenShotAction = new QAction(QIcon(":/core/images/cam2.png"), tr("Export as image"), this);
    connect(saveScreenShotAction, SIGNAL(triggered()), SLOT(sl_saveScreenshot()));
    
    showInfoAction = new QAction(QIcon(":ugene/images/task_report.png"), tr("Show assembly information"), this);
    connect(showInfoAction, SIGNAL(triggered()), SLOT(sl_showAssemblyInfo()));

    exportToSamAction = new QAction(QIcon(":/core/images/sam.png"), tr("Export assembly to SAM format"), this);
    connect(exportToSamAction, SIGNAL(triggered()), SLOT(sl_exportToSam()));
}

void AssemblyBrowser::sl_showAssemblyInfo() {
    QDialog dlg(ui, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    dlg.setWindowTitle(tr("Assembly '%1' Information").arg(gobject->getGObjectName()));
    dlg.setLayout(new QVBoxLayout());
    QLabel * infoLabel = new QLabel();
    {
        U2OpStatusImpl st;
        QString text = "<table>";
        text += QString("<tr><td><b>Name:&nbsp;</b></td><td>%1</td></tr>").arg(gobject->getGObjectName());
        text += QString("<tr><td><b>Length:&nbsp;</b></td><td>%1</td></tr>").arg(FormatUtils::insertSeparators(model->getModelLength(st)));
        text += QString("<tr><td><b>Number of reads:&nbsp;</b></td><td>%1</td></tr>").arg(FormatUtils::insertSeparators(model->getReadsNumber(st)));
        QByteArray md5 = model->getReferenceMd5(st);
        if(!md5.isEmpty()) {
            text += QString("<tr><td><b>MD5:&nbsp;</b></td><td>%1</td></tr>").arg(QString(md5));
        }
        QByteArray species = model->getReferenceSpecies(st);
        if(!species.isEmpty()) {
            text += QString("<tr><td><b>Species:&nbsp;</b></td><td>%1</td></tr>").arg(QString(species));
        }
        QString uri = model->getReferenceUri(st);
        if(!uri.isEmpty()) {
            text += QString("<tr><td><b>URI:&nbsp;</b></td><td>%1</td></tr>").arg(uri);
        }
        text += "</table>";
        infoLabel->setText(text);
        infoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }
    dlg.layout()->addWidget(infoLabel);
    
    dlg.resize(300, dlg.sizeHint().height());
    dlg.setMaximumHeight(dlg.layout()->minimumSize().height());
    dlg.exec();
}

void AssemblyBrowser::sl_saveScreenshot() {
    ExportImageDialog dialog(ui);
    dialog.exec();
}

void AssemblyBrowser::sl_exportToSam() {
    U2OpStatusImpl os;
    QHash<QString, QString> metaInfo = model->getDbiConnection().dbi->getDbiMetaInfo(os);

    ConvertAssemblyToSamDialog dialog(ui, metaInfo[U2_DBI_OPTION_URL]);

    if (dialog.exec()) {
        ConvertAssemblyToSamTask *convertTask = new ConvertAssemblyToSamTask(&(model->getDbiConnection()), dialog.getSamFileUrl());
        AppContext::getTaskScheduler()->registerTopLevelTask(convertTask);
    }
}

void AssemblyBrowser::sl_onShowCoordsOnRulerChanged(bool checked) {
    ui->getRuler()->setShowCoordsOnRuler(checked);
}

void AssemblyBrowser::sl_onShowCoverageOnRulerChanged(bool checked) {
    ui->getRuler()->setShowCoverageOnRuler(checked);
}

void AssemblyBrowser::sl_onReadHintEnabledChanged(bool checked) {
    ui->getReadsArea()->setReadHintEnabled(checked);
}

void AssemblyBrowser::sl_changeOverviewType() {
    QAction * a = qobject_cast<QAction*>(sender());
    if(a == NULL) {
        assert(false);
        return;
    }
    
    AssemblyBrowserSettings::OverviewScaleType t(AssemblyBrowserSettings::Scale_Linear);
    if(a == overviewScaleTypeActions[1]) {
        t = AssemblyBrowserSettings::Scale_Logarithmic;
    } else if(a != overviewScaleTypeActions[0]) {
        assert(false);
    }
    
    ui->getOverview()->setScaleType(t);
    updateOverviewTypeActions();
}

void AssemblyBrowser::updateZoomingActions() {
    bool enableZoomOut = INITIAL_ZOOM_FACTOR != zoomFactor;
    zoomOutAction->setEnabled(enableZoomOut);
    if(posSelector != NULL) {
        posSelector->setEnabled(enableZoomOut);
    }
    
    // decide if on next zoom cellWidth will increase max width
    {
        bool enableZoomIn = false;
        double curZoomFactor = zoomFactor;
        enableZoomIn = !(zoomInFromSize(getCellWidth()) > MAX_CELL_WIDTH);
        zoomFactor = curZoomFactor;
        zoomInAction->setEnabled(enableZoomIn);
    }
}

void AssemblyBrowser::updateOverviewTypeActions() {
    AssemblyBrowserSettings::OverviewScaleType t(ui->getOverview()->getScaleType());
    overviewScaleTypeActions[0]->setChecked(t == AssemblyBrowserSettings::Scale_Linear);
    overviewScaleTypeActions[1]->setChecked(t == AssemblyBrowserSettings::Scale_Logarithmic);
}

void AssemblyBrowser::sl_zoomIn(const QPoint & pos) {
    if(!canPerformZoomIn()) {
        return;
    }
    
    qint64 oldWidth = basesCanBeVisible();
    qint64 posXAsmCoord = calcAsmPosX(pos.x());
    //qint64 posYAsmCoord = calcAsmPosY(pos.y());
    
    // zoom in
    {
        int oldCellSize = getCellWidth();
        if(!oldCellSize) { 
            zoomFactor /= ZOOM_MULT;
        } else { 
            int cellWidth = zoomInFromSize(oldCellSize);
            Q_UNUSED(cellWidth);
            assert(cellWidth <= MAX_CELL_WIDTH);
        }
    }
    
    // calculate new offsets
    qint64 newXOff = 0;
    int cellWidth = getCellWidth();
    if(!pos.isNull() && cellWidth != 0) {
        newXOff = posXAsmCoord - pos.x() / cellWidth;
    } else {
        //zooming to the center of the screen
        newXOff = xOffsetInAssembly + (oldWidth - basesCanBeVisible()) / 2;
    }
    setXOffsetInAssembly(normalizeXoffset(newXOff));
    
    updateZoomingActions();
    emit si_zoomOperationPerformed();
}

void AssemblyBrowser::sl_zoomOut(const QPoint & pos) {
    if(!canPerformZoomOut()) {
        return;
    }

    qint64 oldWidth = basesVisible();
    qint64 posXAsmCoord = calcAsmPosX(pos.x());
    //qint64 posYAsmCoord = calcAsmPosY(pos.y());
    
    // zoom out
    {
        int oldCellSize = getCellWidth();
        if(zoomFactor * ZOOM_MULT > INITIAL_ZOOM_FACTOR) { // next zoom to far
            zoomFactor = INITIAL_ZOOM_FACTOR;
        } else if(!oldCellSize) {
            zoomFactor *= ZOOM_MULT;
        } else {
            zoomOutFromSize(oldCellSize);
        }
    }
    
    // calculate new offsets
    qint64 newXOff = 0;
    int cellWidth = getCellWidth();
    if(!pos.isNull() && cellWidth != 0) {
        newXOff = posXAsmCoord - pos.x() / cellWidth;
    } else {
        //zooming out of the center
        newXOff = xOffsetInAssembly + (oldWidth - basesCanBeVisible()) / 2;
    }
    setXOffsetInAssembly(normalizeXoffset(newXOff));
    
    updateZoomingActions();
    emit si_zoomOperationPerformed();
}

void AssemblyBrowser::sl_zoomToReads() {
    if(!areReadsVisible()) {
        zoomInFromSize(0);
        updateZoomingActions();
        emit si_zoomOperationPerformed();
    }
}

int AssemblyBrowser::zoomInFromSize(int oldCellSize) {
    SAFE_POINT(oldCellSize >= 0, "oldCellSize < 0, zooming will not work correctly!", oldCellSize);

    //single decreasing of the zoomFactor not always changes the cell size
    //so we have to do it in the cycle, until cells grow
    int cellWidth = 0;
    do {
        zoomFactor /= ZOOM_MULT;
        cellWidth = getCellWidth();
    } while(oldCellSize == cellWidth);
    return cellWidth;
}

int AssemblyBrowser::zoomOutFromSize(int oldCellSize) {
    SAFE_POINT(oldCellSize >= 0, "oldCellSize < 0, zooming will not work correctly!", oldCellSize);

    //single increasing of the zoomFactor not always changes the cell size
    //so we have to do it in the cycle
    int cellWidth = 0;
    do {
        zoomFactor *= ZOOM_MULT;
        cellWidth = getCellWidth();
    } while(cellWidth == oldCellSize);
    return cellWidth;
}

void AssemblyBrowser::zoomToSize(int reqCellSize) {
    SAFE_POINT(reqCellSize > 0, "reqCellSize <= 0, cannot zoomToSize",);

    U2OpStatus2Log status;
    qint64 modelLen = model->getModelLength(status);
    qint64 width = ui->getReadsArea()->width();
    zoomFactor = double(width)/modelLen/(reqCellSize - 0.5);

    updateZoomingActions();
    emit si_zoomOperationPerformed();
}

void AssemblyBrowser::onObjectRenamed(GObject*, const QString&) {
    OpenAssemblyBrowserTask::updateTitle(this);
}

void AssemblyBrowser::sl_coveredRegionClicked(const QString link) {
    if(link == AssemblyReadsArea::ZOOM_LINK) {
        sl_zoomToReads();
    } else {
        bool ok;
        int i = link.toInt(&ok);
        assert(ok);
        CoveredRegion cr = getCoveredRegions().at(i);
        ui->getOverview()->checkedSetVisibleRange(cr.region);
        navigateToRegion(cr.region);
    }
}

//==============================================================================
// AssemblyBrowserUi
//==============================================================================

AssemblyBrowserUi::AssemblyBrowserUi(AssemblyBrowser * browser_) : browser(browser_), zoomableOverview(0), 
referenceArea(0), coverageGraph(0), ruler(0), readsArea(0), annotationsArea(0) {
    U2OpStatusImpl os;
    if(browser->getModel()->hasReads(os)) { // has mapped reads -> show rich visualization
        setMinimumSize(300, 200);

        QScrollBar * readsHBar = new QScrollBar(Qt::Horizontal);
        QScrollBar * readsVBar = new QScrollBar(Qt::Vertical);

        zoomableOverview = new ZoomableAssemblyOverview(this, true); //zooming temporarily disabled -iefremov
        referenceArea = new AssemblyReferenceArea(this);
        consensusArea = new AssemblyConsensusArea(this);
        coverageGraph = new AssemblyCoverageGraph(this);
        ruler = new AssemblyRuler(this);
        readsArea  = new AssemblyReadsArea(this, readsHBar, readsVBar);
        annotationsArea = new AssemblyAnnotationsArea(this);

        QVBoxLayout *mainLayout = new QVBoxLayout();
        mainLayout->setMargin(0);
        mainLayout->setSpacing(2);
        mainLayout->addWidget(zoomableOverview);

        QGridLayout * readsLayout = new QGridLayout();
        readsLayout->setMargin(0);
        readsLayout->setSpacing(0);

        readsLayout->addWidget(referenceArea, 0, 0);
        readsLayout->addWidget(consensusArea, 1, 0);
        readsLayout->addWidget(annotationsArea, 2, 0);
        readsLayout->addWidget(ruler, 3, 0);
        readsLayout->addWidget(coverageGraph, 4, 0);

        readsLayout->addWidget(readsArea, 5, 0);
        readsLayout->addWidget(readsVBar, 5, 1, 1, 1);
        readsLayout->addWidget(readsHBar, 5, 0);

        QWidget * readsLayoutWidget = new QWidget;
        readsLayoutWidget->setLayout(readsLayout);
        mainLayout->addWidget(readsLayoutWidget);
        mainLayout->addWidget(readsHBar);

        OPWidgetFactoryRegistry* opWidgetFactoryRegistry = AppContext::getOPWidgetFactoryRegistry();
        OptionsPanel * optionsPanel = browser->getOptionsPanel();
        QList<OPWidgetFactory*> opWidgetFactoriesForAssBr =
            opWidgetFactoryRegistry->getRegisteredFactories(ObjViewType_AssemblyBrowser);
        foreach (OPWidgetFactory* factory, opWidgetFactoriesForAssBr) {
            optionsPanel->addGroup(factory);
        }

        setLayout(mainLayout);

        connect(readsArea, SIGNAL(si_heightChanged()), zoomableOverview, SLOT(sl_visibleAreaChanged()));
        connect(readsArea, SIGNAL(si_mouseMovedToPos(const QPoint&)), ruler, SLOT(sl_handleMoveToPos(const QPoint&)));
        connect(referenceArea, SIGNAL(si_mouseMovedToPos(const QPoint&)), ruler, SLOT(sl_handleMoveToPos(const QPoint&)));
        connect(consensusArea, SIGNAL(si_mouseMovedToPos(const QPoint&)), ruler, SLOT(sl_handleMoveToPos(const QPoint&)));
        connect(coverageGraph, SIGNAL(si_mouseMovedToPos(const QPoint&)), ruler, SLOT(sl_handleMoveToPos(const QPoint&)));
        connect(annotationsArea, SIGNAL(si_mouseMovedToPos(const QPoint&)), ruler, SLOT(sl_handleMoveToPos(const QPoint&)));
        connect(browser, SIGNAL(si_offsetsChanged()), readsArea, SLOT(sl_hideHint()));
        connect(browser->getModel().data(), SIGNAL(si_referenceChanged()), referenceArea, SLOT(sl_redraw()));
        connect(browser->getModel().data(), SIGNAL(si_referenceChanged()), readsArea, SLOT(sl_redraw()));
        connect(browser->getModel().data(), SIGNAL(si_referenceChanged()), consensusArea, SLOT(sl_redraw()));
        connect(zoomableOverview, SIGNAL(si_coverageReady()), readsArea, SLOT(sl_redraw()));
        connect(referenceArea, SIGNAL(si_unassociateReference()), browser->getModel().data(), SLOT(sl_unassociateReference()));
    } 
    // do not how to show them
    else {
        QVBoxLayout * mainLayout = new QVBoxLayout();
        QString msg = tr("Assembly has no mapped reads. Nothing to visualize.");
        QLabel * infoLabel = new QLabel(QString("<table align=\"center\"><tr><td>%1</td></tr></table>").arg(msg));
        infoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainLayout->addWidget(infoLabel);
        setLayout(mainLayout);
    }
}

QColor AssemblyBrowserUi::getCoverageColor(double grayCoeff)
{
    return QColor(80-60*grayCoeff, 160-100*grayCoeff, 200-130*grayCoeff);
}

} //ns
