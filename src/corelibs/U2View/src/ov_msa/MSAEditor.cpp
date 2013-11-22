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

#include "MSAEditor.h"
#include "MSAEditorTasks.h"
#include "MSAEditorFactory.h"
#include "MSAEditorState.h"
#include "MSAEditorConsensusArea.h"
#include "MSAEditorOffsetsView.h"
#include "MSAEditorSequenceArea.h"
#include "MSAEditorNameList.h"
#include "MSAEditorStatusBar.h"
#include "MSAEditorUndoFramework.h"
#include "MSAEditorDataList.h"
#include "ExportHighlightedDialogController.h"

#include <ov_phyltree/TreeViewerTasks.h>

#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/Counter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Settings.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/MSAUtils.h>

#include <U2Algorithm/PhyTreeGeneratorTask.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>
#include <U2Algorithm/MSADistanceAlgorithm.h>
#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include <U2Algorithm/PairwiseAlignmentTask.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/ExportImageDialog.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/OptionsPanel.h>
#include <U2Gui/OPWidgetFactoryRegistry.h>

#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QResizeEvent>
#include <QtGui/QToolBar>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QFontDialog>
#include <U2Gui/GScrollBar.h>
#include <QtSvg/QSvgGenerator>


#include <U2View/CreatePhyTreeDialogController.h>
#include <U2View/TreeViewer.h>
#include <U2View/MSAEditorMultiTreeViewer.h>
#include <U2View/MSAEditorTreeViewer.h>
#include <U2View/TreeOptionsWidgetFactory.h>
#include <U2View/UndoRedoFramework.h>
#include <U2View/MSAColorScheme.h>

#include <QtCore/QEvent>


namespace U2 {

#define MIN_FONT_SIZE 8
#define MAX_FONT_SIZE 18
#define MIN_COLUMN_WIDTH 1

#define SETTINGS_ROOT QString("msaeditor/")
#define SETTINGS_FONT_FAMILY    "font_family"
#define SETTINGS_FONT_SIZE      "font_size"
#define SETTINGS_FONT_ITALIC    "font_italic"
#define SETTINGS_FONT_BOLD      "font_bold"
#define SETTINGS_ZOOM_FACTOR    "zoom_factor"

#define DEFAULT_FONT_FAMILY "Verdana" 
#define DEFAULT_FONT_SIZE 10
#define DEFAULT_ZOOM_FACTOR 1.0f

/* TRANSLATOR U2::MSAEditor */ 

const float MSAEditor::zoomMult = 1.25;

MSAEditor::MSAEditor(const QString& viewName, GObject* obj)
: GObjectView(MSAEditorFactory::ID, viewName), ui(NULL), treeManager(this) {

    msaObject = qobject_cast<MAlignmentObject*>(obj);

    if (0 == msaObject->getLength() || 0 == msaObject->getNumRows()) {
        msaObject->crop(U2_REGION_MAX, QSet<QString>());
    }

    objects.append(msaObject);
    onObjectAdded(msaObject);

    requiredObjects.append(msaObject);
    GCOUNTER(cvar,tvar,"MSAEditor");

    U2OpStatus2Log os;
    msaObject->setTrackMod(TrackOnUpdate, os);

    zoomInAction = new QAction(QIcon(":core/images/zoom_in.png"), tr("Zoom In"), this);
    zoomInAction->setObjectName("Zoom In");
    connect(zoomInAction, SIGNAL(triggered()), SLOT(sl_zoomIn()));

    zoomOutAction = new QAction(QIcon(":core/images/zoom_out.png"), tr("Zoom Out"), this);
    zoomOutAction->setObjectName("Zoom Out");
    connect(zoomOutAction, SIGNAL(triggered()), SLOT(sl_zoomOut()));

    zoomToSelectionAction = new QAction(QIcon(":core/images/zoom_reg.png"), tr("Zoom To Selection"), this);
    zoomToSelectionAction->setObjectName("Zoom To Selection");
    connect(zoomToSelectionAction, SIGNAL(triggered()), SLOT(sl_zoomToSelection()));

    resetFontAction = new QAction(QIcon(":core/images/zoom_whole.png"), tr("Reset Zoom"), this);
    resetFontAction->setObjectName("Reset Zoom");
    connect(resetFontAction, SIGNAL(triggered()), SLOT(sl_resetZoom()));

    changeFontAction = new QAction(QIcon(":core/images/font.png"), tr("Change Font"), this);
    changeFontAction->setObjectName("Change Font");
    connect(changeFontAction, SIGNAL(triggered()), SLOT(sl_changeFont()));

    buildTreeAction = new QAction(QIcon(":/core/images/phylip.png"), tr("Build Tree"), this);
    buildTreeAction->setObjectName("Build Tree");
    connect(buildTreeAction, SIGNAL(triggered()), SLOT(sl_buildTree()));

    Settings* s = AppContext::getSettings();
    zoomFactor = DEFAULT_ZOOM_FACTOR;
    font.setFamily(s->getValue(SETTINGS_ROOT + SETTINGS_FONT_FAMILY, DEFAULT_FONT_FAMILY).toString());
    font.setPointSize(s->getValue(SETTINGS_ROOT + SETTINGS_FONT_SIZE, DEFAULT_FONT_SIZE).toInt());
    font.setItalic(s->getValue(SETTINGS_ROOT + SETTINGS_FONT_ITALIC, false).toBool());
    font.setBold(s->getValue(SETTINGS_ROOT + SETTINGS_FONT_BOLD, false).toBool());
    calcFontPixelToPointSizeCoef();

    if ( (font.pointSize() == MIN_FONT_SIZE) && (zoomFactor < 1.0f) ) {
        resizeMode = ResizeMode_OnlyContent;
    } else {
        resizeMode = ResizeMode_FontAndContent;
    }
    
    pairwiseAlignmentWidgetsSettings = new PairwiseAlignmentWidgetsSettings;
    pairwiseAlignmentWidgetsSettings->customSettings.insert("alphabet", msaObject->getAlphabet()->getId());

    exportHighlightedAction = new QAction(tr("Export highlighted"), this);
    exportHighlightedAction->setObjectName("Export highlighted");
    connect(exportHighlightedAction, SIGNAL(triggered()), this, SLOT(sl_exportHighlighted()));
    exportHighlightedAction->setDisabled(true);

    updateActions();
}

int MSAEditor::getRowHeight() const {
    QFontMetrics fm(font);
    return fm.height() * zoomMult;
}

int MSAEditor::getColumnWidth() const {
    QFontMetrics fm(font);
    int width =  fm.width('W') * zoomMult;
    
    width = (int)(width * zoomFactor);
    width = qMax(width, MIN_COLUMN_WIDTH);
    
    return width;
    
}

void MSAEditor::sl_zoomIn() {

    int pSize = font.pointSize();
    
    if (resizeMode == ResizeMode_OnlyContent) {
        zoomFactor *= zoomMult;
    } else if ( (pSize < MAX_FONT_SIZE) && (resizeMode == ResizeMode_FontAndContent) ) {
        font.setPointSize(pSize+1);
        setFont(font);
    }

    bool resizeModeChanged = false;

    if (zoomFactor >= 1) {
        ResizeMode oldMode = resizeMode;
        resizeMode = ResizeMode_FontAndContent;
        resizeModeChanged = resizeMode != oldMode;
        zoomFactor = 1;
    }
    updateActions();
    
    emit si_zoomOperationPerformed(resizeModeChanged);
}

void MSAEditor::sl_zoomOut() {
    int pSize = font.pointSize();

    bool resizeModeChanged = false;

    if (pSize > MIN_FONT_SIZE) {
        font.setPointSize(pSize-1);
        setFont(font);
    } else {
        SAFE_POINT(zoomMult > 0, QString("Incorrect value of MSAEditor::zoomMult"),);
        zoomFactor /= zoomMult;
        ResizeMode oldMode = resizeMode;
        resizeMode = ResizeMode_OnlyContent;
        resizeModeChanged = resizeMode != oldMode;
    }
    updateActions();

    emit si_zoomOperationPerformed(resizeModeChanged);
}

void MSAEditor::sl_zoomToSelection()
{
    ResizeMode oldMode = resizeMode;
    int seqAreaWidth =  ui->seqArea->width();
    MSAEditorSelection selection = ui->seqArea->getSelection();
    if (selection.isNull()) {
        return;
    }
    int selectionWidth = selection.width();
    float pixelsPerBase = (seqAreaWidth / float(selectionWidth)) * zoomMult;
    int fontPointSize = int(pixelsPerBase / fontPixelToPointSize);
    if (fontPointSize >= MIN_FONT_SIZE) {
        if (fontPointSize > MAX_FONT_SIZE) {
            fontPointSize = MAX_FONT_SIZE;
        }
        font.setPointSize(fontPointSize);
        setFont(font);
        resizeMode = ResizeMode_FontAndContent;
        zoomFactor = 1;
    } else {
        if (font.pointSize() != MIN_FONT_SIZE) {
            font.setPointSize(MIN_FONT_SIZE);
            setFont(font);
        }
        zoomFactor = pixelsPerBase / (MIN_FONT_SIZE * fontPixelToPointSize); 
        resizeMode = ResizeMode_OnlyContent;
    }
    ui->seqArea->setFirstVisibleBase(selection.x());
    ui->seqArea->setFirstVisibleSequence(selection.y());

    updateActions();

    emit si_zoomOperationPerformed(resizeMode != oldMode);
}

void MSAEditor::sl_buildTree() {
    treeManager.buildTreeWithDialog();
}

void MSAEditor::onObjectRenamed(GObject*, const QString&) {
    // update title
    OpenMSAEditorTask::updateTitle(this);
}

static void saveFont(const QFont& f) {
    Settings* s = AppContext::getSettings();
    s->setValue(SETTINGS_ROOT + SETTINGS_FONT_FAMILY, f.family());
    s->setValue(SETTINGS_ROOT + SETTINGS_FONT_SIZE, f.pointSize());
    s->setValue(SETTINGS_ROOT + SETTINGS_FONT_ITALIC, f.italic());
    s->setValue(SETTINGS_ROOT + SETTINGS_FONT_BOLD, f.bold());
}

void MSAEditor::setFont(const QFont& f) {
    int pSize = f.pointSize();
    font = f;
    calcFontPixelToPointSizeCoef();
    font.setPointSize(qBound(MIN_FONT_SIZE, pSize, MAX_FONT_SIZE));
    emit si_fontChanged(f);
    saveFont(font);
}

void MSAEditor::setFirstVisibleBase(int firstPos) {

    if (ui->seqArea->isPosInRange(firstPos)) {
        ui->seqArea->setFirstVisibleBase(firstPos);
    }
}

int MSAEditor::getFirstVisibleBase() const {

    return ui->seqArea->getFirstVisibleBase();
}

void MSAEditor::sl_changeFont() {
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, font, widget, tr("Select font for alignment"));
    if (!ok) {
        return;
    }
    setFont(f);
    updateActions();
}

void MSAEditor::sl_resetZoom() {
    QFont f = getFont();
    f.setPointSize(DEFAULT_FONT_SIZE);
    setFont(f);
    zoomFactor = DEFAULT_ZOOM_FACTOR;
    ResizeMode oldMode = resizeMode;
    resizeMode = ResizeMode_FontAndContent;
    emit si_zoomOperationPerformed(resizeMode != oldMode);
    
    updateActions();
}

MSAEditor::~MSAEditor() {
    delete pairwiseAlignmentWidgetsSettings;
}

void MSAEditor::buildStaticToolbar(QToolBar* tb) {
    tb->addAction(zoomInAction);
    tb->addAction(zoomOutAction);
    tb->addAction(zoomToSelectionAction);
    tb->addAction(resetFontAction);
    tb->addAction(changeFontAction);
    tb->addAction(buildTreeAction);
    tb->addAction(saveScreenshotAction);
    tb->addAction(alignAction);

    toolbar = tb;

    GObjectView::buildStaticToolbar(tb);
}

void MSAEditor::buildStaticMenu(QMenu* m) {
    addLoadMenu(m);
    addCopyMenu(m);
    addEditMenu(m);
    addAlignMenu(m);
    addTreeMenu(m);
    addStatisticsMenu(m);
    addViewMenu(m);
    addExportMenu(m);
    addAdvancedMenu(m);
    //addSNPMenu(m);

    GObjectView::buildStaticMenu(m);

    GUIUtils::disableEmptySubmenus(m);
}


void MSAEditor::addCopyMenu(QMenu* m) {
    QMenu* cm = m->addMenu(tr("Copy"));
    cm->menuAction()->setObjectName(MSAE_MENU_COPY);
}

void MSAEditor::addEditMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("Edit"));
    em->menuAction()->setObjectName(MSAE_MENU_EDIT);
}

void MSAEditor::addExportMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("Export"));
    em->menuAction()->setObjectName(MSAE_MENU_EXPORT);
    em->addAction(saveScreenshotAction);
    em->addAction(saveSvgAction);
    em->addAction(exportHighlightedAction);
    if(ui->getSequenceArea()->getCurrentHighlightingScheme()->getFactory()->isRefFree()){
        exportHighlightedAction->setDisabled(true);
    }else{
        exportHighlightedAction->setEnabled(true);
    }
}

void MSAEditor::addViewMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("View"));
    em->menuAction()->setObjectName(MSAE_MENU_VIEW);
    if (ui->offsetsView != NULL) {
        em->addAction(ui->offsetsView->getToggleColumnsViewAction());
    }
}

void MSAEditor::addAlignMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("Align"));
    em->setIcon(QIcon(":core/images/align.png"));
    em->menuAction()->setObjectName(MSAE_MENU_ALIGN);
}

void MSAEditor::addTreeMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("Tree"));
    //em->setIcon(QIcon(":core/images/tree.png"));
    em->menuAction()->setObjectName(MSAE_MENU_TREES);
    em->addAction(buildTreeAction);
}

void MSAEditor::addAdvancedMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("Advanced"));
    em->menuAction()->setObjectName(MSAE_MENU_ADVANCED);
}

void MSAEditor::addStatisticsMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("Statistics"));
    em->setIcon(QIcon(":core/images/chart_bar.png"));
    em->menuAction()->setObjectName(MSAE_MENU_STATISTICS);
}

void MSAEditor::addLoadMenu( QMenu* m ) {
    QMenu* lsm = m->addMenu(tr("Add"));
    lsm->menuAction()->setObjectName(MSAE_MENU_LOAD);
}

Task* MSAEditor::updateViewTask(const QString& stateName, const QVariantMap& stateData) {
    return new UpdateMSAEditorTask(this, stateName, stateData);
}

QVariantMap MSAEditor::saveState() {
    return MSAEditorState::saveState(this);
}

QWidget* MSAEditor::createWidget() {
    Q_ASSERT(ui == NULL);
    ui = new MSAEditorUI(this);

    QString objName = "msa_editor_" + msaObject->getGObjectName();
    ui->setObjectName(objName);

    connect(ui , SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sl_onContextMenuRequested(const QPoint &)));
    saveScreenshotAction = new QAction(QIcon(":/core/images/cam2.png"), tr("Export as image"), this);
    saveScreenshotAction->setObjectName("Export as image");
    connect(saveScreenshotAction, SIGNAL(triggered()), ui, SLOT(sl_saveScreenshot()));
    
    saveSvgAction = new QAction(tr("Export as SVG"), this);
    saveSvgAction->setObjectName("Export as SVG");
    connect(saveSvgAction, SIGNAL(triggered()), ui, SLOT(sl_saveSvgImage()));

    alignAction = new QAction(QIcon(":core/images/align.png"), tr("Align"), this);
    alignAction->setObjectName("Align");
    connect(alignAction, SIGNAL(triggered()), this, SLOT(sl_align()));

    setAsReferenceSequenceAction = new QAction(tr("Set this sequence as reference"), this);
    setAsReferenceSequenceAction->setObjectName("set_seq_as_reference");
    connect(setAsReferenceSequenceAction, SIGNAL(triggered()), SLOT(sl_setSeqAsReference()));

    unsetReferenceSequenceAction = new QAction(tr("Unset reference sequence"), this);
    unsetReferenceSequenceAction->setObjectName("unset_reference");
    connect(unsetReferenceSequenceAction, SIGNAL(triggered()), SLOT(sl_unsetReferenceSeq()));

    optionsPanel = new OptionsPanel(this);
    OPWidgetFactoryRegistry *opWidgetFactoryRegistry = AppContext::getOPWidgetFactoryRegistry();

    QList<OPFactoryFilterVisitorInterface*> filters;
    filters.append(new OPFactoryFilterVisitor(ObjViewType_AlignmentEditor));

    QList<OPWidgetFactory*> opWidgetFactories = opWidgetFactoryRegistry->getRegisteredFactories(filters);
    foreach (OPWidgetFactory *factory, opWidgetFactories) {
        optionsPanel->addGroup(factory);
    }

    qDeleteAll(filters);

    connect(ui, SIGNAL(si_showTreeOP()), SLOT(sl_showTreeOP()));
    connect(ui, SIGNAL(si_hideTreeOP()), SLOT(sl_hideTreeOP()));
    sl_hideTreeOP();

    treeManager.loadRelatedTrees();

    initDragAndDropSupport();
    return ui;
}

int MSAEditor::getAlignmentLen() const {
    return msaObject->getLength();
}

int MSAEditor::getNumSequences() const {
    return msaObject->getNumRows();
}

bool MSAEditor::isAlignmentEmpty() const {
    return getAlignmentLen() == 0 || getNumSequences() == 0;
}

void MSAEditor::sl_onContextMenuRequested(const QPoint & pos) {
    Q_UNUSED(pos);

    QMenu m;

    addLoadMenu(&m);
    addCopyMenu(&m);
    addEditMenu(&m);
    addAlignMenu(&m);
    addTreeMenu(&m);
    addStatisticsMenu(&m);
    addViewMenu(&m);
    addExportMenu(&m);
    addAdvancedMenu(&m);

    m.addSeparator();
    snp.clickPoint = QCursor::pos( );
    const QPoint nameMapped = ui->nameList->mapFromGlobal( snp.clickPoint );
    const qint64 hoverRowId = ( 0 <= nameMapped.y( ) )
        ? ui->nameList->sequenceIdAtPos( nameMapped ) : MAlignmentRow::invalidRowId( );
    if ( ( hoverRowId != getReferenceRowId( )
        || MAlignmentRow::invalidRowId( ) == getReferenceRowId( ) )
        && hoverRowId != MAlignmentRow::invalidRowId( ) )
    {
        m.addAction( setAsReferenceSequenceAction );
    }
    if ( MAlignmentRow::invalidRowId( ) != getReferenceRowId( ) ) {
        m.addAction( unsetReferenceSequenceAction );
    }
    m.addSeparator();

    emit si_buildPopupMenu(this, &m);

    GUIUtils::disableEmptySubmenus(&m);

    m.exec(QCursor::pos());
}

const QRect& MSAEditor::getCurrentSelection() const {
    return ui->seqArea->getSelection().getRect();
}

void MSAEditor::updateActions() {
    zoomInAction->setEnabled(font.pointSize() < MAX_FONT_SIZE);
    zoomOutAction->setEnabled( getColumnWidth() > MIN_COLUMN_WIDTH );
    zoomToSelectionAction->setEnabled( font.pointSize() < MAX_FONT_SIZE);
    changeFontAction->setEnabled( resizeMode == ResizeMode_FontAndContent);
}

void MSAEditor::calcFontPixelToPointSizeCoef() {
    QFontInfo info(font);
    fontPixelToPointSize = (float) info.pixelSize() / (float) info.pointSize();
    
}

void MSAEditor::copyRowFromSequence(U2SequenceObject *seqObj, U2OpStatus &os) {
    MSAUtils::copyRowFromSequence(msaObject, seqObj, os);
    msaObject->updateCachedMAlignment();
}

void MSAEditor::sl_onSeqOrderChanged( QStringList* order ){
    msaObject->sortRowsByList(*order);
}

void MSAEditor::sl_showTreeOP() {
    OptionsPanelWidget* opWidget = dynamic_cast<OptionsPanelWidget*>(optionsPanel->getMainWidget());
    if(NULL == opWidget) {
        return;
    }
    
    QWidget* addTreeGroupWidget = opWidget->findOptionsWidgetByGroupId("OP_MSA_ADD_TREE_WIDGET");
    if(NULL != addTreeGroupWidget) {
        addTreeGroupWidget->hide();
        opWidget->closeOptionsPanel();
    }
    QWidget* addTreeHeader = opWidget->findHeaderWidgetByGroupId("OP_MSA_ADD_TREE_WIDGET");
    if(NULL != addTreeHeader) {
        addTreeHeader->hide();
    }

    GroupHeaderImageWidget* header = opWidget->findHeaderWidgetByGroupId("OP_MSA_TREES_WIDGET");
    if(NULL != header) {
        header->show();
        header->changeState();
    }
}

void MSAEditor::sl_hideTreeOP() {
    OptionsPanelWidget* opWidget = dynamic_cast<OptionsPanelWidget*>(optionsPanel->getMainWidget());
    if(NULL == opWidget) {
        return;
    }
    GroupHeaderImageWidget* header = opWidget->findHeaderWidgetByGroupId("OP_MSA_TREES_WIDGET");
    QWidget* groupWidget = opWidget->findOptionsWidgetByGroupId("OP_MSA_TREES_WIDGET");
    bool openAddTreeGroup = (NULL != groupWidget);
    if(openAddTreeGroup) {
        //header->changeState();
    }
    
    header->hide();

    GroupHeaderImageWidget* addTreeHeader = opWidget->findHeaderWidgetByGroupId("OP_MSA_ADD_TREE_WIDGET");
    if(NULL != addTreeHeader) {
        addTreeHeader->show();
        if(openAddTreeGroup) {
            addTreeHeader->changeState();
        }
    }
}


bool MSAEditor::eventFilter(QObject*, QEvent* e) {
    if (e->type() == QEvent::DragEnter || e->type() == QEvent::Drop) {
        QDropEvent* de = (QDropEvent*)e;
        const QMimeData* md = de->mimeData();
        const GObjectMimeData* gomd = qobject_cast<const GObjectMimeData*>(md);
        if (gomd != NULL) {
            if (msaObject->isStateLocked()) {
                return false;
            }
            U2SequenceObject* dnaObj = qobject_cast<U2SequenceObject*> (gomd->objPtr.data());
            if (dnaObj != NULL ) {
                if (U2AlphabetUtils::deriveCommonAlphabet(dnaObj->getAlphabet(), msaObject->getAlphabet()) == NULL) {
                    return false;
                }
                if (e->type() == QEvent::DragEnter) {
                    de->acceptProposedAction();
                } else {
                    U2OpStatus2Log os;
                    copyRowFromSequence(dnaObj, os);
                }
            }
        }
    }

    return false;    
}

void MSAEditor::initDragAndDropSupport()
{
    SAFE_POINT(ui!= NULL, QString("MSAEditor::ui is not initialized in MSAEditor::initDragAndDropSupport"),);
    ui->setAcceptDrops(true);
    ui->installEventFilter(this);
}

void MSAEditor::sl_align(){
    QMenu m, *mm;

    addLoadMenu(&m);
    addCopyMenu(&m);
    addEditMenu(&m);
    addAlignMenu(&m);
    addTreeMenu(&m);
    addStatisticsMenu(&m);
    addViewMenu(&m);
    addExportMenu(&m);
    addAdvancedMenu(&m);

    emit si_buildPopupMenu(this, &m);

    GUIUtils::disableEmptySubmenus(&m);

    mm = GUIUtils::findSubMenu(&m, MSAE_MENU_ALIGN);
    SAFE_POINT(mm != NULL, "mm", );

    mm->exec(QCursor::pos());
}

void MSAEditor::createDistanceColumn(MSADistanceMatrix* algo) {
    ui->createDistanceColumn(algo);
}

void MSAEditor::sl_setSeqAsReference(){
    QPoint menuCallPos = snp.clickPoint;
    QPoint nameMapped = ui->nameList->mapFromGlobal(menuCallPos);
    if ( nameMapped.y() >= 0 ) {
        qint64 newRowId = ui->nameList->sequenceIdAtPos(nameMapped);
        if (MAlignmentRow::invalidRowId() != newRowId && newRowId != snp.seqId) {
            setReference(newRowId);
        }
    }
}

void MSAEditor::sl_unsetReferenceSeq( ) {
    if ( MAlignmentRow::invalidRowId( ) != getReferenceRowId( ) ) {
        setReference( MAlignmentRow::invalidRowId( ) );
    }
}

void MSAEditor::setReference(qint64 sequenceId) {
    if(sequenceId == MAlignmentRow::invalidRowId()){
        exportHighlightedAction->setDisabled(true);
    }else{
        exportHighlightedAction->setEnabled(true);
    }
    snp.seqId = sequenceId;
    emit si_referenceSeqChanged(sequenceId);
    //REDRAW OTHER WIDGETS
}

QString MSAEditor::getReferenceRowName() const {
    const MAlignment alignment = getMSAObject()->getMAlignment();
    U2OpStatusImpl os;
    const int refSeq = alignment.getRowIndexByRowId(getReferenceRowId(), os);
    return (MAlignmentRow::invalidRowId() != refSeq) ? alignment.getRowNames().at(refSeq)
        : QString();
}

void MSAEditor::buildTree() {
    sl_buildTree();
}

void MSAEditor::resetCollapsibleModel() {
    MSACollapsibleItemModel *collapsibleModel = ui->getCollapseModel();
    SAFE_POINT(NULL != collapsibleModel, "NULL collapsible model!", );
    collapsibleModel->reset();
}

void MSAEditor::sl_exportHighlighted(){
    ExportHighligtningDialogController d(ui, (QWidget*)AppContext::getMainWindow()->getQMainWindow());
    MSAHighlightingScheme *scheme = ui->getSequenceArea()->getCurrentHighlightingScheme();
    QString cname = scheme->metaObject()->className();
    d.exec();
    if (d.result() == QDialog::Accepted){
        AppContext::getTaskScheduler()->registerTopLevelTask(new ExportHighligtningTask(&d, ui->getSequenceArea()));
    }
}

//////////////////////////////////////////////////////////////////////////
MSAEditorUI::MSAEditorUI(MSAEditor* _editor)
: editor(_editor), seqArea(NULL), offsetsView(NULL), statusWidget(NULL), collapsibleMode(false), multiTreeViewer(NULL), similarityStatistics(NULL) {
    //undoFWK = new MSAEditorUndoFramework(this, editor->getMSAObject());
    undoFWK = new MsaUndoRedoFramework(this, editor->getMSAObject());

    collapseModel = new MSACollapsibleItemModel(this);

    copySelectionAction = new QAction(tr("Copy selection"), this);
    copySelectionAction->setObjectName("copy_selection");
    copySelectionAction->setShortcut(QKeySequence::Copy);
    copySelectionAction->setShortcutContext(Qt::WidgetShortcut);
    copySelectionAction->setToolTip(QString("%1 (%2)").arg(copySelectionAction->text())
        .arg(copySelectionAction->shortcut().toString()));

    addAction(copySelectionAction);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setMinimumSize(300, 200);

    setWindowIcon(GObjectTypes::getTypeInfo(GObjectTypes::MULTIPLE_ALIGNMENT).icon);

    QWidget *label;
    GScrollBar* shBar = new GScrollBar(Qt::Horizontal);
    QScrollBar* nhBar = new QScrollBar(Qt::Horizontal);
    GScrollBar* cvBar = new GScrollBar(Qt::Vertical);

    seqArea = new MSAEditorSequenceArea(this, shBar, cvBar);
    nameList = new MSAEditorNameList(this, nhBar);
    consArea = new MSAEditorConsensusArea(this);
    offsetsView = new MSAEditorOffsetsViewController(this, editor, seqArea);
    statusWidget = new MSAEditorStatusWidget(editor->getMSAObject(), seqArea);

    QWidget* label1 = createLabelWidget();
    QWidget* label2 = createLabelWidget();
    label1->setMinimumHeight(consArea->height());
    label2->setMinimumHeight(consArea->height());
    offsetsView->getLeftWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    offsetsView->getRightWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    QGridLayout* seqAreaLayout = new QGridLayout();
    seqAreaLayout->setMargin(0);
    seqAreaLayout->setSpacing(0);
    seqAreaLayout->addWidget(consArea, 0, 1);
    seqAreaLayout->addWidget(label1, 0, 0);
    seqAreaLayout->addWidget(offsetsView->getLeftWidget(), 1, 0, 2, 1);
    seqAreaLayout->addWidget(seqArea, 1, 1);
    seqAreaLayout->addWidget(label2, 0, 2);
    seqAreaLayout->addWidget(offsetsView->getRightWidget(), 1, 2, 2, 1);
    seqAreaLayout->addWidget(shBar, 2, 1, 1, 1);
    seqAreaContainer = new QWidget();
    seqAreaContainer->setLayout(seqAreaLayout);

    label = createLabelWidget(tr("Consensus"));
    label->setMinimumHeight(consArea->height());
    nameList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    QVBoxLayout* nameAreaLayout = new QVBoxLayout();
    nameAreaLayout->setMargin(0);
    nameAreaLayout->setSpacing(0);
    nameAreaLayout->addWidget(label);
    nameAreaLayout->addWidget(nameList);
    nameAreaLayout->addWidget(nhBar);
    nameAreaContainer = new QWidget();
    nameAreaContainer->setLayout(nameAreaLayout);

    view.addObject(nameAreaContainer, 0, 0.1);
    view.addObject(seqAreaContainer, 1, 3);

    label = createLabelWidget();
    label->setMinimumHeight(consArea->height());
    cvBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(view.getSpliter(), 0, 0, 2, 1);
    mainLayout->addWidget(cvBar, 1, 1);
    mainLayout->addWidget(label, 0, 1);
    mainLayout->addWidget(statusWidget, 2, 0, 1, 2);
    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(mainLayout);
    setLayout(mainLayout);

    connect(collapseModel, SIGNAL(toggled()), offsetsView, SLOT(sl_modelChanged()));
    connect(collapseModel, SIGNAL(toggled()), seqArea,     SLOT(sl_modelChanged()));
}

QWidget* MSAEditorUI::createLabelWidget(const QString& text, Qt::Alignment ali){
    return new MSALabelWidget(this, text, ali);
}

QAction* MSAEditorUI::getUndoAction() const {
    QAction *a = undoFWK->getUndoAction();
    a->setObjectName("msa_action_undo");
    return a;
}

QAction* MSAEditorUI::getRedoAction() const {
    QAction *a = undoFWK->getRedoAction();
    a->setObjectName("msa_action_redo");
    return a;
}

void MSAEditorUI::sl_saveScreenshot(){
    QRect screenRect = view.getSpliter()->geometry();
    screenRect.setBottom(seqArea->geometry().bottom());
    ExportImageDialog dialog(this, screenRect);
    dialog.exec();
}

void MSAEditorUI::sl_saveSvgImage() {

    LastUsedDirHelper lod;
    lod.url = QFileDialog::getSaveFileName(this, tr("Save SVG"),
        lod.dir, tr("SVG files (*.svg)"));

    QSvgGenerator generator;
    generator.setFileName(lod.url);
    generator.setSize(QSize(width(), height()));
    generator.setViewBox(QRect(0, 0, width(), height()));
    generator.setTitle(tr("SVG %1").arg(editor->getMSAObject()->getGObjectName()));
    generator.setDescription(tr("An SVG image of multiple alignment created by Unipro UGENE"));
    
    QPainter painter;
    painter.begin(&generator);
    painter.translate(nameList->width(), 0);
    consArea->drawContent(painter);
    painter.translate(-nameList->width(), consArea->height());
    nameList->drawContent(painter);
    painter.translate(nameList->width(), 0);
    seqArea->drawContent(painter);
    painter.end();

}
void MSAEditorUI::sl_onTabsCountChanged(int curTabsNumber) {
    if(curTabsNumber < 1) {
        view.removeObject(multiTreeViewer);
        delete multiTreeViewer;
        multiTreeViewer = NULL;
        emit si_hideTreeOP();
        nameList->clearGroupsSelections();
    }
}

void MSAEditorUI::createDistanceColumn(MSADistanceMatrix* algo )
{
    dataList->setAlgorithm(algo);
    dataList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    MSAEditorAlignmentDependentWidget* statisticsWidget = new MSAEditorAlignmentDependentWidget(dataList);

    view.addObject(nameAreaContainer, statisticsWidget, 0.04, 1);
}

void MSAEditorUI::addTreeView(GObjectViewWindow* treeView) {
    if(NULL == multiTreeViewer) {
        multiTreeViewer = new MSAEditorMultiTreeViewer(tr("Tree view"), editor);
        view.addObject(nameAreaContainer, multiTreeViewer, 0.35);
        multiTreeViewer->addTreeView(treeView);
        emit si_showTreeOP();
        connect(multiTreeViewer, SIGNAL(si_tabsCountChanged(int)), SLOT(sl_onTabsCountChanged(int)));
    }
    else {
        multiTreeViewer->addTreeView(treeView);
    }
}

void MSAEditorUI::setSimilaritySettings( const SimilarityStatisticsSettings* settings ) {
    similarityStatistics->setSettings(settings);
}

void MSAEditorUI::refreshSimilarityColumn() {
    dataList->updateWidget();
}

void MSAEditorUI::showSimilarity() {
    if(NULL == similarityStatistics) {
        SimilarityStatisticsSettings settings;
        settings.ma = editor->getMSAObject();
        settings.algoName = AppContext::getMSADistanceAlgorithmRegistry()->getAlgorithmIds().at(0);
        settings.ui = this;

        dataList = new MSAEditorSimilarityColumn(this, new QScrollBar(Qt::Horizontal), &settings);
        dataList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        similarityStatistics = new MSAEditorAlignmentDependentWidget(dataList);

        view.addObject(nameAreaContainer, similarityStatistics, 0.04, 1);
    }
    else {
        similarityStatistics->show();
    }

}

void MSAEditorUI::hideSimilarity() {
    if(NULL != similarityStatistics) {
        similarityStatistics->hide();
    }
}

MSAEditorTreeViewer* MSAEditorUI::getCurrentTree() const
{
    if(NULL == multiTreeViewer) {
        return NULL;
    }
    GObjectViewWindow* page = qobject_cast<GObjectViewWindow*>(multiTreeViewer->getCurrentWidget());
    if(NULL == page) {
        return NULL;
    }
    return qobject_cast<MSAEditorTreeViewer*>(page->getObjectView());
}

MSAWidget::MSAWidget(MSAEditorUI* _ui) 
: ui(_ui), heightMargin(0) {
    connect(ui->getEditor(), SIGNAL(si_zoomOperationPerformed(bool)), SLOT(sl_fontChanged()));
    setMinimumHeight(ui->consArea->height() + heightMargin);
}

void MSAWidget::sl_fontChanged() {
    update();
    setMinimumHeight(ui->consArea->height() + heightMargin);
}
void MSAWidget::setHeightMargin(int _heightMargin) {
    heightMargin = _heightMargin;
    setMinimumHeight(ui->consArea->height() + heightMargin);
}

void MSAWidget::mousePressEvent( QMouseEvent * )
{
    ui->seqArea->cancelSelection();
}
void MSAWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), Qt::white);
}

MSALabelWidget::MSALabelWidget(MSAEditorUI* _ui, const QString & _t, Qt::Alignment _a) 
: MSAWidget(_ui), text(_t), ali(_a)
{
}


void MSALabelWidget::paintEvent(QPaintEvent * e) {
    MSAWidget::paintEvent(e);
    QPainter p(this);
    if (!text.isEmpty()) {
        p.setFont(getMsaEditorFont());
        p.drawText(rect(), text, ali);
    }
}

SinchronizedObjectView::SinchronizedObjectView() 
    : seqArea(NULL)
{
    spliter = new QSplitter(Qt::Horizontal);
}
SinchronizedObjectView::SinchronizedObjectView(QSplitter *_spliter)
    : seqArea(NULL), spliter(_spliter)
{
}

QSplitter* SinchronizedObjectView::getSpliter() {
    return spliter;
}

void MSALabelWidget::mousePressEvent( QMouseEvent * e ){
    ui->getSequenceArea()->cancelSelection();
    QMouseEvent eventForNameListArea(e->type(), QPoint(e->x(), 0), e->globalPos(), e->button(), e->buttons(), e->modifiers());
    QApplication::instance()->notify(ui->getEditorNameList(), &eventForNameListArea);
}

void MSALabelWidget::mouseReleaseEvent( QMouseEvent * e )
{
    QMouseEvent eventForNameListArea(e->type(), QPoint(e->x(), qMax(e->y() - height(), 0)), e->globalPos(), e->button(), e->buttons(), e->modifiers());
    QApplication::instance()->notify(ui->getEditorNameList(), &eventForNameListArea);
}

void SinchronizedObjectView::addObject( QWidget *obj, int index, qreal coef)
{
    SAFE_POINT(coef >= 0, QString("Incorrect parameters were passed to SinchronizedObjectView::addObject: coef < 0"),);

    objects.append(obj);
    int baseSize = spliter->width();
    widgetSizes.insert(index, qRound(coef * baseSize));
    int widgetsWidth = 0;
    foreach(int curSize, widgetSizes) {
        widgetsWidth += curSize;
    }
    for(int i = 0; i < widgetSizes.size(); i++) {
        widgetSizes[i] = widgetSizes[i] * baseSize / widgetsWidth;
    }
    spliter->insertWidget(index, obj);
    spliter->setSizes(widgetSizes);
}
void SinchronizedObjectView::addObject(QWidget *neighboringWidget, QWidget *obj, qreal coef, int neighboringShift) {
    int index = spliter->indexOf(neighboringWidget) + neighboringShift;
    addObject(obj, index, coef);
}

void MSALabelWidget::mouseMoveEvent( QMouseEvent * e )
{
    QMouseEvent eventForSequenceArea(e->type(), QPoint(e->x(), e->y() - height()), e->globalPos(), e->button(), e->buttons(), e->modifiers());
    QApplication::instance()->notify(ui->getEditorNameList(), &eventForSequenceArea);
}

void SinchronizedObjectView::removeObject( QWidget *obj )
{
    int widgetsWidth = 0;
    int baseSize = spliter->width();
    int index = spliter->indexOf(obj);
    if(index < 0) {
        return;
    }
    widgetSizes.removeAt(index);

    foreach(int curSize, widgetSizes) {
        widgetsWidth += curSize;
    }
    for(int i = 0; i < widgetSizes.size(); i++) {
        widgetSizes[i] = widgetSizes[i] * baseSize / widgetsWidth;
    }
    foreach(QWidget *curObj, objects) {
        disconnect(obj,     SIGNAL(si_selectionChanged(const QList<QString>&)), curObj, SLOT(sl_selectionChanged(const QList<QString>&)));
        disconnect(obj,    SIGNAL(si_aligmentChanged(const QList<QString>&)),   curObj, SLOT(sl_aligmentChanged(const QList<QString>&)));
        disconnect(obj,    SIGNAL(si_zoomChanged(double)),                      curObj, SLOT(sl_zoomChanged(double)));
        disconnect(curObj, SIGNAL(si_selectionChanged(const QList<QString>&)),  obj,    SLOT(sl_selectionChanged(const QList<QString>&)));
        disconnect(curObj, SIGNAL(si_aligmentChanged(const QList<QString>&)),   obj,    SLOT(sl_aligmentChanged(const QList<QString>&)));
        disconnect(curObj, SIGNAL(sl_zoomChanged(double)),                      obj,    SLOT(si_zoomChanged(double)));
    }
    objects.removeAll(obj);
    obj->setParent(NULL);
    spliter->setSizes(widgetSizes);
}

}//namespace
