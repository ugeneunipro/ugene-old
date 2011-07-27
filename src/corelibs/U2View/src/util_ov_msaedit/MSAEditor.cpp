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

#include <util_ov_phyltree/TreeViewerTasks.h>

#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/ExportImageDialog.h>
#include <U2Gui/DialogUtils.h>
#include <U2Core/AppContext.h>

#include <QtCore/QEvent>

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

#include <U2Core/IOAdapter.h>
#include <U2Core/Counter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Settings.h>
#include <U2Gui/GScrollBar.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>

#include <U2Core/PhyTreeObject.h>
#include <U2Algorithm/PhyTreeGeneratorTask.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>
#include <U2View/CreatePhyTreeDialogController.h>

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
: GObjectView(MSAEditorFactory::ID, viewName), ui(NULL) {
    msaObject = qobject_cast<MAlignmentObject*>(obj);
    objects.append(msaObject);
    onObjectAdded(msaObject);

    requiredObjects.append(msaObject);
    GCOUNTER(cvar,tvar,"MSAEditor");

    zoomInAction = new QAction(QIcon(":core/images/zoom_in.png"), tr("Zoom In"), this);
    connect(zoomInAction, SIGNAL(triggered()), SLOT(sl_zoomIn()));

    zoomOutAction = new QAction(QIcon(":core/images/zoom_out.png"), tr("Zoom Out"), this);
    connect(zoomOutAction, SIGNAL(triggered()), SLOT(sl_zoomOut()));

    zoomToSelectionAction = new QAction(QIcon(":core/images/zoom_reg.png"), tr("Zoom To Selection"), this);
    connect(zoomToSelectionAction, SIGNAL(triggered()), SLOT(sl_zoomToSelection()));

    resetFontAction = new QAction(QIcon(":core/images/zoom_whole.png"), tr("Reset Zoom"), this);
    connect(resetFontAction, SIGNAL(triggered()), SLOT(sl_resetZoom()));

    changeFontAction = new QAction(QIcon(":core/images/font.png"), tr("Change Font"), this);
    connect(changeFontAction, SIGNAL(triggered()), SLOT(sl_changeFont()));

    buildTreeAction = new QAction(QIcon(":/core/images/phylip.png"), tr("Build Tree"), this);
    connect(buildTreeAction, SIGNAL(triggered()), SLOT(sl_buildTree()));

    Settings* s = AppContext::getSettings();
    zoomFactor = DEFAULT_ZOOM_FACTOR;
    font.setFamily(s->getValue(SETTINGS_ROOT + SETTINGS_FONT_FAMILY, DEFAULT_FONT_FAMILY).toString());
    font.setPointSize(s->getValue(SETTINGS_ROOT + SETTINGS_FONT_SIZE, DEFAULT_FONT_SIZE).toInt());
    font.setItalic(s->getValue(SETTINGS_ROOT + SETTINGS_FONT_ITALIC, false).toBool());
    font.setBold(s->getValue(SETTINGS_ROOT + SETTINGS_FONT_BOLD, false).toBool());
    calcFontPixelToPointSizeCoef();

    updateActions();

    if ( (font.pointSize() == MIN_FONT_SIZE) && (zoomFactor < 1.0f) ) {
        resizeMode = ResizeMode_OnlyContent;
    } else {
        resizeMode = ResizeMode_FontAndContent;
    }
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
        resizeMode = ResizeMode_FontAndContent;
        zoomFactor = 1;
        resizeModeChanged = true;
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
        Q_ASSERT(zoomMult > 0);
        zoomFactor /= zoomMult;
        resizeMode = ResizeMode_OnlyContent;
        resizeModeChanged = true;
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

    emit si_zoomOperationPerformed(oldMode == resizeMode);
}

void MSAEditor::sl_buildTree() {
    PhyTreeGeneratorRegistry* registry = AppContext::getPhyTreeGeneratorRegistry();
    QStringList list = registry->getNameList();
    if (list.size() == 0){
        QMessageBox::information(ui, tr("Calculate phy tree"),
            tr("No algorithms for building phylogenetic tree are available.") );
        return;
    }        

    CreatePhyTreeDialogController dlg(widget, msaObject, settings);

    int rc = dlg.exec();
    if (rc != QDialog::Accepted) {
        return;
    }

    treeGeneratorTask = new PhyTreeGeneratorTask(msaObject->getMAlignment(), settings);
    connect(treeGeneratorTask, SIGNAL(si_stateChanged()), SLOT(sl_openTree()));
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    scheduler->registerTopLevelTask(treeGeneratorTask);
}

void MSAEditor::sl_openTree() {
    if (treeGeneratorTask->getState() != Task::State_Finished || treeGeneratorTask->hasError()) {
        return;
    }

    const GUrl& msaURL = msaObject->getDocument()->getURL();
    assert(!msaURL.isEmpty());
    if (msaURL.isEmpty()) {
        return;
    }

    Project* p = AppContext::getProject();
    QString treeFileName = settings.fileUrl.getURLString();
    if (treeFileName.isEmpty()) {
        treeFileName = GUrlUtils::rollFileName(msaURL.dirPath() + "/" + msaURL.baseFileName() + ".nwk", DocumentUtils::getNewDocFileNameExcludesHint());
    }

    PhyTreeObject *newObj = new PhyTreeObject(treeGeneratorTask->getResult(), "Tree");
    DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::NEWICK);
    IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    Document *d = new Document(df, iof, treeFileName, QList<GObject*>()<<newObj);
    p->addDocument(d);

    Task* task = new OpenTreeViewerTask(newObj);
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    scheduler->registerTopLevelTask(task);
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
    QFont f(DEFAULT_FONT_FAMILY, DEFAULT_FONT_SIZE);
    setFont(f);
    zoomFactor = DEFAULT_ZOOM_FACTOR;
    ResizeMode oldMode = resizeMode;
    resizeMode = ResizeMode_FontAndContent;
    emit si_zoomOperationPerformed(resizeMode == oldMode);
    
    updateActions();
}

MSAEditor::~MSAEditor() {
}

void MSAEditor::buildStaticToolbar(QToolBar* tb) {
    tb->addAction(zoomInAction);
    tb->addAction(zoomOutAction);
    tb->addAction(zoomToSelectionAction);
    tb->addAction(resetFontAction);
    tb->addAction(changeFontAction);
    tb->addAction(buildTreeAction);
    tb->addAction(saveScreenshotAction);
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
    assert(ui == NULL);
    ui = new MSAEditorUI(this);
    connect(ui , SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sl_onContextMenuRequested(const QPoint &)));
    saveScreenshotAction = new QAction(QIcon(":/core/images/cam2.png"), tr("Export as image"), this);
    connect(saveScreenshotAction, SIGNAL(triggered()), ui, SLOT(sl_saveScreenshot()));
    initDragAndDropSupport();
    return ui;
}

int MSAEditor::getAlignmentLen() const {
    return msaObject->getMAlignment().getLength();
}

int MSAEditor::getNumSequences() const {
    return msaObject->getMAlignment().getNumRows();
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

bool MSAEditor::eventFilter(QObject*, QEvent* e) {
    if (e->type() == QEvent::DragEnter || e->type() == QEvent::Drop) {
        QDropEvent* de = (QDropEvent*)e;
        const QMimeData* md = de->mimeData();
        const GObjectMimeData* gomd = qobject_cast<const GObjectMimeData*>(md);
        if (gomd != NULL) {
            if (msaObject->isStateLocked()) {
                return false;
            }
            DNASequenceObject* dnaObj = qobject_cast<DNASequenceObject*> (gomd->objPtr.data());
            if (dnaObj != NULL ) {
                if (DNAAlphabet::deriveCommonAlphabet(dnaObj->getAlphabet(), msaObject->getAlphabet()) == NULL) {
                    return false;
                }
                if (e->type() == QEvent::DragEnter) {
                    de->acceptProposedAction();
                } else {     
                    msaObject->addRow(dnaObj->getDNASequence());
                }    
            }
        }
    }

    return false;    
}

void MSAEditor::initDragAndDropSupport()
{
    assert(ui!= NULL);
    ui->setAcceptDrops(true);
    ui->installEventFilter(this);
}

//////////////////////////////////////////////////////////////////////////
MSAEditorUI::MSAEditorUI(MSAEditor* _editor): editor(_editor), seqArea(NULL), offsetsView(NULL), statusWidget(NULL), collapsibleMode(false) {
    undoFWK = new MSAEditorUndoFramework(this, editor->getMSAObject());
    collapseModel = new MSACollapsibleItemModel(this);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setMinimumSize(300, 200);

    setWindowIcon(GObjectTypes::getTypeInfo(GObjectTypes::MULTIPLE_ALIGNMENT).icon);

    QWidget *seqAreaContainer, *nameAreaContainer, *label;
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

    splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(nameAreaContainer);
    splitter->addWidget(seqAreaContainer);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 0);
    splitter->setStretchFactor(2, 1);
    //TODO: make initial namelist size depend on font
    int baseSize = splitter->width();
    int nameListSize = baseSize / 5;
    splitter->setSizes(QList<int>()<< nameListSize << baseSize - nameListSize); 
    splitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    label = createLabelWidget();
    label->setMinimumHeight(consArea->height());
    cvBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(splitter, 0, 0, 2, 1);
    mainLayout->addWidget(cvBar, 1, 1);
    mainLayout->addWidget(label, 0, 1);
    mainLayout->addWidget(statusWidget, 2, 0, 1, 2);
    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(mainLayout);
    setLayout(mainLayout);

    connect(collapseModel, SIGNAL(toggled()), offsetsView, SLOT(sl_modelChanged()));
}

QWidget* MSAEditorUI::createLabelWidget(const QString& text, Qt::Alignment ali) const {
    return new MSALabelWidget(this, text, ali);
}

QAction* MSAEditorUI::getUndoAction() const {
    return undoFWK->getUndoAction();
}

QAction* MSAEditorUI::getRedoAction() const {
    return undoFWK->getRedoAction();
}

void MSAEditorUI::sl_saveScreenshot(){
    ExportImageDialog dialog(this);
    dialog.exec();
}

MSALabelWidget::MSALabelWidget(const MSAEditorUI* _ui, const QString & _t, Qt::Alignment _a) 
: ui(_ui), text(_t), ali(_a)
{
    connect(ui->getEditor(), SIGNAL(si_fontChanged(const QFont&)), SLOT(sl_fontChanged()));
}

void MSALabelWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), Qt::white);
    if (!text.isEmpty()) {
        p.setFont(ui->getEditor()->getFont());
        p.drawText(rect(), text, ali);
    }
}

void MSALabelWidget::sl_fontChanged() {
    update();
    setMinimumHeight(ui->consArea->height());
}

void MSALabelWidget::mousePressEvent( QMouseEvent * )
{
    ui->seqArea->cancelSelection();
}


}//namespace

