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

#include "AssemblyBrowser.h"

#include "AssemblyBrowserFactory.h"
#include "ZoomableAssemblyOverview.h"
#include "AssemblyReferenceArea.h"
#include "AssemblyDensityGraph.h"
#include "AssemblyRuler.h"
#include "AssemblyReadsArea.h"
#include "AssemblyBrowserSettings.h"

#include <U2Core/U2Type.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/FormatUtils.h>

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
#include <U2Gui/GUIUtils.h>
#include <U2Gui/ExportImageDialog.h>

#include <U2Misc/DialogUtils.h>
#include <U2Misc/PositionSelector.h>

#include <memory>

namespace U2 {

//==============================================================================
// AssemblyBrowser
//==============================================================================
const double AssemblyBrowser::ZOOM_MULT = 1.25;
const double AssemblyBrowser::INITIAL_ZOOM_FACTOR= 1.;

AssemblyBrowser::AssemblyBrowser(AssemblyObject * o) : 
GObjectView(AssemblyBrowserFactory::ID, GObjectViewUtils::genUniqueViewName(o->getDocument(), o)), ui(0),
gobject(o), model(0), zoomFactor(INITIAL_ZOOM_FACTOR), xOffsetInAssembly(0), yOffsetInAssembly(0), coverageReady(false),
zoomInAction(0), zoomOutAction(0), posSelectorAction(0), posSelector(0), showCoordsOnRulerAction(0), saveScreenShotAction(0),
showInfoAction(0)
{
    initFont();
    setupActions();
 
    if(gobject) {
        objects.append(o);
        requiredObjects.append(o);
        const U2DataRef& ref= gobject->getDbiRef();
        model = QSharedPointer<AssemblyModel>(new AssemblyModel(DbiHandle(ref.factoryId, ref.dbiId, dbiOpStatus)));
        sl_assemblyLoaded();
    }   
}

QWidget * AssemblyBrowser::createWidget() {
    ui = new AssemblyBrowserUi(this);
    U2OpStatusImpl os;
    if(model->hasReads(os)) {
        updateOverviewTypeActions();
        showCoordsOnRulerAction->setChecked(ui->getRuler()->getShowCoordsOnRuler());
        ui->installEventFilter(this);
        ui->setAcceptDrops(true);
    }
    return ui;
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
    DNASequenceObject * seqObj = qobject_cast<DNASequenceObject*>(obj);
    if(seqObj == NULL) {
        return tr("Only sequence object can be added to assembly browser");
    }
    Document * seqDoc = seqObj->getDocument();
    if(seqDoc == NULL) {
        assert(false);
        return tr("Internal error: only sequence with document can be added to browser");
    }
    assert(seqDoc->getDocumentFormat() != NULL);
    
    U2SequenceDbi * seqDbi = seqObj->asDbi();
    assert(seqDbi != NULL);
    U2OpStatusImpl status;
    QString seqObjName = seqObj->getGObjectName();
    U2Sequence u2SeqObj = seqDbi->getSequenceObject(seqObjName.toAscii(), status);
    if(status.hasError()) {
        return status.getError();
    }
        
    QStringList errs;
    qint64 modelLen = model->getModelLength(status);
    if(u2SeqObj.length != modelLen) {
        errs << tr("- Reference sequence is %1 than assembly").arg(u2SeqObj.length < modelLen ? tr("lesser") : tr("bigger"));
    }
    if(seqObjName != gobject->getGObjectName()) {
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
        model->setReference(seqDbi, u2SeqObj);
        QMessageBox::StandardButtons fl = QMessageBox::Yes | QMessageBox::No;
        QMessageBox::StandardButton btn = QMessageBox::question(ui, tr("Question"), tr("Associate assembly with '%1'?").arg(seqObjName), fl, QMessageBox::Yes);
        if(QMessageBox::Yes == btn) {
            U2CrossDatabaseReferenceDbi * crossDbi = model->getDbiHandle().dbi->getCrossDatabaseReferenceDbi();
            U2CrossDatabaseReference crossDbRef;
            crossDbRef.dataRef.dbiId = u2SeqObj.dbiId;
            crossDbRef.dataRef.entityId = u2SeqObj.id;
            crossDbRef.dataRef.version = 1;
            crossDbRef.dataRef.factoryId = "FileDbi_" + seqDoc->getDocumentFormatId();
            crossDbi->createCrossReference(crossDbRef, status);
            LOG_OP(status);
            model->associateWithReference(crossDbRef);
        }
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
        tb->addAction(saveScreenShotAction);
        tb->addAction(showInfoAction);
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
    }
    GObjectView::buildStaticMenu(m);
    GUIUtils::disableEmptySubmenus(m);
}

void AssemblyBrowser::setGlobalCoverageInfo(const CoverageInfo & newInfo) {
    coverageReady = true;
    if(newInfo.coverageInfo.size() <= coveredRegionsManager.getSize()) {
        return;
    }
    U2OpStatusImpl os;
    U2Region globalRegion(0, model->getModelLength(os));
    coveredRegionsManager = CoveredRegionsManager(globalRegion, newInfo.coverageInfo);
    coverageInfo = newInfo;
}

QList<CoveredRegion> AssemblyBrowser::getCoveredRegions() const {
    if(!coveredRegionsManager.isEmpty()) {
        return coveredRegionsManager.getTopCoveredRegions(10, 1);
    }
    return QList<CoveredRegion>();
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
    return qMin(modelLength, basesCanBeVisible());
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
    U2OpStatusImpl status;
    qint64 modelLen = model->getModelLength(status);
    qint64 modelHeight = model->getModelHeight(status);

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
    U2Dbi * dbi = model->getDbiHandle().dbi;
    assert(U2DbiState_Ready == dbi->getState());

    U2AssemblyDbi * assmDbi = dbi->getAssemblyDbi();

    U2DataId objectId = gobject->getDbiRef().entityId;
    U2Assembly assm = dbi->getAssemblyDbi()->getAssemblyObject(objectId, dbiOpStatus);
    LOG_OP(dbiOpStatus);

    model->setAssembly(assmDbi, assm);
}


void AssemblyBrowser::navigateToRegion(const U2Region & region) {
    //if cells are not visible -> make them visible
    if(!areCellsVisible()) {
        while(!areCellsVisible()) {
            sl_zoomIn();
        }
    }
    
    //if visible area does not contain reads area -> shift reads area
    if(xOffsetInAssembly < region.startPos) {
        setXOffsetInAssembly(region.startPos);
    } else if(xOffsetInAssembly > region.endPos()-region.length) {
        setXOffsetInAssembly(region.endPos()-region.length);
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
    connect(showCoordsOnRulerAction, SIGNAL(triggered()), SLOT(sl_onShowCoordsOnRulerChanged()));
    
    saveScreenShotAction = new QAction(QIcon(":/core/images/cam2.png"), tr("Export as image"), this);
    connect(saveScreenShotAction, SIGNAL(triggered()), SLOT(sl_saveScreenshot()));
    
    showInfoAction = new QAction(QIcon(":ugene/images/task_report.png"), tr("Show assembly information"), this);
    connect(showInfoAction, SIGNAL(triggered()), SLOT(sl_showContigInfo()));
}

void AssemblyBrowser::sl_showContigInfo() {
    QDialog dlg(ui, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    dlg.setWindowTitle(tr("'%1' Contig Information").arg(gobject->getGObjectName()));
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

void AssemblyBrowser::sl_onShowCoordsOnRulerChanged() {
    bool showRulerCoords = showCoordsOnRulerAction->isChecked();
    ui->getRuler()->setShowCoordsOnRuler(showRulerCoords);
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
    qint64 posYAsmCoord = calcAsmPosY(pos.y());
    
    // zoom in
    {
        int oldCellSize = getCellWidth();
        if(!oldCellSize) { 
            zoomFactor /= ZOOM_MULT;
        } else { 
            int cellWidth = zoomInFromSize(oldCellSize);
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
    qint64 posYAsmCoord = calcAsmPosY(pos.y());
    
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
        emit si_zoomOperationPerformed();
    }
}

int AssemblyBrowser::zoomInFromSize(int oldCellSize) {
    assert(oldCellSize >= 0);
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
    assert(oldCellSize);
    //single increasing of the zoomFactor not always changes the cell size
    //so we have to do it in the cycle
    int cellWidth = 0;
    do {
        zoomFactor *= ZOOM_MULT;
        cellWidth = getCellWidth();
    } while(cellWidth == oldCellSize);
    return cellWidth;
}

//==============================================================================
// AssemblyBrowserUi
//==============================================================================

AssemblyBrowserUi::AssemblyBrowserUi(AssemblyBrowser * browser_) : browser(browser_), zoomableOverview(0), 
referenceArea(0), densityGraph(0), ruler(0), readsArea(0){
    U2OpStatusImpl os;
    if(browser->getModel()->hasReads(os)) { // has mapped reads -> show rich visualization
        setMinimumSize(300, 200);

        QScrollBar * readsHBar = new QScrollBar(Qt::Horizontal);
        QScrollBar * readsVBar = new QScrollBar(Qt::Vertical);

        zoomableOverview = new ZoomableAssemblyOverview(this, true); //zooming temporarily disabled -iefremov
        referenceArea = new AssemblyReferenceArea(this);
        //densityGraph = new AssemblyDensityGraph(this);
        densityGraph = NULL;
        ruler = new AssemblyRuler(this);
        readsArea  = new AssemblyReadsArea(this, readsHBar, readsVBar);

        QVBoxLayout *mainLayout = new QVBoxLayout();
        mainLayout->setMargin(0);
        mainLayout->setSpacing(2);
        mainLayout->addWidget(zoomableOverview);

        QGridLayout * readsLayout = new QGridLayout();
        readsLayout->setMargin(0);
        readsLayout->setSpacing(0);

        //readsLayout->addWidget(densityGraph);
        readsLayout->addWidget(referenceArea, 0, 0);

        readsLayout->addWidget(ruler, 1, 0);

        readsLayout->addWidget(readsArea, 2, 0);
        readsLayout->addWidget(readsVBar, 2, 1, 1, 1);
        //readsLayout->addWidget(readsHBar, 3, 0);

        QWidget * readsLayoutWidget = new QWidget;
        readsLayoutWidget->setLayout(readsLayout);
        mainLayout->addWidget(readsLayoutWidget);
        mainLayout->addWidget(readsHBar);

        setLayout(mainLayout);  

        connect(readsArea, SIGNAL(si_heightChanged()), zoomableOverview, SLOT(sl_visibleAreaChanged()));
        connect(readsArea, SIGNAL(si_mouseMovedToPos(const QPoint&)), ruler, SLOT(sl_handleMoveToPos(const QPoint&)));
        connect(referenceArea, SIGNAL(si_mouseMovedToPos(const QPoint&)), ruler, SLOT(sl_handleMoveToPos(const QPoint&)));
        connect(browser, SIGNAL(si_offsetsChanged()), readsArea, SLOT(sl_hideHint()));
        connect(browser->getModel().data(), SIGNAL(si_referenceChanged()), referenceArea, SLOT(sl_redraw()));
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

} //ns
