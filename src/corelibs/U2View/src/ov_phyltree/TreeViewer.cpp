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
#include "TreeViewer.h"
#include "TreeViewerFactory.h"
#include "GraphicsBranchItem.h"
#include "GraphicsButtonItem.h"
#include "TreeViewerUtils.h"
#include "CreateBranchesTask.h"
#include "CreateCircularBranchesTask.h"
#include "CreateRectangularBranchesTask.h"
#include "CreateUnrootedBranchesTask.h"
#include "CreateRectangularBranchesTask.h"
#include "GraphicsRectangularBranchItem.h"

#include "TreeViewerTasks.h"
#include "TreeViewerState.h"


#include <U2Core/AppContext.h>

#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/L10n.h>
#include <U2Core/U2Region.h>
#include <U2Core/PhyTree.h>
#include <U2Core/Counter.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/ExportImageDialog.h>

#include <U2Gui/HBar.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/OptionsPanel.h>
#include <U2Gui/OPWidgetFactoryRegistry.h>
#include <U2View/CreatePhyTreeDialogController.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>

#include <QtCore/QStack>
#include <QtCore/QQueue>

#include <QtGui/QVBoxLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QtGui/QPainter>
#include <QtXml/QtXml>
#include <QtSvg/QSvgGenerator>
#include <QtGui/QGraphicsSimpleTextItem>
#include <QtGui/QGraphicsLineItem>
#include <QtGui/QMessageBox>
#include <QtGui/QSplitter>


namespace U2 {


TreeViewer::TreeViewer(const QString& viewName, GObject* obj, GraphicsRectangularBranchItem* _root, qreal s):
    GObjectView(TreeViewerFactory::ID, viewName),
    treeSettingsAction(NULL),
    layoutGroup(NULL),
    rectangularLayoutAction(NULL),
    circularLayoutAction(NULL),
    unrootedLayoutAction(NULL),
    branchesSettingsAction(NULL),
    nameLabelsAction(NULL),
    distanceLabelsAction(NULL),
    textSettingsAction(NULL),
    contAction(NULL),
    zoomToSelAction(NULL),
    zoomToAllAction(NULL),
    zoomOutAction(NULL),
    printAction(NULL),
    captureTreeAction(NULL),
    exportAction(NULL),
    root(_root),
    scale(s),
    ui(NULL)
{
    GCOUNTER( cvar, tvar, "PhylTreeViewer" );
    phyObject = qobject_cast<PhyTreeObject*>(obj);
    objects.append(phyObject);
    requiredObjects.append(phyObject);
    onObjectAdded(phyObject);
    connect(phyObject, SIGNAL(si_phyTreeChanged()), SLOT(sl_onPhyTreeChanged()));
}

QTransform TreeViewer::getTransform() const {
    return ui->transform();
}

void TreeViewer::setTransform(const QTransform& m) {

    ui->setTransform(m);
}

QVariantMap TreeViewer::saveState() {

    return TreeViewerState::saveState(this);
}


qreal TreeViewer::getHorizontalZoom() const {
    return ui->getHorizontalZoom();
}
void TreeViewer::setHorizontalZoom(qreal z) {
    ui->setHorizontalZoom(z);
}
qreal TreeViewer::getVerticalZoom() const {
    return ui->getVerticalZoom();
}
void TreeViewer::setVerticalZoom(qreal z) {
    ui->setVerticalZoom(z);
}

QVariantMap TreeViewer::getSettingsState() const {

    return ui->getSettingsState();
}

void TreeViewer::setSettingsState(const QVariantMap& m) {

    ui->setSettingsState(m);
}

Task* TreeViewer::updateViewTask(const QString& stateName, const QVariantMap& stateData) {

    return new UpdateTreeViewerTask(this, stateName, stateData);
}

void TreeViewer::createActions() {
    // Tree Settings
    treeSettingsAction = new QAction(QIcon(":core/images/phylip.png"), tr("Tree Settings..."), ui);

    // Layout
    layoutGroup = new QActionGroup(ui);

    rectangularLayoutAction = new QAction(tr("Rectangular"), layoutGroup);
    rectangularLayoutAction->setCheckable(true);
    rectangularLayoutAction->setChecked(true);
    rectangularLayoutAction->setObjectName("Rectangular");

    circularLayoutAction = new QAction(tr("Circular"), layoutGroup);
    circularLayoutAction->setCheckable(true);
    circularLayoutAction->setObjectName("Circular");

    unrootedLayoutAction = new QAction(tr("Unrooted"), layoutGroup);
    unrootedLayoutAction->setCheckable(true);
    unrootedLayoutAction->setObjectName("Unrooted");

    // Branch Settings
    branchesSettingsAction = new QAction(QIcon(":core/images/color_wheel.png"), tr("Branch Settings..."), ui);
    branchesSettingsAction->setObjectName("Branch Settings");
    // Show Labels
    nameLabelsAction = new QAction(tr("Show Names"), ui);
    nameLabelsAction->setCheckable(true);
    nameLabelsAction->setChecked(true);
    nameLabelsAction->setObjectName("Show Names");

    distanceLabelsAction = new QAction(tr("Show Distances"), ui);
    distanceLabelsAction->setCheckable(true);
    distanceLabelsAction->setChecked(true);
    distanceLabelsAction->setObjectName("Show Distances");

    // Formatting
    textSettingsAction = new QAction(QIcon(":core/images/font.png"), tr("Formatting..."), ui);

    // Align Labels
    // Note: the icon is truncated to 15 px height to look properly in the main menu when it is checked
    contAction = new QAction(QIcon(":core/images/align_tree_labels.png"), tr("Align Labels"), ui);
    contAction->setCheckable(true);
    contAction->setObjectName("Align Labels");

    // Zooming
    zoomToSelAction = new QAction(QIcon(":core/images/zoom_in.png"), tr("Zoom In") , ui);
    zoomOutAction = new QAction(QIcon(":core/images/zoom_out.png"), tr("Zoom Out"), ui);
    zoomToAllAction = new QAction(QIcon(":core/images/zoom_whole.png"), tr("Reset Zooming"), ui);

    // Print Tree
    printAction = new QAction(QIcon(":/core/images/printer.png"), tr("Print Tree..."), ui);

    // Screen Capture
    captureTreeAction = new QAction(tr("Screen Capture..."), ui);
    captureTreeAction->setObjectName("Screen Capture");
    exportAction = new QAction(tr("Whole Tree as SVG..."), ui);
    exportAction->setObjectName("Whole Tree as SVG");
}

void TreeViewer::setupLayoutSettingsMenu(QMenu* m)
{
    m->addActions(layoutGroup->actions());
}

void TreeViewer::setupShowLabelsMenu(QMenu* m)
{
    m->addAction(nameLabelsAction);
    m->addAction(distanceLabelsAction);
}

void TreeViewer::setupCameraMenu(QMenu* m)
{
    m->addAction(captureTreeAction);
    m->addAction(exportAction);
}

void TreeViewer::buildStaticToolbar(QToolBar* tb) 
{
    // Tree Settings
    tb->addAction(treeSettingsAction);

    // Layout
    QToolButton* layoutButton = new QToolButton(tb);
    QMenu* layoutMenu = new QMenu(tr("Layout"), ui);
    setupLayoutSettingsMenu(layoutMenu);
    layoutButton->setDefaultAction(layoutMenu->menuAction());
    layoutButton->setPopupMode(QToolButton::InstantPopup);
    layoutButton->setIcon(QIcon(":core/images/tree_layout.png"));
    layoutButton->setObjectName("Layout");
    tb->addWidget(layoutButton);

    // Branch Settings
    tb->addAction(branchesSettingsAction);

    // Labels and Text Settings
    tb->addSeparator();
    QToolButton* showLabelsButton = new QToolButton();
    QMenu* showLabelsMenu = new QMenu(tr("Show Labels"), ui);
    showLabelsButton->setObjectName("Show Labels");
    setupShowLabelsMenu(showLabelsMenu);
    showLabelsButton->setDefaultAction(showLabelsMenu->menuAction());
    showLabelsButton->setPopupMode(QToolButton::InstantPopup);
    showLabelsButton->setIcon(QIcon(":/core/images/text_ab.png"));
    tb->addWidget(showLabelsButton);

    tb->addAction(textSettingsAction);
    tb->addAction(contAction);
    // Zooming
    tb->addSeparator();
    tb->addAction(zoomToSelAction);
    tb->addAction(zoomOutAction);
    tb->addAction(zoomToAllAction);

    // Print and Capture
    tb->addSeparator();
    tb->addAction(printAction);

    QToolButton* cameraButton = new QToolButton();
    QMenu* cameraMenu = new QMenu(tr("Export Tree Image"), ui);
    setupCameraMenu(cameraMenu);
    cameraButton->setDefaultAction(cameraMenu->menuAction());
    cameraButton->setPopupMode(QToolButton::InstantPopup);
    cameraButton->setIcon(QIcon(":/core/images/cam2.png"));
    cameraMenu->menuAction()->setObjectName("Export Tree Image");
    cameraButton->setObjectName("cameraMenu");
    tb->addWidget(cameraButton);
}

void TreeViewer::buildMSAEditorStaticToolbar(QToolBar* tb) {
    buildStaticToolbar(tb);
    tb->removeAction(zoomToSelAction);
    tb->removeAction(zoomOutAction);
    tb->removeAction(zoomToAllAction);
}

void TreeViewer::buildStaticMenu(QMenu* m)
{
    // Tree Settings
    m->addAction(treeSettingsAction);

    // Layout
    QMenu* layoutMenu = new QMenu(tr("Layout"), ui);
    setupLayoutSettingsMenu(layoutMenu);
    layoutMenu->setIcon(QIcon(":core/images/tree_layout.png"));
    m->addMenu(layoutMenu);

    // Branch Settings
    m->addAction(branchesSettingsAction);

    // Labels and Text Settings
    m->addSeparator();

    QMenu* labelsMenu = new QMenu(tr("Show Labels"), ui);
    labelsMenu->menuAction()->setObjectName("show_labels_action");
    setupShowLabelsMenu(labelsMenu);
    labelsMenu->setIcon(QIcon(":/core/images/text_ab.png"));
    m->addMenu(labelsMenu);

    m->addAction(textSettingsAction);

    m->addAction(contAction);
    // Zooming
    m->addSeparator();
    m->addAction(zoomToSelAction);
    m->addAction(zoomOutAction);
    m->addAction(zoomToAllAction);

    // Print and Capture
    m->addSeparator();
    m->addAction(printAction);

    QMenu* cameraMenu = new QMenu(tr("Export Tree Image"), ui);
    setupCameraMenu(cameraMenu);
    cameraMenu->setIcon(QIcon(":/core/images/cam2.png"));
    cameraMenu->menuAction()->setObjectName("Export Tree Image");
    m->addMenu(cameraMenu);

    m->addSeparator();

    GObjectView::buildStaticMenu(m);
    GUIUtils::disableEmptySubmenus(m);
}

QWidget* TreeViewer::createWidget() {
    assert(ui == NULL);
    ui = new TreeViewerUI(this);

    optionsPanel = new OptionsPanel(this);
    OPWidgetFactoryRegistry* opWidgetFactoryRegistry = AppContext::getOPWidgetFactoryRegistry();

    QList<OPFactoryFilterVisitorInterface*> filters;
    filters.append(new OPFactoryFilterVisitor(ObjViewType_PhylogeneticTree));

    QList<OPWidgetFactory*> opWidgetFactoriesForSeqView = opWidgetFactoryRegistry->getRegisteredFactories(filters);
    foreach (OPWidgetFactory* factory, opWidgetFactoriesForSeqView) {
        optionsPanel->addGroup(factory);
    }

    qDeleteAll(filters);

    return ui;
}

void TreeViewer::onObjectRenamed(GObject*, const QString&) {
    // update title
    OpenTreeViewerTask::updateTitle(this);
}

void TreeViewer::sl_onPhyTreeChanged() {
    ui->onPhyTreeChanged();
}


////////////////////////////
// TreeViewerUI

const qreal TreeViewerUI::ZOOM_COEF = 1.2;
const qreal TreeViewerUI::MINIMUM_ZOOM = 0.1;
const qreal TreeViewerUI::MAXIMUM_ZOOM = 100.0; 
const int TreeViewerUI::MARGIN = 10;
const qreal TreeViewerUI::SIZE_COEF = 0.1;


TreeViewerUI::TreeViewerUI(TreeViewer* treeViewer): phyObject(treeViewer->getPhyObject()), root(treeViewer->getRoot()), layout(TreeLayout_Rectangular), curTreeViewer(NULL), rectRoot(treeViewer->getRoot()) {
    curTreeViewer = treeViewer;
    labelsSettings.alignLabels = false;
    labelsSettings.showDistances = true;
    labelsSettings.showNames = true;
    maxNameWidth = 0;
    horizontalScale = 1.0;
    verticalScale = 1.0;

    setWindowIcon(GObjectTypes::getTypeInfo(GObjectTypes::PHYLOGENETIC_TREE).icon);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setFrameShape(QFrame::NoFrame);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setScene(new QGraphicsScene());
    scene()->addItem(root);
    setScale(treeViewer->getScale());
    addLegend(treeViewer->getScale());
    updateRect();

    treeViewer->createActions();
    connect(treeViewer->getNameLabelsAction(), SIGNAL(triggered(bool)), SLOT(sl_showNameLabelsTriggered(bool)));
    connect(treeViewer->getDistanceLabelsAction(), SIGNAL(triggered(bool)), SLOT(sl_showDistanceLabelsTriggered(bool)));
    connect(treeViewer->getPrintAction(), SIGNAL(triggered()), SLOT(sl_printTriggered()));
    connect(treeViewer->getCaptureTreeAction(), SIGNAL(triggered()), SLOT(sl_captureTreeTriggered()));
    connect(treeViewer->getExportAction(), SIGNAL(triggered()), SLOT(sl_exportTriggered()));
    connect(treeViewer->getContAction(), SIGNAL(triggered(bool)), SLOT(sl_contTriggered(bool)));
    connect(treeViewer->getRectangularLayoutAction(), SIGNAL(triggered(bool)), SLOT(sl_rectangularLayoutTriggered()));
    connect(treeViewer->getCircularLayoutAction(), SIGNAL(triggered(bool)), SLOT(sl_circularLayoutTriggered()));
    connect(treeViewer->getUnrootedLayoutAction(), SIGNAL(triggered(bool)), SLOT(sl_unrootedLayoutTriggered()));
    connect(treeViewer->getTextSettingsAction(), SIGNAL(triggered()), SLOT(sl_textSettingsTriggered()));
    connect(treeViewer->getTreeSettingsAction(), SIGNAL(triggered()), SLOT(sl_treeSettingsTriggered()));
    connect(treeViewer->getZoomToSelAction(), SIGNAL(triggered()), SLOT(sl_zoomToSel()));
    connect(treeViewer->getZoomOutAction(), SIGNAL(triggered()), SLOT(sl_zoomOut()));
    connect(treeViewer->getZoomToAllAction(), SIGNAL(triggered()), SLOT(sl_zoomToAll()));
    connect(treeViewer->getBranchesSettingsAction(), SIGNAL (triggered()), SLOT(sl_setSettingsTriggered()));

    zoomToAction = treeViewer->getZoomToSelAction();
    zoomOutAction = treeViewer->getZoomOutAction();
    zoomToAllAction = treeViewer->getZoomToAllAction();
    setColorAction = treeViewer->getBranchesSettingsAction();
    captureAction = treeViewer->getCaptureTreeAction();
    exportAction = treeViewer->getExportAction();
    buttonPopup = new QMenu(this);

    //chrootAction->setEnabled(false); //not implemented yet

    swapAction = buttonPopup->addAction(QObject::tr("Swap Siblings"));
    connect(swapAction, SIGNAL(triggered(bool)), SLOT(sl_swapTriggered()));
    swapAction->setObjectName("Swap Siblings");

    buttonPopup->addAction(zoomToAction);

    collapseAction = buttonPopup->addAction(QObject::tr("Collapse"));
    connect(collapseAction, SIGNAL(triggered(bool)), SLOT(sl_collapseTriggered()));
    collapseAction->setObjectName("Collapse");

    buttonPopup->addAction(setColorAction);

    QMenu* cameraMenu = new QMenu(tr("Export Tree Image"), this);
    cameraMenu->addAction(captureAction);
    cameraMenu->addAction(exportAction);
    cameraMenu->menuAction()->setObjectName("Export Tree Image");
    cameraMenu->setIcon(QIcon(":/core/images/cam2.png"));
    buttonPopup->addMenu(cameraMenu);

    updateActionsState();
    this->setObjectName("treeView");
    updateTreeSettings();

    setContextMenuPolicy(Qt::CustomContextMenu);
}

TreeViewerUI::~TreeViewerUI() {
    delete scene();
}


BranchSettings TreeViewerUI::getBranchSettings() const {

    return branchSettings;
}

ButtonSettings TreeViewerUI::getButtonSettings() const {

    return buttonSettings;
}

TextSettings TreeViewerUI::getTextSettings() const {

    return textSettings;
}

TreeSettings TreeViewerUI::getTreeSettings() const {

    return treeSettings;
}
TreeLabelsSettings TreeViewerUI::getLabelsSettings() const {
    return labelsSettings;
}

void TreeViewerUI::setTreeLayout(TreeLayout newLayout) {
    QMenu* layoutMenu = curTreeViewer->getRectangularLayoutAction()->menu();
    switch(newLayout) {
        case TreeLayout_Rectangular:
            if(layoutMenu) {
                layoutMenu->setActiveAction(curTreeViewer->getRectangularLayoutAction());
            }
            sl_rectangularLayoutTriggered();
            break;
        case TreeLayout_Circular:
            if(layoutMenu) {
                layoutMenu->setActiveAction(curTreeViewer->getCircularLayoutAction());
            }
            sl_circularLayoutTriggered();
            break;
        case TreeLayout_Unrooted:
            if(layoutMenu) {
                layoutMenu->setActiveAction(curTreeViewer->getUnrootedLayoutAction());
            }
            sl_unrootedLayoutTriggered();
            break;
    }
}
const TreeViewerUI::TreeLayout& TreeViewerUI::getTreeLayout() const {
    return layout;
}

void TreeViewerUI::onPhyTreeChanged() {
    layoutTask = new CreateRectangularBranchesTask(phyObject->getTree()->getRootNode());

    connect(layoutTask, SIGNAL(si_stateChanged()), SLOT(sl_rectLayoutRecomputed()));
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    scheduler->registerTopLevelTask(layoutTask);

}

void TreeViewerUI::updateSettings(const BranchSettings &settings) {

    branchSettings = settings;
    updateSettings();
}

void TreeViewerUI::updateSettings(const ButtonSettings &settings) {

    buttonSettings = settings;
    updateSettings();
}

void TreeViewerUI::updateSettings(const TextSettings &settings) {

    textSettings = settings;
    updateTextSettings();
}

void TreeViewerUI::updateSettings(const TreeSettings &settings) {

    treeSettings = settings;
    if(layout!=TreeLayout_Rectangular){
        updateLayout();
    }else{
        updateTreeSettings();
    }
}

void TreeViewerUI::updateSettings(const TreeLabelsSettings &settings) {
    sl_showNameLabelsTriggered(settings.showNames);
    sl_showDistanceLabelsTriggered(settings.showDistances);
    sl_contTriggered(settings.alignLabels && settings.showNames);
}

void TreeViewerUI::sl_setSettingsTriggered() {

    sl_branchSettings();
}

void TreeViewerUI::sl_branchSettings() {

    BranchSettingsDialog d(this, getBranchSettings());
    if (d.exec()) {
        updateSettings( d.getSettings() );
        emit si_settingsChanged();
    }
}

void TreeViewerUI::updateSettings() {
    QList<QGraphicsItem*> updatingItems = this->scene()->selectedItems();
    if(updatingItems.isEmpty()){
        updatingItems = this->items();

        QGraphicsLineItem* legendLine = dynamic_cast<QGraphicsLineItem*>(legend);
        if(legendLine){
            QPen legendPen;
            legendPen.setColor(branchSettings.branchColor);
            legendLine->setPen(legendPen);
        }
    }

    foreach (QGraphicsItem *graphItem, updatingItems) {

        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(graphItem);
        if (branchItem) {
            branchItem->updateSettings(branchSettings);
            if(branchItem->getCorrespondingItem()){
                branchItem->getCorrespondingItem()->updateSettings(branchSettings);
            }
        }

        scene()->update();
    }
    
}
void TreeViewerUI::updateTextSettings(){
    QList<QGraphicsItem*> updatingItems = this->scene()->selectedItems();
    if(updatingItems.isEmpty()){
        updatingItems = this->items();

        QList<QGraphicsItem*> legendChildItems = legend->children();
        if(!legendChildItems.isEmpty()){
            QGraphicsSimpleTextItem* legendText = dynamic_cast<QGraphicsSimpleTextItem*>(legendChildItems.first());
            if(legendText){
                legendText->setBrush(textSettings.textColor);
            }
        }
    }

    foreach (QGraphicsItem *graphItem, updatingItems) {

        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(graphItem);
        if (branchItem) {
            branchItem->updateTextSettings(textSettings.textFont, textSettings.textColor);
            if(branchItem->getCorrespondingItem()){
                branchItem->getCorrespondingItem()->updateTextSettings(textSettings.textFont, textSettings.textColor);
            }
        }

        //scene()->update();
    }    

    updateLayout();
    updateTreeSettings();
}
void TreeViewerUI::updateTreeSettings(bool setDefautZoom){

    qreal avgW = 0;
    if(treeSettings.type != TreeSettings::PHYLOGRAM){
        avgW = avgWidth(); 
        legend->setVisible(false);
    }else{
         legend->setVisible(true);
    }

    if(treeSettings.type == TreeSettings::DEFAULT) {
        determineBranchLengths();
    }
    
    QStack<GraphicsBranchItem*> stack;
    stack.push(rectRoot);
    while (!stack.empty()) {
        GraphicsBranchItem *item = stack.pop();

        foreach (QGraphicsItem* ci, item->childItems()) {
            GraphicsBranchItem* gbi = dynamic_cast<GraphicsBranchItem*>(ci);
            if (gbi != NULL) {
                stack.push(gbi);
                gbi->setLenghtCoef(qMax(1, item->getBranchLength() - gbi->getBranchLength()));
            }
        }

        if(item!=rectRoot){
         
            if(layout == TreeLayout_Rectangular){
                GraphicsRectangularBranchItem *rectItem = dynamic_cast<GraphicsRectangularBranchItem*>(item);
                if(rectItem){
                    rectItem->setHeightCoef(treeSettings.height_coef);
                }
            }

            qreal coef  = qMax(1.0, TreeViewerUI::SIZE_COEF*treeSettings.width_coef);

            switch (treeSettings.type)
            {
            case TreeSettings::DEFAULT:
                if(item->getDistanceText() != NULL){
                    if(item->getDistanceText()->text() == ""){
                        item->setDistanceText("0");
                    }
                }
                if(item->getNameText() == NULL) {
                    item->setWidth(avgW * getScale()* coef * item->getLengthCoef());
                }
                else {
                    item->setWidth(0);
                }
                break;
            case TreeSettings::PHYLOGRAM:
                if(item->getDistanceText() != NULL){
                    if(item->getDistanceText()->text() == "0"){
                        item->setDistanceText("");
                    }
                }
                item->setWidth(qAbs(item->getDist()) * getScale()* coef);
                break;
            case TreeSettings::CLADOGRAM:
                if(item->getDistanceText() != NULL){
                    if(item->getDistanceText()->text() == ""){
                        item->setDistanceText("0");
                    }
                }
                if(item->getNameText() == NULL) {
                    item->setWidth(avgW * getScale()* coef);
                }
                else {
                    item->setWidth(0);
                }
                break;
            default:
                assert(false && "Unexpected tree type value");
                break;
            }
        }
    }
    updateRect();
    scene()->update();

    if(labelsSettings.alignLabels){
        updateLabelsAlignment(labelsSettings.alignLabels);
    }

    defaultZoom();
    if(setDefautZoom) {
        fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    }
}

void TreeViewerUI::determineBranchLengths() {
    QStack<GraphicsBranchItem*> stack;
    QQueue<GraphicsBranchItem*> childQueue;
    stack.push(rectRoot);
    while (!stack.empty()) {
        GraphicsBranchItem *item = stack.pop();
        int childsCounter = 0;

        foreach (QGraphicsItem* ci, item->childItems()) {
            GraphicsBranchItem* gbi = dynamic_cast<GraphicsBranchItem*>(ci);
            if (gbi != NULL) {
                stack.push(gbi);
                childsCounter++;
            }
        }

        if(item == rectRoot) { 
            continue;
        }

        if(0 == childsCounter) {
            childQueue.enqueue(item);
            item->setBranchLength(0);
            continue;
        }
    }
    while(!childQueue.isEmpty()) {
        GraphicsBranchItem *item = childQueue.dequeue();
        GraphicsBranchItem *parentItem = dynamic_cast<GraphicsBranchItem *>(item->getParentItem());
        if(NULL == parentItem) {
            continue;
        }

        parentItem->setBranchLength(qMax(parentItem->getBranchLength(), item->getBranchLength() + 1));

        childQueue.enqueue(parentItem);
    }
}

#define BRANCH_COLOR        QString("branch_color")
#define BRANCH_THICKNESS    QString("branch_thickness")

QVariantMap TreeViewerUI::getSettingsState() const {

    QVariantMap m;

    int i=0;
    foreach (QGraphicsItem *graphItem, this->items()) {
        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(graphItem);
        if (branchItem) {
            m[BRANCH_COLOR + i] = branchItem->getBranchSettings().branchColor;
            m[BRANCH_THICKNESS + i] = branchItem->getBranchSettings().branchThickness;
            i++;
        }
    }

    return m;
}

void TreeViewerUI::setSettingsState(const QVariantMap& m) {

    int i=0;
    foreach (QGraphicsItem *graphItem, this->items()) {
        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(graphItem);
        if (branchItem) {
            BranchSettings branchSettings = branchItem->getBranchSettings();

            QVariant vColor = m[BRANCH_COLOR + i];
            if (vColor.type() == QVariant::Color) {
                branchSettings.branchColor = vColor.value<QColor>();
            }

            QVariant vThickness = m[BRANCH_THICKNESS + i];
            if (vThickness.type() == QVariant::Int) {
                branchSettings.branchThickness = vThickness.toInt();
            }

            branchItem->updateSettings(branchSettings);
            i++;
        }
    }
}

void TreeViewerUI::addLegend(qreal scale) {
    static const qreal WIDTH = 30.0;
    qreal d = WIDTH / scale;
    QString str = QString::number(d, 'f', 3);
    int i = str.length() - 1;
    for (; i >= 0 && str[i] == '0'; --i) ;
    if (str[i] == '.')
        --i;
    str.truncate(i + 1);

    legend = new QGraphicsLineItem(0, 0, WIDTH, 0);
    QGraphicsSimpleTextItem* text = new QGraphicsSimpleTextItem(str, legend);
    text->setFont(TreeViewerUtils::getFont());
    QRectF rect = text->boundingRect();
    text->setPos(0.5 * (WIDTH - rect.width()), -rect.height());
    scene()->addItem(legend);
}

void TreeViewerUI::wheelEvent(QWheelEvent *we) {
    qreal zoom1 = pow(ZOOM_COEF, we->delta() / 120.0);
    zooming(zoom1);
    we->accept();
}

void TreeViewerUI::zooming(qreal newzoom){
    zooming(newzoom, newzoom);
}

void TreeViewerUI::zooming(qreal horizontalZoom, qreal verticalZoom) {
    if(verticalZoom < 0 || horizontalZoom < 0){
        return ;
    }
    verticalZoom = verticalScale * verticalZoom;
    horizontalZoom = horizontalScale * horizontalZoom;
    
    verticalZoom = qMax(MINIMUM_ZOOM, verticalZoom);
    verticalZoom  = qMin(MAXIMUM_ZOOM * qMax(treeSettings.height_coef*TreeViewerUI::SIZE_COEF, 1.0), verticalZoom);
    horizontalZoom = qMax(MINIMUM_ZOOM, horizontalZoom);
    horizontalZoom  = qMin(MAXIMUM_ZOOM * qMax(treeSettings.width_coef*TreeViewerUI::SIZE_COEF, 1.0), horizontalZoom);

    scale(horizontalZoom / horizontalScale, verticalZoom / verticalScale);
    verticalScale = verticalZoom;
    horizontalScale = horizontalZoom;
    updateActionsState();
}

void TreeViewerUI::mousePressEvent(QMouseEvent *e) {

    bool shiftPressed = e->modifiers() & Qt::ShiftModifier;
    bool leftButton = e->button() == Qt::LeftButton;

    if (leftButton) {
        setDragMode(QGraphicsView::ScrollHandDrag);
    }

    if (leftButton && !shiftPressed) {
        QList<QString> selection;
        root->setSelectedRecurs(false, true); // clear selection
    }

    if (e->button() == Qt::RightButton) {
        updateActionsState();
                       
        buttonPopup->popup(e->globalPos());
     
        e->accept();
        return;
    }
    
    QGraphicsView::mousePressEvent(e);
}

void TreeViewerUI::mouseReleaseEvent(QMouseEvent *e) {
    bool leftButton = e->button() == Qt::LeftButton;
    if (leftButton) {
        setDragMode(QGraphicsView::NoDrag);
    }
    e->accept();
}

void TreeViewerUI::resizeEvent(QResizeEvent *e) {
    QRectF rect = scene()->sceneRect();
    rect.setWidth(rect.width() / horizontalScale);
    rect.setHeight(rect.height() / verticalScale);
    rect.moveCenter(scene()->sceneRect().center());
    fitInView(rect, Qt::KeepAspectRatio);
    QGraphicsView::resizeEvent(e);
}

void TreeViewerUI::paint(QPainter &painter) {
    painter.setBrush(Qt::darkGray);
    painter.setFont(TreeViewerUtils::getFont());
    scene()->render(&painter);
}

void TreeViewerUI::updateRect() {
    QRectF rect = root->mapToScene(root->childrenBoundingRect().united(root->boundingRect())).boundingRect();
    rect.setLeft(rect.left() - MARGIN);
    rect.setRight(rect.right() - (labelsSettings.showNames ? 0 : maxNameWidth) + MARGIN);
    rect.setTop(rect.top() - MARGIN);
    rect.setBottom(rect.bottom() + legend->childrenBoundingRect().height() + MARGIN);
    legend->setPos(0, rect.bottom() - MARGIN);
    scene()->setSceneRect(rect);
}

void TreeViewerUI::sl_chrootTriggered() {

    //Q_ASSERT(0);
}

void TreeViewerUI::sl_swapTriggered() {
    foreach (QGraphicsItem *graphItem, this->items()) {
        GraphicsButtonItem *buttonItem = dynamic_cast<GraphicsButtonItem*>(graphItem);
        if (buttonItem && buttonItem->isSelectedTop()) {
            buttonItem->swapSiblings();
            break;
        }
    }
    qreal curHScale = horizontalScale;
    qreal curVScale = verticalScale;

    QTransform curTransform = viewportTransform();
    setTransformationAnchor(NoAnchor);

    redrawRectangularLayout(); 
    updateLayout();
    updateTreeSettings();

    setTransform(curTransform);
    horizontalScale = curHScale;
    verticalScale = curVScale;
    updateActionsState();
    setTransformationAnchor(AnchorUnderMouse);
}

void TreeViewerUI::collapseSelected() {
    phyObject->update();
    foreach (QGraphicsItem *graphItem, this->items()) {
        GraphicsButtonItem *buttonItem = dynamic_cast<GraphicsButtonItem*>(graphItem);

        if (buttonItem && buttonItem->isSelectedTop()) {
            buttonItem->collapse();
        }
    }
}

bool TreeViewerUI::isSelectedCollapsed(){
    phyObject->update();
    foreach (QGraphicsItem *graphItem, this->items()) {
        GraphicsButtonItem *buttonItem = dynamic_cast<GraphicsButtonItem*>(graphItem);

        if (buttonItem && buttonItem->isSelectedTop()) {
            return buttonItem->isCollapsed();
        }
    }
    return false;
}

void TreeViewerUI::sl_collapseTriggered() {
    collapseSelected();
}

void TreeViewerUI::sl_captureTreeTriggered() {
    Document* doc = phyObject->getDocument();
    const GUrl& url = doc->getURL();
    const QString& fileName = url.baseFileName();
    ExportImageDialog dialog(viewport(), viewport()->rect(), false, true, fileName);
    dialog.exec();
}

void TreeViewerUI::sl_exportTriggered() {
    QString fileName = phyObject->getDocument()->getName();
    QString format = "SVG - Scalable Vector Graphics (*.svg)";
    TreeViewerUtils::saveImageDialog(format, fileName, format);
    if (!fileName.isEmpty()) {
        QRect rect = scene()->sceneRect().toRect();
        rect.moveTo(0, 0);
        QSvgGenerator generator;
        generator.setFileName(fileName);
        generator.setSize(rect.size());
        generator.setViewBox(rect);

        QPainter painter;
        painter.begin(&generator);
        paint(painter);
        painter.end();
    }
}

void TreeViewerUI::sl_contTriggered(bool on) {
    if (on != labelsSettings.alignLabels) {
        labelsSettings.alignLabels = on;
        //emit si_settingsChanged();
        TreeLayout curLayout = layout;
        QStack<GraphicsBranchItem*> stack;
       
        if (on){
            labelsSettings.alignLabels = false;
            hide();                             //to remove blinking
            sl_rectangularLayoutTriggered();
            labelsSettings.alignLabels = true;
            stack.push(rectRoot);
        }else{
            stack.push(root);
            if (root != rectRoot) {
                stack.push(rectRoot);
            }
        }
        updateLabelsAlignment(on);

        switch (curLayout)
        {
        case TreeLayout_Circular:
            sl_circularLayoutTriggered();
            fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
            break;

        case TreeLayout_Unrooted:
            sl_unrootedLayoutTriggered();
            fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
            break;
        case TreeLayout_Rectangular:
            show();
            break;
        }
    }
}

void TreeViewerUI::sl_rectangularLayoutTriggered() {
    if (layout != TreeLayout_Rectangular) {
        swapAction->setEnabled(true);
        root->setSelectedRecurs(false, true); // clear selection

        layout = TreeLayout_Rectangular;
        emit si_settingsChanged();
        scene()->removeItem(root);
        if(!rectRoot){
            redrawRectangularLayout();
        }
        root = rectRoot;
        scene()->addItem(root);
        defaultZoom();
        updateRect();
        updateTreeSettings();
        fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
        onLayoutChanged(layout);
    }
}

void TreeViewerUI::sl_circularLayoutTriggered() {
    if (layout != TreeLayout_Circular) {
        root->setSelectedRecurs(false, true); // clear selection

        layout = TreeLayout_Circular;
        emit si_settingsChanged();
        updateTreeSettings();
        if(getScale() <= GraphicsRectangularBranchItem::DEFAULT_WIDTH){
            layoutTask = new CreateCircularBranchesTask(rectRoot, true);
        }else{
            layoutTask = new CreateCircularBranchesTask(rectRoot);
        }
        connect(layoutTask, SIGNAL(si_stateChanged()), SLOT(sl_layoutRecomputed()));
        TaskScheduler* scheduler = AppContext::getTaskScheduler();
        scheduler->registerTopLevelTask(layoutTask);
        onLayoutChanged(layout);
    }
}

void TreeViewerUI::sl_unrootedLayoutTriggered() {
    if (layout != TreeLayout_Unrooted) {
        root->setSelectedRecurs(false, true); // clear selection

        layout = TreeLayout_Unrooted;
        emit si_settingsChanged();
        updateTreeSettings();
        layoutTask = new CreateUnrootedBranchesTask(rectRoot);
        connect(layoutTask, SIGNAL(si_stateChanged()), SLOT(sl_layoutRecomputed()));
        TaskScheduler* scheduler = AppContext::getTaskScheduler();
        scheduler->registerTopLevelTask(layoutTask);
        onLayoutChanged(layout);
    }
}

void TreeViewerUI::sl_rectLayoutRecomputed() {
    bool taskIsFailed = layoutTask->getState() != Task::State_Finished || layoutTask->hasError();
    CHECK(!taskIsFailed, );

    GraphicsRectangularBranchItem* rootNode = dynamic_cast<GraphicsRectangularBranchItem*>(layoutTask->getResult());
    CHECK(NULL != rootNode,);
    rectRoot = rootNode;

    switch (layout)
    {
    case TreeLayout_Circular:
        layout = TreeViewerUI::TreeLayout_Rectangular;
        sl_circularLayoutTriggered();
        fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
        break;

    case TreeLayout_Unrooted:
        layout = TreeViewerUI::TreeLayout_Rectangular;
        sl_unrootedLayoutTriggered();
        fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
        break;
    case TreeLayout_Rectangular:
        scene()->removeItem(root);
        root = rootNode;
        scene()->addItem(root);
        defaultZoom();
        updateRect();

        updateTreeSettings();

        break;
    }
}

void TreeViewerUI::sl_layoutRecomputed() {
    if (layoutTask->getState() != Task::State_Finished || layoutTask->hasError()) {
        return;
    }

    scene()->removeItem(root);
    root = layoutTask->getResult();
    scene()->addItem(root);
    defaultZoom();
    updateRect();

    if (!labelsSettings.showNames || !labelsSettings.showDistances) {
        LabelTypes lt;
        if (!labelsSettings.showDistances) {
            lt |= LabelType_Distance;
        }
        if (!labelsSettings.showNames) {
            lt |= LabelType_SequnceName;
        }
        showLabels(lt);
    }
    show();
    
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}

void TreeViewerUI::showLabels(LabelTypes labelTypes) {
    QStack<GraphicsBranchItem*> stack;
    stack.push(root);
    if (root != rectRoot) {
        stack.push(rectRoot);
    }
    while (!stack.isEmpty()) {
        GraphicsBranchItem *node = stack.pop();
        if (labelTypes.testFlag(LabelType_SequnceName)) {
            if (node->getNameText() != NULL) {
                node->getNameText()->setVisible(labelsSettings.showNames);
            }
        }
        if (labelTypes.testFlag(LabelType_Distance)) {
            if (node->getDistanceText() != NULL) {
                node->getDistanceText()->setVisible(labelsSettings.showDistances);
            }
        }
        foreach (QGraphicsItem* item, node->childItems()) {
            GraphicsBranchItem *bitem = dynamic_cast<GraphicsBranchItem*>(item);
            if (bitem != NULL) {
                stack.push(bitem);
            }
        }
    }
}

void TreeViewerUI::sl_showNameLabelsTriggered(bool on) {
    if (on != labelsSettings.showNames) {
        QRectF rect = sceneRect();
        rect.setWidth(rect.width() + (on ? 1 : -1) * maxNameWidth);
        scene()->setSceneRect(rect);
        labelsSettings.showNames = on;
        showLabels(LabelType_SequnceName);
        if(curTreeViewer){
            curTreeViewer->getContAction()->setDisabled(!labelsSettings.showNames);
            if(!labelsSettings.showNames){
                sl_contTriggered(false);
            }else{
                if(curTreeViewer->getContAction()->isChecked()){
                    sl_contTriggered(true);
                }
            }
        }
    }
}

void TreeViewerUI::sl_showDistanceLabelsTriggered(bool on) {
    if (on != labelsSettings.showDistances) {
        labelsSettings.showDistances = on;
        showLabels(LabelType_Distance);
        //emit si_settingsChanged();
    }
}

void TreeViewerUI::sl_printTriggered() {
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QPainter painter(&printer);
    paint(painter);
}

void TreeViewerUI::sl_textSettingsTriggered(){
    TextSettingsDialog dialog(this, getTextSettings());
    if(dialog.exec()){
         updateSettings( dialog.getSettings() );
         if(labelsSettings.alignLabels){
             QStack<GraphicsBranchItem*> stack;
             stack.push(root);
             if (root != rectRoot) {
                 stack.push(rectRoot);
             }
             while (!stack.empty()) {
                 GraphicsBranchItem* item = stack.pop();
                 if (item->getNameText() == NULL) {
                     foreach (QGraphicsItem* citem, item->childItems()) {
                         GraphicsBranchItem* gbi = dynamic_cast<GraphicsBranchItem*>(citem);
                         if (gbi != NULL) {
                             stack.push(gbi);
                         }
                     }
                 } else {
                     item->setWidth(0);
                 }
             }
            updateRect();
            labelsSettings.alignLabels = false;
            sl_contTriggered(true);
            emit si_settingsChanged();
         }
    }
}

void TreeViewerUI::sl_treeSettingsTriggered(){
    TreeSettingsDialog dialog(this, getTreeSettings(), layout == TreeLayout_Rectangular);
    if(dialog.exec()){
        updateSettings(dialog.getSettings());
        emit si_settingsChanged();
    }
}

void TreeViewerUI::sl_zoomToSel(){
    QList<QGraphicsItem*> selectedItems = this->scene()->selectedItems();
    if(selectedItems.isEmpty()){
         zooming(ZOOM_COEF); 
    }else{
        GraphicsButtonItem *topButton = NULL;
        foreach (QGraphicsItem *graphItem, selectedItems) {
            GraphicsButtonItem *buttonItem = dynamic_cast<GraphicsButtonItem*>(graphItem);

            if (buttonItem && buttonItem->isSelectedTop()) {
                topButton = buttonItem;
                break;
            }
        }
        if(!topButton){
            zooming(ZOOM_COEF);
        }else{
            defaultZoom();
            QGraphicsItem *topItem = topButton->parentItem();
            QRectF rect = topItem->mapRectToScene(topItem->childrenBoundingRect());
            QRectF rect1 = scene()->sceneRect();
            qreal zoom1 = qMin(rect1.width() / rect.width(), rect1.height() / rect.height());
            zooming(zoom1);
            centerOn(rect.center());
        }
    }
}
void TreeViewerUI::sl_zoomOut(){
    zooming(1.0/(float)ZOOM_COEF);
}
void TreeViewerUI::sl_zoomToAll(){
    defaultZoom();
}

void TreeViewerUI::defaultZoom(){
    zooming(1.0/horizontalScale, 1.0/verticalScale);
}

void TreeViewerUI::redrawRectangularLayout(){
    int current = 0;
    qreal minDistance = -2, maxDistance = 0;
    GraphicsRectangularBranchItem* item = rectRoot;

    item->redrawBranches(current, minDistance, maxDistance, phyObject->getTree()->getRootNode());

    item->setWidthW(0);
    item->setDist(0);
    item->setHeightW(0);

    if(minDistance == 0){
        minDistance = GraphicsRectangularBranchItem::EPSILON;
    }
    if(maxDistance == 0){
        maxDistance = GraphicsRectangularBranchItem::EPSILON;
    }

    qreal minDistScale = GraphicsRectangularBranchItem::DEFAULT_WIDTH / (qreal)minDistance;
    qreal maxDistScale = GraphicsRectangularBranchItem::MAXIMUM_WIDTH / (qreal)maxDistance;

    qreal scale = qMin(minDistScale, maxDistScale);
    setScale(scale);
}

qreal TreeViewerUI::avgWidth(){
    qreal sumW = 0;
    int count = 0;

    QList<QGraphicsItem*> updatingItems = scene()->items();

    foreach (QGraphicsItem *graphItem, updatingItems) {

        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(graphItem);
        if (branchItem) {
            sumW += qAbs(branchItem->getDist());
            count++;
        }
    }

    return sumW/(qreal)count;
}

void TreeViewerUI::updateActionsState(){

    zoomToAction->setEnabled(horizontalScale < MAXIMUM_ZOOM* qMax(treeSettings.width_coef*TreeViewerUI::SIZE_COEF, 1.0));
    zoomOutAction->setEnabled(horizontalScale > MINIMUM_ZOOM);
    
    if(isSelectedCollapsed()){
        collapseAction->setText(QObject::tr("Expand"));
    }else{
        collapseAction->setText(QObject::tr("Collapse"));
    }

    QList<QGraphicsItem*> updatingItems = this->scene()->selectedItems();
    collapseAction->setEnabled(!updatingItems.isEmpty());
    swapAction->setEnabled(!updatingItems.isEmpty());
}

void TreeViewerUI::updateLayout()
{
    TreeLayout tmpL = layout;
    layout = TreeLayout_Rectangular;
    switch(tmpL){
        case TreeLayout_Circular:
            sl_circularLayoutTriggered();
            break;
        case TreeLayout_Unrooted:
            sl_unrootedLayoutTriggered();
            break;
        case TreeLayout_Rectangular:
            //here to please compiler
            break;
    }
}

void TreeViewerUI::updateLabelsAlignment(bool on)
{
    QStack<GraphicsBranchItem*> stack;
    stack.push(root);
    if (root != rectRoot) {
        stack.push(rectRoot);
    }

    while (!stack.empty()) {
        GraphicsBranchItem* item = stack.pop();
        if (item->getNameText() == NULL) {
            foreach (QGraphicsItem* citem, item->childItems()) {
                GraphicsBranchItem* gbi = dynamic_cast<GraphicsBranchItem*>(citem);
                if (gbi != NULL) {
                    stack.push(gbi);
                }
            }
        } else {
            qreal nWidth = 0;
            if(on){
                qreal scW = scene()->sceneRect().width();
                qreal scL = scene()->sceneRect().left();
                qreal textScPosX = item->getNameText()->scenePos().x();
                nWidth = scW + scL - textScPosX;
                if (labelsSettings.showNames){
                    qreal textBRW = item->getNameText()->boundingRect().width();
                    nWidth -= textBRW + GraphicsBranchItem::TextSpace;
                }
            }
            item->setWidth(nWidth);
        }
    }
    updateRect();
}

}//namespace
