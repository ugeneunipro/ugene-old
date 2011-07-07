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
#include "TreeViewer.h"
#include "TreeViewerFactory.h"
#include "GraphicsBranchItem.h"
#include "GraphicsButtonItem.h"
#include "TreeViewerUtils.h"
#include "CreateBranchesTask.h"
#include "CreateCircularBranchesTask.h"
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

#include <U2Gui/GUIUtils.h>
#include <U2Gui/ExportImageDialog.h>

#include <U2Misc/HBar.h>
#include <U2Misc/DialogUtils.h>
#include <U2View/CreatePhyTreeDialogController.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>

#include <QtCore/QStack>

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
    ui(NULL),
    root(_root),
    scale(s)
{
    phyObject = qobject_cast<PhyTreeObject*>(obj);
    objects.append(phyObject);
    requiredObjects.append(phyObject);
    onObjectAdded(phyObject);
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

qreal TreeViewer::getZoom() const {

    return ui->getZoom();
}

void TreeViewer::setZoom(qreal z) {

    ui->setZoom(z);
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

    circularLayoutAction = new QAction(tr("Circular"), layoutGroup);
    circularLayoutAction->setCheckable(true);

    unrootedLayoutAction = new QAction(tr("Unrooted"), layoutGroup);
    unrootedLayoutAction->setCheckable(true);

    // Branch Settings
    branchesSettingsAction = new QAction(QIcon(":core/images/color_wheel.png"), tr("Branch Settings..."), ui);

    // Show Labels
    nameLabelsAction = new QAction(tr("Show Names"), ui);
    nameLabelsAction->setCheckable(true);
    nameLabelsAction->setChecked(true);

    distanceLabelsAction = new QAction(tr("Show Distances"), ui);
    distanceLabelsAction->setCheckable(true);
    distanceLabelsAction->setChecked(true);

    // Formatting
    textSettingsAction = new QAction(QIcon(":core/images/font.png"), tr("Formatting..."), ui);

    // Align Labels
    // Note: the icon is truncated to 15 px height to look properly in the main menu when it is checked
    contAction = new QAction(QIcon(":core/images/align_tree_labels.png"), tr("Align Labels"), ui);
    contAction->setCheckable(true);

    // Zooming
    zoomToSelAction = new QAction(QIcon(":core/images/zoom_in.png"), tr("Zoom In") , ui);
    zoomOutAction = new QAction(QIcon(":core/images/zoom_out.png"), tr("Zoom Out"), ui);
    zoomToAllAction = new QAction(QIcon(":core/images/zoom_whole.png"), tr("Reset Zooming"), ui);

    // Print Tree
    printAction = new QAction(QIcon(":/core/images/printer.png"), tr("Print Tree..."), ui);

    // Screen Capture
    captureTreeAction = new QAction(tr("Screen Capture..."), ui);
    exportAction = new QAction(tr("Whole Tree as SVG..."), ui);
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
    tb->addWidget(layoutButton);

    // Branch Settings
    tb->addAction(branchesSettingsAction);

    // Labels and Text Settings
    tb->addSeparator();
    QToolButton* showLabelsButton = new QToolButton();
    QMenu* showLabelsMenu = new QMenu(tr("Show Labels"), ui);
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
    tb->addWidget(cameraButton);
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
    m->addMenu(cameraMenu);

    m->addSeparator();

    GObjectView::buildStaticMenu(m);
    GUIUtils::disableEmptySubmenus(m);
}

QWidget* TreeViewer::createWidget() {
    assert(ui == NULL);
    ui = new TreeViewerUI(this);

    return ui;
}

void TreeViewer::onObjectRenamed(GObject* obj, const QString& oldName) {
    // update title
    OpenTreeViewerTask::updateTitle(this);
}

////////////////////////////
// TreeViewerUI

const qreal TreeViewerUI::ZOOM_COEF = 1.2;
const qreal TreeViewerUI::MINIMUM_ZOOM = 1.0;
const qreal TreeViewerUI::MAXIMUM_ZOOM = 10.0; 
const int TreeViewerUI::MARGIN = 10;
const qreal TreeViewerUI::SIZE_COEF = 0.1;


TreeViewerUI::TreeViewerUI(TreeViewer* treeViewer): phyObject(treeViewer->getPhyObject()), root(treeViewer->getRoot()), rectRoot(treeViewer->getRoot()), layout(TreeLayout_Rectangular), curTreeViewer(NULL) {
    curTreeViewer = treeViewer;
    contEnabled = false;
    showDistanceLabels = true;
    showNameLabels = true;
    maxNameWidth = 0;
    zoom = 1.0;

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

    buttonPopup->addAction(zoomToAction);

    collapseAction = buttonPopup->addAction(QObject::tr("Collapse"));
    connect(collapseAction, SIGNAL(triggered(bool)), SLOT(sl_collapseTriggered()));

    buttonPopup->addAction(setColorAction);

    QMenu* cameraMenu = new QMenu(tr("Export Tree Image"), this);
    cameraMenu->addAction(captureAction);
    cameraMenu->addAction(exportAction);
    cameraMenu->setIcon(QIcon(":/core/images/cam2.png"));
    buttonPopup->addMenu(cameraMenu);

    updateActionsState();
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

void TreeViewerUI::sl_setSettingsTriggered() {

    sl_branchSettings();
}

void TreeViewerUI::sl_branchSettings() {

    BranchSettingsDialog d(this, getBranchSettings());
    if (d.exec()) {
        updateSettings( d.getSettings() );
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
void TreeViewerUI::updateTreeSettings(){

    qreal avgW = 0;
    if(treeSettings.type == TreeSettings::CLADOGRAM){
        avgW = avgWidth(); 
        legend->setVisible(false);
    }else{
         legend->setVisible(true);
    }
    
    QStack<GraphicsBranchItem*> stack;
    stack.push(rectRoot);
    while (!stack.empty()) {
        GraphicsBranchItem *item = stack.pop();
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
                item->setWidth(avgW * getScale()* coef);
                break;
            default:
                assert(false && "Unexpected tree type value");
                break;
            }
        }
        foreach (QGraphicsItem* ci, item->childItems()) {
            GraphicsBranchItem* gbi = dynamic_cast<GraphicsBranchItem*>(ci);
            if (gbi != NULL) {
                stack.push(gbi);
            }
        }
    }
    updateRect();
    scene()->update();

    if(contEnabled){
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
        contEnabled = false;
        sl_contTriggered(true);
    }

    defaultZoom();
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}


#define BRANCH_COLOR        QString("branch_color")
#define BRANCH_THICKNESS    QString("branch_thickness")

QVariantMap TreeViewerUI::getSettingsState() const {

    QVariantMap m;

    int i=0;
    foreach (QGraphicsItem *graphItem, this->items()) {
        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(graphItem);
        if (branchItem) {
            m[BRANCH_COLOR + i] = branchItem->settings.branchColor;
            m[BRANCH_THICKNESS + i] = branchItem->settings.branchThickness;
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
            BranchSettings branchSettings = branchItem->settings;

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
    if(newzoom < 0){
        return ;
    }
    newzoom = zoom * newzoom;
    newzoom = qMax(MINIMUM_ZOOM, newzoom);
    newzoom = qMin(MAXIMUM_ZOOM * qMax(treeSettings.width_coef*TreeViewerUI::SIZE_COEF, 1.0), newzoom);
    scale(newzoom / zoom, newzoom / zoom);
    zoom = newzoom;   
    updateActionsState();
}

void TreeViewerUI::mousePressEvent(QMouseEvent *e) {
    bool shiftPressed = e->modifiers() & Qt::ShiftModifier;
    bool leftButton = e->button() == Qt::LeftButton;

    if (leftButton) {
        setDragMode(QGraphicsView::ScrollHandDrag);
    }

    if (leftButton && !shiftPressed) {
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
    rect.setWidth(rect.width() / zoom);
    rect.setHeight(rect.height() / zoom);
    rect.moveCenter(scene()->sceneRect().center());
    fitInView(rect, Qt::KeepAspectRatio);
    QGraphicsView::resizeEvent(e);
}

// needed to export tree
void TreeViewerUI::paint(QPainter &painter) {
    painter.setBrush(Qt::darkGray);
    painter.setFont(TreeViewerUtils::getFont());
    scene()->render(&painter);
}

void TreeViewerUI::updateRect() {
    QRectF rect = root->mapToScene(root->childrenBoundingRect().united(root->boundingRect())).boundingRect();
    rect.setLeft(rect.left() - MARGIN);
    rect.setRight(rect.right() - (showNameLabels ? 0 : maxNameWidth) + MARGIN);
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
    redrawRectangularLayout(); 
    updateLayout();
    updateTreeSettings();
}

void TreeViewerUI::collapseSelected() {

    foreach (QGraphicsItem *graphItem, this->items()) {
        GraphicsButtonItem *buttonItem = dynamic_cast<GraphicsButtonItem*>(graphItem);

        if (buttonItem && buttonItem->isSelectedTop()) {
            buttonItem->collapse();
        }
    }
}

bool TreeViewerUI::isSelectedCollapsed(){
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
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ExportImageDialog dialog(this,this->rect(),false,true,phyObject->getDocument()->getURL().baseFileName());
    dialog.exec();

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
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
    if (on != contEnabled) {
        contEnabled = on;
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
                    if (showNameLabels){
                        qreal textBRW = item->getNameText()->boundingRect().width();
                        nWidth -= textBRW + GraphicsBranchItem::TextSpace;
                    }
                }
// 
//                 //test
//                 QGraphicsItem* pitem =  item->parentItem();
//                 GraphicsBranchItem* grPItem = dynamic_cast<GraphicsBranchItem*>(pitem);
//                 if(grPItem){
//                     if(grPItem->getDistanceText()!= NULL && grPItem->getDistanceText()->text() == ""){
//                         nWidth+=8;
//                     }
//                 }
//                 //test

                item->setWidth(nWidth);
            }
        }
        updateRect();
    }
}

void TreeViewerUI::sl_rectangularLayoutTriggered() {
    if (layout != TreeLayout_Rectangular) {
        swapAction->setEnabled(true);
        root->setSelectedRecurs(false, true); // clear selection

        layout = TreeLayout_Rectangular;
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
    }
}

void TreeViewerUI::sl_circularLayoutTriggered() {
    if (layout != TreeLayout_Circular) {
        //swapAction->setEnabled(false);
        root->setSelectedRecurs(false, true); // clear selection

        layout = TreeLayout_Circular;
        updateTreeSettings();
        layoutTask = new CreateCircularBranchesTask(rectRoot);
        connect(layoutTask, SIGNAL(si_stateChanged()), SLOT(sl_layoutRecomputed()));
        TaskScheduler* scheduler = AppContext::getTaskScheduler();
        scheduler->registerTopLevelTask(layoutTask);
    }
}

void TreeViewerUI::sl_unrootedLayoutTriggered() {
    if (layout != TreeLayout_Unrooted) {
       // swapAction->setEnabled(false);
        root->setSelectedRecurs(false, true); // clear selection

        layout = TreeLayout_Unrooted;
        updateTreeSettings();
        layoutTask = new CreateUnrootedBranchesTask(rectRoot);
        connect(layoutTask, SIGNAL(si_stateChanged()), SLOT(sl_layoutRecomputed()));
        TaskScheduler* scheduler = AppContext::getTaskScheduler();
        scheduler->registerTopLevelTask(layoutTask);
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

    if (!showNameLabels || !showDistanceLabels) {
        LabelTypes lt;
        if (!showDistanceLabels) {
            lt |= LabelType_Distance;
        }
        if (!showNameLabels) {
            lt |= LabelType_SequnceName;
        }
        showLabels(lt);
    }
    
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
                node->getNameText()->setVisible(showNameLabels);
            }
        }
        if (labelTypes.testFlag(LabelType_Distance)) {
            if (node->getDistanceText() != NULL) {
                node->getDistanceText()->setVisible(showDistanceLabels);
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
    if (on != showNameLabels) {
        QRectF rect = sceneRect();
        rect.setWidth(rect.width() + (on ? 1 : -1) * maxNameWidth);
        scene()->setSceneRect(rect);
        showNameLabels = on;
        showLabels(LabelType_SequnceName);
        if(curTreeViewer){
            curTreeViewer->getContAction()->setDisabled(!showNameLabels);
            if(!showNameLabels){
                sl_contTriggered(false);
            }else{
                if(curTreeViewer->getContAction()->isChecked()){
                    sl_contTriggered(true);
                }
            }
        }else{
            if (contEnabled) {
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
                        item->setWidth(item->getWidth() + (on ? 1 : -1) * (maxNameWidth - item->getNameText()->boundingRect().width() - 2 * GraphicsBranchItem::TextSpace));
                    }
                }
            }
        }
    }
}

void TreeViewerUI::sl_showDistanceLabelsTriggered(bool on) {
    if (on != showDistanceLabels) {
        showDistanceLabels = on;
        showLabels(LabelType_Distance);
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
         if(contEnabled){
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
            contEnabled = false;
            sl_contTriggered(true);
         }
    }
}

void TreeViewerUI::sl_treeSettingsTriggered(){
    TreeSettingsDialog dialog(this, getTreeSettings(), layout == TreeLayout_Rectangular);
    if(dialog.exec()){
        updateSettings(dialog.getSettings());
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
    zooming(1.0/zoom);
}

void TreeViewerUI::redrawRectangularLayout(){
 
    //if(layout == TreeLayout_Rectangular){
        int current = 0;
        qreal minDistance = -2, maxDistance = 0;
        GraphicsRectangularBranchItem* item = rectRoot;

        item->redrawBranches(current, minDistance, maxDistance, phyObject->getTree()->rootNode);

        item->setWidthW(0);
        item->setDist(0);
        item->setHeightW(0);

        if(minDistance == 0){
            minDistance = 1;
        }
        if(maxDistance == 0){
            maxDistance = 1;
        }

        qreal minDistScale = GraphicsRectangularBranchItem::DEFAULT_WIDTH / (qreal)minDistance;
        qreal maxDistScale = GraphicsRectangularBranchItem::MAXIMUM_WIDTH / (qreal)maxDistance;

        qreal scale = qMin(minDistScale, maxDistScale);
        setScale(scale);
    //}
    
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

    zoomToAction->setEnabled(zoom < MAXIMUM_ZOOM* qMax(treeSettings.width_coef*TreeViewerUI::SIZE_COEF, 1.0));
    zoomOutAction->setEnabled(zoom > MINIMUM_ZOOM);
    
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

}//namespace
