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
#include "AssemblyOverview.h"
#include "AssemblyReferenceArea.h"
#include "AssemblyDensityGraph.h"
#include "AssemblyRuler.h"
#include "AssemblyReadsArea.h"
#include "AssemblyBrowserSettings.h"

#include <U2Core/U2Type.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/DNASequenceObject.h>

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

#include <U2Core/AppContext.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/Log.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/Timer.h>
#include <U2Gui/GUIUtils.h>

#include <U2Misc/DialogUtils.h>
#include <U2Misc/PositionSelector.h>

#include <memory>

namespace U2 {

const double AssemblyBrowser::ZOOM_MULT = 1.25;
const double AssemblyBrowser::INITIAL_ZOOM_FACTOR= 1.;

//==============================================================================
// AssemblyModel
//==============================================================================

AssemblyModel::AssemblyModel(const DbiHandle & dbiHandle_) : 
cachedModelLength(NO_VAL), cachedModelHeight(NO_VAL), referenceDbi(0), dbiHandle(dbiHandle_) 
{
}


bool AssemblyModel::isEmpty() const {
    assert(assemblies.isEmpty() == assemblyDbis.isEmpty());
    return assemblies.isEmpty() || assemblyDbis.isEmpty(); 
}

QList<U2AssemblyRead> AssemblyModel::getReadsFromAssembly(int assIdx, const U2Region & r, qint64 minRow, qint64 maxRow, U2OpStatus & os) {
    std::auto_ptr< U2DbiIterator<U2AssemblyRead> > it(assemblyDbis.at(assIdx)->getReadsByRow(assemblies.at(assIdx).id, r, minRow, maxRow, os));
    return U2DbiUtils::toList(it.get());
}

qint64 AssemblyModel::countReadsInAssembly(int assIdx, const U2Region & r, U2OpStatus & os) {
    return assemblyDbis.at(assIdx)->countReads(assemblies.at(assIdx).id, r, os);
}

qint64 AssemblyModel::getModelLength(U2OpStatus & os) {
    if(NO_VAL == cachedModelLength) {
        //TODO: length must be calculated as length of the reference sequence or
        //if there is no reference, as maximum of all assembly lengths in the model
        qint64 refLen = hasReference() ? reference.length : 0;
        qint64 assLen = assemblyDbis.at(0)->getMaxEndPos(assemblies.at(0).id, os);
        cachedModelLength = qMax(refLen, assLen);
    }
    return cachedModelLength;
}

qint64 AssemblyModel::getModelHeight(U2OpStatus & os) {
    if(NO_VAL == cachedModelHeight) {
        // TODO: get rid of this? Use predefined max value?
        qint64 zeroAsmLen = assemblyDbis.at(0)->getMaxEndPos(assemblies.at(0).id, os); 
        //TODO: model height should be calculated as sum of all assemblies ? 
        //Or consider refactoring to getHeightOfAssembly(int assIdx, ...)
        cachedModelHeight = assemblyDbis.at(0)->getMaxPackedRow(assemblies.at(0).id, U2Region(0, zeroAsmLen), os);
        return cachedModelHeight;
    }
    return cachedModelHeight;
}

void AssemblyModel::addAssembly(U2AssemblyDbi * dbi, const U2Assembly & assm) {
    //TODO: == operator for U2Assembly
    assert(!assemblyDbis.contains(dbi));
    //assert(!assemblies.contains(assm));
    assemblyDbis.push_back(dbi);
    assemblies.push_back(assm);
}

bool AssemblyModel::hasReference() const {
    return (bool)referenceDbi;
}

void AssemblyModel::setReference(U2SequenceDbi * dbi, const U2Sequence & seq) {
    //TODO emit signal ??
    reference = seq;
    referenceDbi = dbi;
}

QByteArray AssemblyModel::getReferenceRegion(const U2Region& region, U2OpStatus& os) {
    return referenceDbi->getSequenceData(reference.id, region, os);
}

//==============================================================================
// AssemblyBrowser
//==============================================================================

AssemblyBrowser::AssemblyBrowser(AssemblyObject * o) : 
GObjectView(AssemblyBrowserFactory::ID, GObjectViewUtils::genUniqueViewName(o->getDocument(), o)), ui(0),
gobject(o), model(0), zoomFactor(INITIAL_ZOOM_FACTOR), xOffsetInAssembly(0), yOffsetInAssembly(0), 
zoomInAction(0), zoomOutAction(0), posSelectorAction(0), posSelector(0), showCoordsOnRulerAction(0)
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
    updateOverviewTypeActions();
    showCoordsOnRulerAction->setChecked(ui->getRuler()->getShowCoordsOnRuler());
    ui->installEventFilter(this);
    ui->setAcceptDrops(true);
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
        return tr("Only sequence can be added to assembly browser");
    }
    
    U2SequenceDbi * seqDbi = seqObj->asDbi();
    assert(seqDbi != NULL);
    U2OpStatusImpl status;
    U2Sequence u2SeqObj = seqDbi->getSequenceObject(seqObj->getGObjectName().toAscii(), status);
    if(status.hasError()) {
        return status.getError();
    }
    model->setReference(seqDbi, u2SeqObj);
    ui->getReferenceArea()->update();
    return "";
}

void AssemblyBrowser::buildStaticToolbar(QToolBar* tb) {
    tb->addAction(zoomInAction);
    tb->addAction(zoomOutAction);
    
    U2OpStatusImpl st;
    posSelector = new PositionSelector(tb, 1, model->getModelLength(st));
    if(!checkAndLogError(st)) {
        connect(posSelector, SIGNAL(si_positionChanged(int)), SLOT(sl_onPosChangeRequest(int)));
        tb->addSeparator();
        tb->addWidget(posSelector);
    }
    tb->addSeparator();
    updateZoomingActions(true);
    
    QToolButton * overviewScaleTypeToolButton = new QToolButton(tb);
    QMenu * scaleTypeMenu = new QMenu(tr("Scale type"), ui);
    foreach(QAction * a, overviewScaleTypeActions) {
        scaleTypeMenu->addAction(a);
    }
    overviewScaleTypeToolButton->setDefaultAction(scaleTypeMenu->menuAction());
    overviewScaleTypeToolButton->setPopupMode(QToolButton::InstantPopup);
    tb->addWidget(overviewScaleTypeToolButton);
    
    tb->addAction(showCoordsOnRulerAction);
    
    GObjectView::buildStaticToolbar(tb);
}

void AssemblyBrowser::sl_onPosChangeRequest(int pos) {
    setXOffsetInAssembly(normalizeXoffset(pos));
    ui->getReadsArea()->setFocus();
}

void AssemblyBrowser::buildStaticMenu(QMenu* m) {
    m->addAction(zoomInAction);
    m->addAction(zoomOutAction);
    GObjectView::buildStaticMenu(m);
    GUIUtils::disableEmptySubmenus(m);
}

int AssemblyBrowser::getCellWidth() const {
    return calcPixelCoord(1);
}

qint64 AssemblyBrowser::calcAsmCoordX(qint64 xPixCoord) const {
    U2OpStatusImpl status;
    qint64 modelLen = model->getModelLength(status);
    qint64 width = ui->getReadsArea()->width();
    qint64 xAsmCoord = (double(modelLen) / width * getZoomFactor() * double(xPixCoord)) + 0.5;
    return xAsmCoord;
}

qint64 AssemblyBrowser::calcAsmCoordY(qint64 pixCoord)const {
    U2OpStatusImpl status;
    qint64 modelHeight = model->getModelHeight(status);
    checkAndLogError(status);
    qint64 h = ui->getReadsArea()->height();
    return (double(modelHeight) / h * getZoomFactor() * double(pixCoord)) + 0.5;
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
    qint64 xPixelCoord = (double(width) / modelLen * double(xAsmCoord)) / getZoomFactor() + 0.5;
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
    qint64 xMax = model->getModelLength(st) - qMax((qint64)1, basesCanBeVisible());
    checkAndLogError(st);
    if(x > xMax) {
        return xMax;
    }
    return x;
}

qint64 AssemblyBrowser::normalizeYoffset(qint64 y) const {
    if(y < 0) {
        return 0;
    }
    U2OpStatusImpl st;
    qint64 yMax = model->getModelHeight(st) - qMax((qint64)1, rowsCanBeVisible());
    checkAndLogError(st);
    if(y > yMax) {
        return yMax;
    }
    return y;
}

void AssemblyBrowser::setXOffsetInAssembly(qint64 x) {
    U2OpStatusImpl st; Q_UNUSED(st);
    assert(x >= 0 && x < model->getModelLength(st));
    xOffsetInAssembly = x;
    emit si_offsetsChanged();
}

void AssemblyBrowser::setYOffsetInAssembly(qint64 y) {
    U2OpStatusImpl st; Q_UNUSED(st);
    assert(y >= 0 && y < model->getModelHeight(st));
    yOffsetInAssembly = y;
    emit si_offsetsChanged();
}

void AssemblyBrowser::setOffsetsInAssembly(qint64 x, qint64 y) {
    U2OpStatusImpl st; Q_UNUSED(st);
    assert(x >= 0 && x < model->getModelLength(st));
    assert(y >= 0 && y < model->getModelHeight(st));
    xOffsetInAssembly = x;
    yOffsetInAssembly = y;
    emit si_offsetsChanged();
}

void AssemblyBrowser::adjustOffsets(qint64 dx, qint64 dy) {
    U2OpStatusImpl status;
    qint64 modelLen = model->getModelLength(status);
    qint64 modelHeight = model->getModelHeight(status);

    xOffsetInAssembly = qMax(qMin(xOffsetInAssembly + dx, modelLen - basesVisible()), qint64(0));
    yOffsetInAssembly = qMax(qMin(yOffsetInAssembly + dy, modelHeight - rowsVisible()), qint64(0));

    emit si_offsetsChanged();
}

void AssemblyBrowser::setFocusToPosSelector() {
    posSelector->getPosEdit()->setFocus();
}

void AssemblyBrowser::sl_loadAssembly() {
    U2OpStatusImpl os;
    QHash<QString, QString> props;
    //props["url"] = "E:/BT474_dir75.sqlite";
    //props["url"] = "E:/MCF7_dir100.sqlite";
    //props["url"] = "E:/example-alignment.sqlite";
    //dbi->init(props, QVariantMap(), os);
    //checkAndLogError(os);

    //dbi->getAssemblyRWDbi()->pack(1, os);

    if(!checkAndLogError(os)) {
        sl_assemblyLoaded();
    }

    return;
    if(!checkAndLogError(os)) {
        sl_assemblyLoaded();
        U2Dbi * sdbi = AppContext::getDbiRegistry()->getDbiFactoryById("SQLiteDbi")->createDbi();
        U2OpStatusImpl os;
        QHash<QString, QString> props;
        props["url"] = "E:/test_ref.sqlite";
//#define CREATE_REF
#ifdef CREATE_REF
        QFile::remove("E:/test_ref.sqlite");
        props["create"] = "1";
#endif
        sdbi->init(props, QVariantMap(), os);
        checkAndLogError(os);
#ifdef CREATE_REF
        U2SequenceRWDbi * seqDbi = sdbi->getSequenceRWDbi();
        sdbi->getFolderDbi()->createFolder("/", os);
        checkAndLogError(os);
        U2Sequence s;
        seqDbi->createSequenceObject(s, "/", os);
        checkAndLogError(os);
        extern QByteArray example_sequence;
        seqDbi->updateSequenceData(s.id, U2Region(), example_sequence, os);
        checkAndLogError(os);

        QByteArray seq = seqDbi->getSequenceData(s.id, U2Region(0, 9996), os);
        checkAndLogError(os);
#else
        U2DataId refId = sdbi->getObjectDbi()->getObjects(U2Type::Sequence, 0, -1, os).at(0);
        checkAndLogError(os);
        U2Sequence s = sdbi->getSequenceDbi()->getSequenceObject(refId, os);
        checkAndLogError(os);
#endif
        model->setReference(sdbi->getSequenceDbi(), sdbi->getSequenceDbi()->getSequenceObject(s.id, os));
        checkAndLogError(os);
    }
}

void AssemblyBrowser::sl_assemblyLoaded() {
    assert(model);
    GTIMER(c1, t1, "AssemblyBrowser::sl_assemblyLoaded");
    checkAndLogError(dbiOpStatus);
    U2Dbi * dbi = model->getDbiHandle().dbi;
    assert(U2DbiState_Ready == dbi->getState());

    U2AssemblyDbi * assmDbi = dbi->getAssemblyDbi();

    U2DataId objectId = gobject->getDbiRef().entityId;
    U2Assembly assm = dbi->getAssemblyDbi()->getAssemblyObject(objectId, dbiOpStatus);
    checkAndLogError(dbiOpStatus);

    model->addAssembly(assmDbi, assm);
}

void AssemblyBrowser::sl_zoomIn() {
    if(!zoomInAction->isEnabled()) { // cannot perform zoom
        return;
    }
    qint64 oldWidth = basesVisible();
    int oldCellSize = getCellWidth();
    
    bool enableZoomIn = true;
    if(!oldCellSize) { 
        //if cells are not visible -> simply decrease the zoomFactor
        zoomFactor /= ZOOM_MULT;
    } else { 
        //single decreasing of the zoomFactor not always changes the cell size
        //so we have to do it in the cycle, until cells grow
        double oldZoomFactor = zoomFactor;
        int cellWidth = zoomInFromSize(oldCellSize);
        assert(cellWidth <= MAX_CELL_WIDTH);
        
        // decide if on next zoom cellWidth will increase max width
        double curZoomFactor = zoomFactor;
        if(zoomInFromSize(getCellWidth()) > MAX_CELL_WIDTH) {
            enableZoomIn = false;
        }
        zoomFactor = curZoomFactor;
    }
    
    //zooming to the center of the screen
    setXOffsetInAssembly(normalizeXoffset(getXOffsetInAssembly() + (oldWidth - basesVisible()) / 2));
    
    updateZoomingActions(enableZoomIn);
    emit si_zoomOperationPerformed();
}

int AssemblyBrowser::zoomInFromSize(int oldCellSize) {
    assert(oldCellSize > 0);
    int cellWidth = 0;
    do {
        zoomFactor /= ZOOM_MULT;
        cellWidth = getCellWidth();
    } while(oldCellSize == cellWidth);
    return cellWidth;
}

void AssemblyBrowser::sl_zoomOut() {
    if(!zoomOutAction->isEnabled()) { // cannot perform zoom
        return;
    }
    qint64 oldWidth = basesVisible();
    int oldCellSize = getCellWidth();

    if(zoomFactor * ZOOM_MULT > INITIAL_ZOOM_FACTOR) { //initial zoom factor
        zoomFactor = INITIAL_ZOOM_FACTOR;
    } else if(!oldCellSize) {
        //if cells are not visible -> simply increase the zoomFactor
        zoomFactor *= ZOOM_MULT;
    } else {
        //single increasing of the zoomFactor not always changes the cell size
        //so we have to do it in the cycle
        do {
            zoomFactor *= ZOOM_MULT;
        } while(oldCellSize && getCellWidth() == oldCellSize);
    }
    
    //zooming out of the center
    setXOffsetInAssembly(normalizeXoffset(getXOffsetInAssembly() + (oldWidth - basesVisible()) / 2));
    
    updateZoomingActions(true);
    emit si_zoomOperationPerformed();
}

void AssemblyBrowser::initFont() {
    font.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
}

void AssemblyBrowser::setupActions() {
    zoomInAction = new QAction(QIcon(":core/images/zoom_in.png"), tr("Zoom In"), this);
    connect(zoomInAction, SIGNAL(triggered()), SLOT(sl_zoomIn()));

    zoomOutAction = new QAction(QIcon(":core/images/zoom_out.png"), tr("Zoom Out"), this);
    connect(zoomOutAction, SIGNAL(triggered()), SLOT(sl_zoomOut()));
    
    QAction * linearScaleAction = new QAction(tr("Linear"), this);
    linearScaleAction->setCheckable(true);
    QAction * logScaleAction = new QAction(tr("Logarithmic"), this);
    logScaleAction->setCheckable(true);
    connect(linearScaleAction, SIGNAL(triggered()), SLOT(sl_changeOverviewType()));
    connect(logScaleAction, SIGNAL(triggered()), SLOT(sl_changeOverviewType()));
    overviewScaleTypeActions << linearScaleAction << logScaleAction;
    
    showCoordsOnRulerAction = new QAction(QIcon(":core/images/ruler.png"), tr("Show coordinates on ruler"), this);
    showCoordsOnRulerAction->setCheckable(true);
    connect(showCoordsOnRulerAction, SIGNAL(triggered()), SLOT(sl_onShowCoordsOnRulerChanged()));
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

void AssemblyBrowser::updateZoomingActions(bool enableZoomIn) {
    bool enableZoomOut = INITIAL_ZOOM_FACTOR != zoomFactor;
    zoomOutAction->setEnabled(enableZoomOut);
    if(posSelector != NULL) {
        posSelector->setEnabled(enableZoomOut);
    }
    zoomInAction->setEnabled(enableZoomIn);
}

void AssemblyBrowser::updateOverviewTypeActions() {
    AssemblyBrowserSettings::OverviewScaleType t(ui->getOverview()->getScaleType());
    overviewScaleTypeActions[0]->setChecked(t == AssemblyBrowserSettings::Scale_Linear);
    overviewScaleTypeActions[1]->setChecked(t == AssemblyBrowserSettings::Scale_Logarithmic);
}

//==============================================================================
// AssemblyBrowserUi
//==============================================================================

AssemblyBrowserUi::AssemblyBrowserUi(AssemblyBrowser * browser_) : browser(browser_) {
    setMinimumSize(300, 200);

    QScrollBar * readsHBar = new QScrollBar(Qt::Horizontal);
    QScrollBar * readsVBar = new QScrollBar(Qt::Vertical);

    overview = new AssemblyOverview(this);        
    referenceArea = new AssemblyReferenceArea(this);
    densityGraph = new AssemblyDensityGraph(this);
    ruler = new AssemblyRuler(this);
    readsArea  = new AssemblyReadsArea(this, readsHBar, readsVBar);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->setSpacing(2);
    mainLayout->addWidget(overview);

    QGridLayout * readsLayout = new QGridLayout();
    readsLayout->setMargin(0);
    readsLayout->setSpacing(0);

    //    readsLayout->addWidget(densityGraph);
    readsLayout->addWidget(referenceArea, 0, 0);
    
    readsLayout->addWidget(ruler, 1, 0);

    readsLayout->addWidget(readsArea, 2, 0);
    readsLayout->addWidget(readsVBar, 2, 1, 2, 1);
    readsLayout->addWidget(readsHBar, 3, 0);

    QWidget * readsLayoutWidget = new QWidget;
    readsLayoutWidget->setLayout(readsLayout);
    mainLayout->addWidget(readsLayoutWidget);

    setLayout(mainLayout);  
    
    connect(readsArea, SIGNAL(si_heightChanged()), overview, SLOT(sl_visibleAreaChanged()));
    connect(readsArea, SIGNAL(si_mouseMovedToPos(const QPoint&)), ruler, SLOT(sl_handleMoveToPos(const QPoint&)));
    connect(referenceArea, SIGNAL(si_mouseMovedToPos(const QPoint&)), ruler, SLOT(sl_handleMoveToPos(const QPoint&)));
    connect(browser, SIGNAL(si_offsetsChanged()), readsArea, SLOT(sl_hideHint()));
}

bool checkAndLogError(const U2OpStatusImpl & status) {
    if(status.hasError()) {
        uiLog.error(AssemblyBrowser::tr(QString("Assembly Browser -> Database Error: " + status.getError()).toAscii().data()));
    }
    return status.hasError();
}

qint64 countReadLength(qint64 realLen, const QList<U2CigarToken> & cigar) {
    return realLen + U2AssemblyUtils::getCigarExtraLength(cigar);
}

QByteArray getReadSequence(U2Dbi * dbi, const U2AssemblyRead & read, U2OpStatus & os) {
    return read->readSequence;
}



} //ns

QByteArray example_sequence("GGCACGAAAATTTTTTAAATAAAATGCATTTTCTAGATACTACCGCGCAATCACTCCCAG\
GGAGTACGTTCGGAAAAGAAAGAGCGTTTGGTGACAAGTTATACGTACATCTCTTGCGCA\
TTTTTTTAACCCGTAGAGAACGGCAGTACGCGGCCCTGCCAAGCCGGTTCAATTGACCTC\
TCAGGATAACACTTGACTCCCAATGGTGACCAGGCCCACCACCGCTACGACGTCACATCA\
TACTTCGCCGCGTACCATCCTCGCAACCTGGTCCACTGTTCCTGCACTAAAGCTGGAACT\
TATTGTCGTGCAGCCCTACGTAGGATGGAATCAATACAAAACTCCCAACCCGGTCCTAGC\
TTCATATGGAAACCGAAAGCATGCGCTACGGACTTCAACTAACGAACAATGTGTCAACGT\
GCGCTATTGGGTAGTTCATCGATACCCAGAGGTTCCGTACTCAGCGGGATCCGGCGCGGC\
CCTCAGGTATCATCCTAAATGACAAATTAAAGCGAGAAGAGATTCGCGGCGAACACGGAA\
GAGCGCTTCGGAAGGAATACGTATTAAGCACTGATCGGCCCAGTGCAGGCTATCTTCCGA\
TACCTGTCACTTACGGGCGGTTTATCCTCTCGTATTCGCCAATCTGTGGTAGTGAGACAT\
GCAACTACGATTGACCATCCCAGAAGCCGCTGTGCCTAGGCCACGGTACCCGAGAAATTC\
ATCCTCTGACTCTGTTACGTATACGCCAGTATTTAATTCTCTTTGTCCCCCGGCCTGTCC\
GCATACGGTTAAAGCGACGCGTTACATCTGTGACTAAATTCCACTGGTATAAGGGCCCCT\
GTTTAGCCTTGCAGCATCCGCACGTCCGCGCTACAACCCGACAGGCAGCGCGTCCAAGCG\
TCCTTAGAGTTACAGCGGCCACAGCACATGATGGGCGAGCCATTGGATCGGGGGAGGAAT\
CCGGTACTGGACACTATGTGGCCTGACCATAACGGGTCAATCCTCCCATGGGCGGGTGGA\
CGACGCCAATATCGTCGCTTTCCTGTCCCTTCGCCCCCTGACCTATATGCCAAGCCACAA\
GCAGGTCCCGGTGCTGCAATGATGAACCACTGCAACATAGATGCGAGCATTCGAAGAACT\
TCGATCCCAATTGTTAATCTTGGCGGGATGTATAGACTACCTGGTTGGCTGACGACGGGA\
CAAACGCGAGTCGGATCGTACCTGTGTTACTTGGGATGCGGCGTGTGCATAGTAACGGGG\
ACGTGGCGTCACTCAGTGTTGTCTTGCGGTCTCAAACTTTTGGGTGACATGGGATAAGCT\
CGAGTCACTAAGTCCAACATGATCCCAGTAACGGTGCAACTTACCACGTACCGCCAACAT\
ATGCTGTGATCGATTCGAAGCTCCAACGGGCGCGTTAGGGGGTTAGCTTGCTCGACGTCC\
ATTCTGAGTACGGGGCGCTTTCTTCTATATCGAGGTCACATGCTCTAACGCGCGCTCAGG\
TCTATTTCCGAATAGCTGGACGCATTTGATCTACCGTATGGCTACTCGGTTACGGACCCA\
GCTGCGCATGGGTATAGACATAGCACTCGGCGCTTCGGAGGTCGATCGGAGGAATCTGGG\
GCAGTTTCGTGGACAGGTACGCTGCCCCACGCTCGGCTGGCGTCCGTACTAGAGAACTAG\
CTTAAATTAATTTGCGGCATAGAGAAAGCCTGCGTACCAGAGATATGATAGCAAAACGAC\
GTCATGCGTGCTGGTTCGTTCACCGCTAACACACCCGGTCCGCAGGTTGGTAAGGCAGTA\
ACCGGATCGCCGATTAACTCCGGTCCGCCTGCTGCCCTTTTATTGCCCTCGGCGTAGCGA\
GCTTGCGAGTATCGCGCCATCATGCTTCTTCACTGTCGAAGCTGCCGAAATAAGAGTCCC\
GAAATTGTTCTTGGTTCGGATGTACGATAGTGTCGTCCCACATATGTTTGAAGGAAGGGA\
AGGTGCACAGCCTCTTGCTGAATGCTTTTCTTCTCTACTCAGTCGGATTTATGTCTCGTC\
TCTTACCACCCCGGCTGTGAGAGATCTGAATGGGCCCCCCATTTGTCTCGGAGTATGTAC\
GACGATTATGCCGGAATTAGTCAATCAAGCTGACCTTCTCCCTTTGCGGGAGTGGCAGGG\
GTAACAGTGCTGAACACTGCGATCAGCGCAAGATCGCTCACACAAAAGGCCCAATTCCGT\
GAATGCAGTCCGTTCGGCCCACCAACACAGATCTTTGTGAGTTCAGGCAGAATGGGGGTG\
AAATGTATCGATATAGTGAGCCGCCACCATCTCTTTGGGATCAACATCGCGTAGCCGCAT\
CCGAGCTCTATGGGAAGACCGCCTGCAACCTATATCCCAGCTCAATATCGTTTCTATACC\
TCGGATCACCCATAGGGAATCAGCTTACCGCAATCCGGTTATCGTTGTTGGATAGACATT\
AAAACGGTCGGTAAATGTTAACAGTATTCACAAACGTTAATCGCAAGAGCGTGAAGATCA\
CATCGGGGTGGATCTCTCCGTGAAAGTTTGTCGAAGAACGGATTTTTTATGCGTGTATCG\
GCCGCTACTAATGGTAGCCGCAGTCCACTTGCCCAGCTCCAATACCTTTGTCCACGGTAC\
CAGAGTTCGCGTAAAGGACGGCCTTATCTGTGAGTCTTCTAGTAAGAAGGTTTACCAAGC\
TAACAACCCGGCATAGTAAAAGAGAACTCAGGAGACTCTTCAATAACCCTCTGATTAGTC\
TCGGTAAACGAGAGATTCCGCGTGATCTTGAGTTTGTATCTCTACAGCTAATCTTTTGTC\
GGACTTTCTATTCTATGTCGAGCGTACGTTCGAGTAACAGGACGACCTGAACATCGCAAA\
CCCGATGAAGAGATCTGGTAGAAACACTGGTACACGTTTGTGCACAGTACCTGTCCAGGA\
AACTCCCTGTACAGGTGAACCTGGCCAGACTTTCGGATAGACCTTGCCATCTTATATGGG\
TATAGTCTTGGCCACCCTAGAACCTCGGACCTCGCGCGATACGAACTCGATCATCAGCGT\
GTTAAACTTGAGCCTGCGCTGGGCAAATCCATGGTTCTAGGCCTATCGATAAGATATCAT\
CGCCCACTGACCCCTACGCTTCAAGTATTTACACATATACGAGCAAGGGGAAAACCTTGC\
GGAAGTCACGTATTCGATGCGTATAGCGACTAAGGTATGAATCATACGTTGCGCGTCGCG\
ACTCAGAGAAAGCGAGTAGGCGTAGGATTAGGTGGGCTCGTTAGCATTCTTCAAAACCTT\
CCTAATCACATGATGCGATTGTACAGTTGACAGCACCCCATGTCATATGTCAAGGTCGCA\
GGTCATGTCCCACTTGAGCTGTTACAGCAGGAGATCAGCGTACCCAAAACGGCGTCTGAA\
ATTTGTAGAAGCAGACCAGAAAGCAAATGAAGACGACTGCGGTAACGCTGCGCCCTATGT\
GCCCTCGGTCACGGTCGACAATATGAAGCATTGAATCTTTTCATCACTCCTGTTCAGCGA\
GCTTCCCGCTGGGCACTTAATCAGCCTGCGGACTATCCTTATCTAGCTACCTAAAATCGG\
TTCACCAATGCAATGTCTTGCCGCTATCCCTTATTGAATGACGGCTTGTGGTGCCTAGTA\
GGCTTCTTGTGGGCCGAATGTAGAGAGTCATCGCTTCATGCTGTGTAGTTGGCACCTTAC\
CCAGCGGGTGGTCTATCTAAGGGGGGCGCAGCCTCAGCCGAAACGACGTAGCCGTCGAAC\
CATATAGTGATATCGAAGATCGCTTAGGCTCCGCCACGACTAGTACCTCGAACTTTTGAA\
ACAAGGTGTGTCATTGGCTTCTAATAGGCGGTAAGTGTCCCGGATATAGAACCAGACACC\
CGTCGTTTTTCCGCGCGGTAAAGCTCTAGCTTAGAGTCCTGCCAGGGGATTTACTGATGA\
AACCCCATTAGCAACATGACTCTCGCTCTACTTCGGACCAGTCTAATTCGCGTTAACCAA\
CCTGACGACCACAATGGCGGATAGACCCGGGGTTTAGCCACTGCATAGATCGCATCACCT\
AGTTGGAAAGGTGTGAGTCGGGGAGATCGCTCGACCAGTTCCGTCTACTGTTACGGGGCT\
AACCGTTTTCTAAGGAATCTAGTAATAGGACGCGGCCGCAAGTAGGTCTGATGTTAGCGT\
ATACGTATTGTCGGGCGACCACCCAGCGATATATGTAAGGGGTGAAAAGCAAAATGAGAG\
CACAGATGTGCGACTCCGACGTCGCTTAGCAGAGAATGGCAAGAAAATAGTGGTTAAGAC\
GGCACGAATTATCAACGTCCTGTCGGATTATTCCGACTGTTTCCATGTAGACCATGACTT\
ATCAATACACACCTACTTGTCGAAAGTCTACGCGACGTACTGATTCGGTCATTGAGCCCC\
GTGGATACCACACCACCCTCAGCTATGGGCTTTGTCGGTCGGGTCTGGAGACTATTCCGC\
AAGAAATAGGGAAGTATATGACCTCTAGCATCTCGAGTACGCTGTTCAAGGTTAAAAGGT\
GAAAGCTGGGACGGGTCCAATCGATTGCGGCGATAAAGACTAGGACATCCACGATCTCAA\
GCGACTGACGCTGCACGTACCCGTCCGCGAACTCAAATGACTCACGCATGATAATCCGCT\
TAAAGTGCAACAGGCTTGACCATCTACATTTGCAAGCAACACTGACACGTCTTCCGATGA\
AGATCCTTTGTAGCTGTTCTTGCGGTGCGCGAATTTGAGGACTGCCGGTCCGAACTTCTG\
GTGATCCTACGGATAGACATGGCAATAGACGCCCCATCCAAATTGTTCCTGCAAATAGCG\
ATAGGCGATACGAGCTTTTTGATGCTTGGCTATCACAATCGAGACCTACCCAGAGGTCCG\
CTCGGTGACGATTGTGAATACAACTTTTGTGTGTTTCCATCACTGGGGGGGCTGCAGCAG\
CCGCTTAAGGGGCATAAGCTTGTATTTTCTCTAAGGCGGGTATCATGCCATAAGGTGATA\
AGCGCACCCCCGCCCTGCCGATACATGGGCAGTTGTCTAATGTGCACTGAAGCGTTTCTA\
TTTTAACGCTGGCTAGTGTAAATGATCGGAGAAGGAGGATGTGCAAACCACCTTGGCCTT\
CAAGCCCGGGATAAGTATCAGAGGCTAACTGAGGTCTTTGGCCAGAACTGGATATACCGT\
ACCGGCACTGCCAAAAGGCTCTCATCAATGAGAGTTGTGAGTCAATGCTCAATGCTGCGG\
CGTAGTTCTTGATGTATTCAACAATCGTAATAATGTATCGTACCATGTTGAATCTATATC\
TTCATCAACGCCAATGCGCCATACACTGACGCAGCGAGCCGCCTTCATACTTTGTTAGCA\
CGGCTCCACACATATGGTCCGACTCTAATGAAGATCCTCATGTTATATCGGCAGTGGGTT\
GATCAATCCACGTGGATAGATGCCCCCCGAGCCACGAAAACCGGGCACTTGTAACTCCAG\
GCTCAACTATGCGACGGACGGGGTCTTGTTACCGCCACTTGAGACTGTAAAGAGATATCG\
TCCCTCTGTGAATCCTCAAGAATTCCAAGCCTAGTCTTGGCGGTGGTTACCGCCTCCTTA\
TACTCATTATGAAGACGGTGCGGGATCGCCCCTGGTATTTAGGTCTACGCTAACAACCAC\
TTCTTTGGGGACGCCTCGGGGACTGACAGATAATGATCATGGCCCAAGCTATATACGTGT\
GTCTCGGGGCCTCTCCGTATACGATTTAGGCCGAGGACTTGGAATACGGCTTGGCAACCC\
CACGGCATGGTATCCAAGGGAAGAGACCGATAAATTACCCGCGCTGTCGAACTGCGATAA\
GATTTTTTATTCAGCGTCAATACGAATAGCTTATCGGAGGAAGAGTTAAAGTTTGCTAGC\
GAGTTTTCTTGACTCAGTGGGCGGGGGTACAATATGAGCTCTCCTGTTAATCCAATCTTT\
AGAAGGAATTTGTGCCCTCTTGTCAAATACCTGTGAGGTCTAACGAATATTGGCACATTG\
TGGCTGCCTCGCCGGCCGCTTCCTCAGATTAGGTACGCCGGAATCGGTCAAAGTCCTTCG\
CCTTAGTTCCGCATAACTACTTCAGGGAAGCTACAGTCCCACTTCGGATGGTACAACCCC\
TTACCTACTCGGATATAAGGAGGGAAAACATCCGAATTTCGACCAGGTCGTCGGGCGGCT\
TACGGTCCGATGGAGAATTGGCGAGCTGATTTGGGAAAGGAAGTTATCTGACAATAAAGG\
CCTAGCCAATGTTTCAATTTATTGATCCCGAGGTAAACATCAGCGGCTACATGTACCCTT\
TGTGCGTAAGATGCATTTAGAAGCCTGAGCTATGCATGAATGAGAGCTTATGAGCGCGGT\
TTCTTCCACGTGATCTTGAGGCTCATCGGACGCGGGCTCTGCAGCCTTGGGGCAAAGGAT\
GATCCAGACTACGATACTTGTGCTGTTTCTCCAGCGGACACAACTATAACGATCAAACTA\
CGTAAAGATTGAACCCCTCGTTGATAGTGTAGCTCCTCTGAGACCCTCTGTAACATAACC\
CTTGACGCGAAGTTCGGGTCGGCCATCACAAGTTGCGTATGGCAGGCTTCCCTACGTACT\
CGGGGTAGGGAGGTCTCATGGAAAGACAATCGAGCGCCACTTACCGGCTGACCGTACTTA\
TTAAAAGATTGATGACAAAGTGATGCGGGAAACTTTGGAGGGATCACTCATGTCACGAAG\
AAAGTTGTTCATAAACTGACATGCATAACTCGCCACGTTGGGGCTATCCAAGCATGAGCT\
CGGATGTTTCGGCAACGTTGAAGGGGAATACCGGGCCGCCACTGATCGCCAGGGCTGTAG\
AATCCTGCAATATGCTTAAACACCTCCATTAGCTGTGCCGAGGCGGGCTCCAGTAAGGCA\
ACAAAAAGTCGATGTCATCTAAGCGTCGGCCGGACCTGGGCGCACCTTGCGTTTTTCAAC\
GAAACTGCCTATGGGTCGTTAGTAGCGACACACTCTAGCGCTATGATGTTTCGCTAGGAA\
TGTCCCAAATTACTAGCGGGTAATCTGAGAGGGCATCACGGTGGGATGTCCGTCGGAGCG\
TGATGAATGCAACCCTAAAGCCCTGTGCTACTCGCACGAACAGGAGCACCTAACCTGCGA\
GACTTGCTGATTTCGAGCCCCGGTCGCCCCCGGGGGTGATACTCGTCCATAGTATTAGGC\
ATCCATTAGGGCGATGTTTAGAGCATCCATTAGCGGATAGTTTCATATGTGGGTATTACA\
GTCTGAATCGAGCACCGCACGATAGACGCTATAGTACTTAGATATTACGACGTACAGTGT\
AGTCTCTAGGACTCCTCTGATGGCTCCCACTACCCCGCGTCTACACGCACTCTTCTCGCT\
CCAGGGTACAAATAAATATGTATCTTCGCCCCGGTCGGCGCGGGACTGGACACCCAGGGA\
TAGCTCCGGATCAGCGAGCGCAGGATGCAATATTGGAGTTCTGATGAACAGCTCGGTACG\
CCGGGTCTAGTGGTTTCACGCTGAGATAGACTACTCCACAAGCTAATCCTTCACGGTCGC\
CTCCTTACCAACTATCGTTTGCCTATATCAGTGGTCGATGTTACCATAAGTGAGGTCACG\
GGACACGGACTAAGGTTAAATGAATAGGAAATACGGGAGTCTCTCCAGCCCTTATGGGCC\
GCCATCGCCTCTGCGTCAGTCCGTCCAACAGCTTTGGAGGCCTTCTGACAGCCGCTGGAT\
GTACTGCTACGCCTACCGGAGTGGTCTCGCGCATACGCACCAACTTTTTCCTTTTGACCA\
GTCATACATGAACGCAAGACGAATTCGATTGGCCTGGAATCCATTAGTGCTGCTCAGTGT\
TCTGCCGCGCGCACTGCGAGTGCATGCGAAAGTGCGGCCGTTCGATAGGGCTCTGTGTAA\
GTTCTCACTGATCTTGTCCGTCCTACACCACATTCTTATAAGTTATGTGTCATCCGAGAC\
GCAGCCCTGGACGGAATACATGTTGTATTTGTCGACGACCGTAATCCGAACGGACTTGAC\
CGAGGTCCTGTGCCCTGCATTTTTCTGCCCTACTTCCATGACTCTTCTATAGCGTAGGCC\
GCTTGTGGCGCGGCCACTAAAAACGCGTGCCAGCCGTAATTCCACCGGGATGACGCGACT\
TTACATGCCCCGAAGCGTTATATCCGCGTAGCTTCACTGTGGATAGGGTTTTGATCGCGA\
AGGAATCTCAATTCTAATCACAATATTATTCCTAAGGCAATACTCGGGCCACGCAATGAA\
ACTTGGCGGGATTATCGTAAATAGGATGAATCGATTTGTGCGAGATAACGCGCTGAAAAC\
TGAAACCATACCCTCTAATACGGTCCATTGATGGGTAGTATTTGTTGACCTGCGATTTGT\
CACGTTCGGGCCGTGGAATTTTTTTTCGGCGCATTTTATCCCGCGTACCTGGGCGCCGAT\
TTCCTAGCGACTATTTCGCAAGATCCACGTGTTCGTCACAAGGAGATAGCTCCAGCCGTG\
ATGCCACACCGGCACATTTTCTGCACGCGTGAGCCGTATATATCCAGTTAATTTTGGAAT\
CCGTAAGGTCGTCGCTGTCATGACTAGTCGTCAGGGACTCAATCAATCCCCGGTATCTCA\
AAACTCTGCAGGAGTTCCCAGTTATGGCACCGTTCTCAATTTGACGTAGATGATGCGCGA\
TTACGTACCGTACCCTTGGTGAAAACCAACCTGCTTCGCGCCGTAATGTCAAGCAACGGA\
CAAGCTGCGCTGGGTCGGCAAGACAGTCCGCGCTATTCACCGAGTGAAGCCTTAAGCTCA\
TGCAGTGAAAAGATGCGTCTAGGACGTGTCCATCTCAGAACTAGCTGTGCAGGTGAAGCC\
CAGCTTTAGGTGTCGTGACGGTCAAGATATCCAGTAAATGCGCCGACTAGTCCGGGCAAT\
ACGGTTTGTGATGCTACCTGATCTTGAGGACATTTAGTACACGATTCGGATTGGCCGGGT\
CGTGGATTATCCTTGTTAAAAACAGGACTCTATCAGATACCAGACAGCGAAAATTAACGT\
CGGCGCTGAGAACGAGCGTCATCGGGAGCACGCGGCGAAACGCTCCAATTACTAATTTCC\
TAATATGATATCGCCCGCCACCAGCGGGTAAGAGGCCAGGTATGACTTCGTTATTCGCCA\
TTGACTCAGCGCTTCAAACCGGTTGCTTGGTGAGAGTCAGCATTGGCGTCGTTCTGGGAG\
CTCCTCCGGTCATATCCTCATGCATCGCTGTACATCTCTATTATCCCTCGGCTTGGGATC\
CTACTTCGCCTTCGGGAACTTGAGCTGCGGGCCCAGGCAATGCTGGGGCACATTTACTAG\
GAGATCGATATTCATGCTGGCCGCGCCATAATTATGTCACTAGACACCATTCAGTTGCAC\
TTTGCTAAAAAGATTTGAGTTACTACTAATCTGTAAGATCATGCCTCAGCCCCGATCACC\
ACGGCACCCTATTGCGCGGATGACCAATCCCATGGTTTGTTTCGAACCAACTATCAAACC\
TAAGCCAAAATCGAGTTAGCTCCGCCCGGTACGACAATAACCCGAACCAACTTCTAGACC\
AACCTGACGCGATTCCGTGGTACTGTACGCTCAGAAACACTCCCTCCGGAGAAGCAATCC\
CCTATTCCACTGCCCATTGTCTACGGGTCCGAATTCATGCGCGGAGAAACCCGGTAGCGA\
CTGTTTGCAGTTAAAGGTGGCGCAAACGACATTAATTATATTAATTCCGAGGCGACGATG\
TTCTTAGATTCGGTCTATTGCAATACTAAGAGTTCGAAGCAGGCCGTCGCAGGGAACGAA\
CCACCTACTTGATCTACACTTGATTGAGCCAGCATGTAATTGCTCTGAACAGTATCATTA\
CAAACGGCTCTAAACTTCATAATGAGGTAACATCCGAAGG\
");