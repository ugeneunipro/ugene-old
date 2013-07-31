/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "MSAEditorDataList.h"
#include "MSAEditor.h"
#include "MSAEditorSequenceArea.h"
#include "PhyTrees/MSAEditorTreeManager.h"

#include <U2Core/AppContext.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MAlignment.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/PhyTreeObject.h>

#include <U2Gui/GUIUtils.h>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QSplitter>

#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include <U2Algorithm/MSADistanceAlgorithm.h>

namespace U2 
{

const QString MSAEditorAlignmentDependentWidget::DataIsOutdatedMessage(QString("<FONT COLOR=#FF0000>%1</FONT>").arg(tr("Data is outdated")));
const QString MSAEditorAlignmentDependentWidget::DataIsValidMessage(QString("<FONT COLOR=#00FF00>%1</FONT>").arg(tr("Data is valid")));
const QString MSAEditorAlignmentDependentWidget::DataIsBeingUpdatedMessage(QString("<FONT COLOR=#0000FF>%1</FONT>").arg(tr("Data is being updated")));

MSAEditorSimilarityColumn::MSAEditorSimilarityColumn(MSAEditorUI* ui, QScrollBar* nhBar, const SimilarityStatisticsSettings* _settings)
: MSAEditorNameList(ui, nhBar), algo(NULL), autoUpdate(true) {
    newSettings = curSettings = *_settings;
    updateDistanceMatrix();
}

MSAEditorSimilarityColumn::~MSAEditorSimilarityColumn() {
    CHECK(NULL != algo, );
    delete algo;
}

QString MSAEditorSimilarityColumn::getTextForRow( int s ) {
    if (NULL == algo) {
        return tr("-");
    }

    const MAlignment& ma = editor->getMSAObject()->getMAlignment();
    const qint64 referenceRowId = editor->getReferenceRowId();
    if(MAlignmentRow::invalidRowId() == referenceRowId) {
        return tr("-");
    }

    U2OpStatusImpl os;
    const int refSequenceIndex = ma.getRowIndexByRowId(referenceRowId, os);
    CHECK_OP(os, QString());

    int sim = algo->getSimilarity(refSequenceIndex, s);
    CHECK(-1 != sim, tr("-"));
    const QString units = algo->areUsePercents() ? "%" : "";
    return QString("%1").arg(sim) + units;
}

QString MSAEditorSimilarityColumn::getSeqName(int s) {
    const MAlignment& ma = editor->getMSAObject()->getMAlignment();

    return ma.getRowNames().at(s);
}

void MSAEditorSimilarityColumn::setSettings(const UpdatedWidgetSettings* _settings) {
    const SimilarityStatisticsSettings* set= static_cast<const SimilarityStatisticsSettings*>(_settings);
    CHECK(NULL != set,);
    autoUpdate = set->autoUpdate;
    state = DataIsValid; 
    if(curSettings.algoName != set->algoName) {
        state = DataIsOutdated;
    }
    if(curSettings.excludeGaps != set->excludeGaps) {
        state = DataIsOutdated;
    }
    if(curSettings.usePercents != set->usePercents) {
        if(NULL != algo) {
            algo->showSimilarityInPercents(set->usePercents);
            updateContent();
        }
        curSettings.usePercents = set->usePercents;
    }
    newSettings = *set;
    if(autoUpdate && DataIsOutdated == state) {
        state = DataIsBeingUpdated;
        emit si_dataStateChanged(state);
        updateDistanceMatrix();
    }
    emit si_dataStateChanged(state);
}

void MSAEditorSimilarityColumn::updateDistanceMatrix() {
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    Task* createMatrix = new CreateDistanceMatrixTask(newSettings);
    connect(new TaskSignalMapper(createMatrix), SIGNAL(si_taskFinished(Task*)), this, SLOT(sl_createMatrixTaskFinished(Task*)));
    scheduler->registerTopLevelTask(createMatrix);
}

void MSAEditorSimilarityColumn::onAlignmentChanged(const MAlignment&, const MAlignmentModInfo&) {
    if(autoUpdate) {
        state = DataIsBeingUpdated;
        updateDistanceMatrix();
    }
    else {
        state = DataIsOutdated;
    }
    emit si_dataStateChanged(state);
}

void MSAEditorSimilarityColumn::sl_createMatrixTaskFinished(Task* t) {
    CreateDistanceMatrixTask* task = qobject_cast<CreateDistanceMatrixTask*> (t);
    if(NULL != task) {
        if(NULL != algo) {
            delete algo;
        }
        algo = task->getResult();
        if(NULL != algo) {
            algo->showSimilarityInPercents(newSettings.usePercents);
        }
    }
    updateContent();
    state = DataIsValid;
    curSettings = newSettings;
    emit si_dataStateChanged(state);
}

CreateDistanceMatrixTask::CreateDistanceMatrixTask(const SimilarityStatisticsSettings& _s) 
: Task(tr("Generate distance matrix"), TaskFlag_NoRun), s(_s), resMatrix(NULL) {
    SAFE_POINT(NULL != s.ma, QString("Incorrect MAlignment in MSAEditorSimilarityColumnTask ctor!"), );
    SAFE_POINT(NULL != s.ui, QString("Incorrect MSAEditorUI in MSAEditorSimilarityColumnTask ctor!"), );
    setVerboseLogMode(true);
}

void CreateDistanceMatrixTask::prepare() {
    MSADistanceAlgorithmFactory* factory = AppContext::getMSADistanceAlgorithmRegistry()->getAlgorithmFactory(s.algoName);
    CHECK(NULL != factory,);
    if(s.excludeGaps){
        factory->setFlag(DistanceAlgorithmFlag_ExcludeGaps);
    }else{
        factory->resetFlag(DistanceAlgorithmFlag_ExcludeGaps);
    }

    MSADistanceAlgorithm* algo = factory->createAlgorithm(s.ma->getMAlignment());
    CHECK(NULL != algo,);
    addSubTask(algo);
}

QList<Task*> CreateDistanceMatrixTask::onSubTaskFinished(Task* subTask){ 
    QList<Task*> res;
    MSADistanceAlgorithm* algo = qobject_cast<MSADistanceAlgorithm*>(subTask);
    MSADistanceMatrix *matrix = new MSADistanceMatrix(algo, s.usePercents && s.excludeGaps);
    if(NULL != algo) {
        resMatrix = matrix;
    }
    return res;
}
MSAEditorAlignmentDependentWidget::MSAEditorAlignmentDependentWidget(UpdatedWidgetInterface* _contentWidget) 
: contentWidget(_contentWidget), automaticUpdating(true){
    SAFE_POINT(NULL != _contentWidget, QString("Argument is NULL in constructor MSAEditorAlignmentDependentWidget()"),);

    settings = &contentWidget->getSettings();
    connect(settings->ma, SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)),
        this, SLOT(sl_onAlignmentChanged(const MAlignment&, const MAlignmentModInfo&)));
    connect(dynamic_cast<QObject*>(contentWidget), SIGNAL(si_dataStateChanged(DataState)),
        this, SLOT(sl_onDataStateChanged(DataState)));
    connect(settings->ui->getEditor(), SIGNAL(si_fontChanged(const QFont&)), SLOT(sl_onFontChanged(const QFont&)));

    createWidgetUI();

    setSettings(settings);
}
void MSAEditorAlignmentDependentWidget::createWidgetUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout();

    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    createHeaderWidget();

    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(contentWidget->getWidget());
    this->setLayout(mainLayout);
}
void MSAEditorAlignmentDependentWidget::createHeaderWidget() {
    QVBoxLayout* headerLayout = new QVBoxLayout();
    headerLayout->setMargin(0);
    headerLayout->setSpacing(0);

    nameWidget.setText("%");
    nameWidget.setAlignment(Qt::AlignCenter);
    nameWidget.setFont(settings->ui->getEditor()->getFont());
    headerLayout->addWidget(&nameWidget);

    state = DataIsValid;
    headerWidget = new MSAWidget(settings->ui);
    headerWidget->setLayout(headerLayout);
}

void MSAEditorAlignmentDependentWidget::setSettings(const UpdatedWidgetSettings* _settings) {
    settings = _settings; 
    automaticUpdating = settings->autoUpdate;
    contentWidget->setSettings(settings);
}


void MSAEditorAlignmentDependentWidget::sl_onAlignmentChanged(const MAlignment& maBefore, const MAlignmentModInfo& modInfo) {
    contentWidget->onAlignmentChanged(maBefore, modInfo);
}
void MSAEditorAlignmentDependentWidget::sl_onUpdateButonPressed() {
    contentWidget->updateWidget();
}
void MSAEditorAlignmentDependentWidget::sl_onDataStateChanged(DataState newState) {
    state = DataIsValid;
    switch(newState) {
        case DataIsValid:
            statusBar.setText(DataIsValidMessage);
            updateButton.setEnabled(false);
            break;
        case DataIsBeingUpdated:
            statusBar.setText(DataIsBeingUpdatedMessage);
            updateButton.setEnabled(false);
            break;
        case DataIsOutdated:
            statusBar.setText(DataIsOutdatedMessage);
            updateButton.setEnabled(true);
            break;
    }
}
void MSAEditorAlignmentDependentWidget::sl_onFontChanged( const QFont& font) {
    nameWidget.setFont(font);
}

UpdatedTabWidget::UpdatedTabWidget(QWidget* parent)
: QTabWidget(parent) {
    tabBar()->installEventFilter(this);
    setTabsClosable(true);
    buildMenu();
    connect(this, SIGNAL(tabCloseRequested(int)), SLOT(sl_onTabCloseRequested(int)));
}
void UpdatedTabWidget::sl_onTabCloseRequested(int index) {
    emit si_onTabCloseRequested(widget(index));
}

void UpdatedTabWidget::buildMenu()
{
    tabsMenu = new QMenu(this);
    refreshAction = tabsMenu->addAction(tr("Refresh tab"));
    connect(refreshAction, SIGNAL(triggered(bool)), SLOT(sl_refreshTriggered()));
    refreshAllAction = tabsMenu->addAction(tr("Refresh all tabs"));
    connect(refreshAllAction, SIGNAL(triggered(bool)), SLOT(sl_refreshAllTriggered()));

    tabsMenu->addSeparator();
    
    closeOtherTabs = tabsMenu->addAction(tr("Close other tabs"));
    connect(closeOtherTabs, SIGNAL(triggered(bool)), SLOT(sl_closeOtherTabsTriggered()));
    closeAllTabs = tabsMenu->addAction(tr("Close all tabs"));
    connect(closeAllTabs, SIGNAL(triggered(bool)), SLOT(sl_closeAllTabsTriggered()));
    closeTab = tabsMenu->addAction(tr("Close tab"));
    connect(closeTab, SIGNAL(triggered(bool)), SLOT(sl_closeTabTriggered()));

    tabsMenu->addSeparator();

    addHorizontalSplitter = tabsMenu->addAction(tr("Add horizontal splitter"));
    connect(addHorizontalSplitter, SIGNAL(triggered(bool)), SLOT(sl_addHSplitterTriggered()));
    addVerticalSplitter = tabsMenu->addAction(tr("Add vertical splitter"));
    connect(addVerticalSplitter, SIGNAL(triggered(bool)), SLOT(sl_addVSplitterTriggered()));
}

void UpdatedTabWidget::updateActionsState() {

}


int UpdatedTabWidget::addTab(QWidget *page, const QString &label ) {
    return QTabWidget::addTab(page, label);
}

bool UpdatedTabWidget::eventFilter(QObject *target, QEvent *event) 
{ 
    if (target == tabBar()) { 
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event); 
        if(NULL == mouseEvent) {
            return QTabWidget::eventFilter(target, event);
        }
        if (mouseEvent->button() == Qt::RightButton) { 
            updateActionsState();
            menuPos = mouseEvent->pos();
            tabsMenu->popup( mouseEvent->globalPos());
            return true;
        } 
    } 
    return QTabWidget::eventFilter(target, event); 
}

void UpdatedTabWidget::sl_refreshTriggered() {

}
void UpdatedTabWidget::sl_refreshAllTriggered() {

}
void UpdatedTabWidget::sl_closeOtherTabsTriggered() {
    int selectedIndex = tabBar()->tabAt(menuPos);
    int i = 0, start = 0;
    while(1 < count()) {
        if(i == selectedIndex) {
            start = 1;
            continue;
        }
        delete widget(start);
        removeTab(start);
        i++;
    }
}
void UpdatedTabWidget::sl_closeAllTabsTriggered() {
    while(count()) {
        delete widget(0);
        removeTab(0);
    }
}
void UpdatedTabWidget::sl_closeTabTriggered() {
    int index = tabBar()->tabAt(menuPos);
    QWidget* pageWidget = widget(index);
    removeTab(index);
    delete pageWidget;
}

void UpdatedTabWidget::sl_addHSplitterTriggered() {
    int index = tabBar()->tabAt(menuPos);
    QWidget* pageWidget = widget(index);
    if(pageWidget) {
        emit si_addSplitterTriggered(Qt::Vertical, pageWidget, tabText(index));
    }
}
void UpdatedTabWidget::sl_addVSplitterTriggered() {
    int index = tabBar()->tabAt(menuPos);
    QWidget* pageWidget = widget(index);
    if(pageWidget) {
        emit si_addSplitterTriggered(Qt::Horizontal, pageWidget, tabText(index));
    }
}
TabWidgetArea::TabWidgetArea(QWidget* parent) 
: QWidget(parent), tabsCount(0), currentWidget(NULL), currentLayout(NULL) {
}
void TabWidgetArea::initialize() {
    currentWidget = createTabWidget();
    currentLayout = new QVBoxLayout();
    currentLayout->setMargin(0);
    currentLayout->setSpacing(0);
    currentLayout->addWidget(currentWidget);
    setLayout(currentLayout);
}
void TabWidgetArea::sl_onWidgetSelected(UpdatedTabWidget* widget) {
    currentWidget = widget;
}
UpdatedTabWidget* TabWidgetArea::createTabWidget() {
    UpdatedTabWidget* widget = new UpdatedTabWidget(this);
    connect(widget, SIGNAL(si_onTabCloseRequested(QWidget*)), SLOT(sl_onTabCloseRequested(QWidget*)));
    connect(widget, SIGNAL(si_addSplitterTriggered(Qt::Orientation, QWidget*, const QString &)), SLOT(sl_addSplitter(Qt::Orientation, QWidget*, const QString &)));
    tabWidgets << widget;
    return widget;
}
TabWidgetArea::~TabWidgetArea() {
    foreach(QSplitter* splitter, splitters) {
        delete splitter;
    }
}
void TabWidgetArea::sl_onTabCloseRequested(QWidget* page) {
    deleteTab(page);
}
void TabWidgetArea::deleteTab(QWidget *page) {
    CHECK(-1 != currentWidget->indexOf(page),);
    if(NULL != page) {
        delete page;
        tabsCount--;
        emit si_tabsCountChanged(tabsCount);
    }
}
void TabWidgetArea::addTab(QWidget *page, const QString &label) {
    currentWidget->addTab(page, label);
    widgetLocations[page] = currentWidget;
    tabsCount++;
    emit si_tabsCountChanged(tabsCount);
}
void TabWidgetArea::sl_addSplitter(Qt::Orientation splitterOrientation, QWidget* page, const QString &label) {
    QSplitter* newSplitter = new QSplitter(splitterOrientation, this);
    QWidget*   oldArea = new QWidget(this);
    oldArea->setLayout(currentLayout);
    UpdatedTabWidget* newTabs = createTabWidget();
    newTabs->addTab(page, label);
    newSplitter->addWidget(oldArea); 
    newSplitter->addWidget(newTabs);
    splitters << newSplitter;

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(newSplitter);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    currentLayout = layout;
}
void TabWidgetArea::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), Qt::white);
}

MSAEditorUpdatedTabWidget::MSAEditorUpdatedTabWidget(MSAEditor* _msa, QWidget* parent)
:   UpdatedTabWidget(parent), msa(_msa){
    addTabButton = new QPushButton(QIcon(":/core/images/add_tree.png"), "", this);
    addTabButton->setToolTip(tr("Add existing tree"));
    setCornerWidget(addTabButton);
    connect(addTabButton, SIGNAL(clicked(bool)), this, SLOT(sl_addTabTriggered()));
}
void MSAEditorUpdatedTabWidget::sl_addTabTriggered() {
    emit si_widgetSelected(this);

    msa->getTreeManager()->showAddTreeDialog();
}

void MSAEditorUpdatedTabWidget::addExistingTree() {
    msa->getTreeManager()->addTreeToMSA();
}

MSAEditorTabWidgetArea::MSAEditorTabWidgetArea(MSAEditor* _msa, QWidget* parent )
: TabWidgetArea(parent), addTabButton(NULL), msa(_msa) {
    initialize();
}

UpdatedTabWidget* MSAEditorTabWidgetArea::createTabWidget()
{
    MSAEditorUpdatedTabWidget* widget = new MSAEditorUpdatedTabWidget(msa, this);
    connect(widget, SIGNAL(si_onTabCloseRequested(QWidget*)), SLOT(sl_onTabCloseRequested(QWidget*)));
    connect(widget, SIGNAL(si_addSplitterTriggered(Qt::Orientation, QWidget*, const QString &)), SLOT(sl_addSplitter(Qt::Orientation, QWidget*, const QString &)));
    connect(widget, SIGNAL(si_widgetSelected(UpdatedTabWidget*)), this, SLOT(sl_onWidgetSelected(UpdatedTabWidget*)));
    tabWidgets << widget;
    return widget;
}

void MSAEditorTabWidgetArea::deleteTab(QWidget *page) {
    GObjectViewWindow* win = qobject_cast<GObjectViewWindow*>(page); 
    const GObject* obj = win->getObjectView()->getObjects().at(0);
    Document* doc = obj->getDocument();
    GObjectReference treeRef(doc->getURLString(), "", GObjectTypes::PHYLOGENETIC_TREE);
    treeRef.objName = obj->getGObjectName();
    msa->getMSAObject()->removeObjectRelation(GObjectRelation(treeRef, GObjectRelationRole::PHYLOGENETIC_TREE));

    TabWidgetArea::deleteTab(page);
}

} //namespace


