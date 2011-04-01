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

#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>
#include <QtGui/QSizePolicy>
#include <QtGui/QPixmap>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObject.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/BioStruct3DObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>
#include <U2Gui/ProjectTreeController.h>
#include <U2Misc/HBar.h>
#include <U2View/AnnotatedDNAView.h>

#include "BioStruct3DSplitter.h"
#include "BioStruct3DGLWidget.h"
#include "GLFrameManager.h"


namespace U2 { 

#define HEADER_HEIGHT 24
#define MAX_SPLITTER_HEIGHT 1000

BioStruct3DSplitter::BioStruct3DSplitter(QAction* _closeAction, AnnotatedDNAView* view) 
: ADVSplitWidget(view), glFrameManager(new GLFrameManager), parentSplitter(NULL) 
{
    closeAction = _closeAction;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,3,0);
    splitter = new QSplitter(Qt::Horizontal);
    splitterHeight = 400;
    setMinimumHeight(HEADER_HEIGHT);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    setAcceptDrops(true);
    header = new SplitterHeaderWidget(this);
    layout->addWidget(header);
    layout->addWidget(splitter);
    isViewCollapsed = false;

    setLayout(layout);
}

bool BioStruct3DSplitter::acceptsGObject( GObject* obj )
{
    if (obj->getGObjectType() == GObjectTypes::BIOSTRUCTURE_3D) {
        return true;
    } else {
        return false;
    }

}

BioStruct3DGLWidget* BioStruct3DSplitter::addBioStruct3DGLWidget( BioStruct3DObject* bioStructObj )
{
    if (isViewCollapsed) {
        adaptSize(1);
    }

    BioStruct3DGLWidget* glWidget = new BioStruct3DGLWidget(bioStructObj, dnaView, getGLFrameManager(), this);
    glWidget->installEventFilter(this);
    // Add widget to splitter
    biostrucViewMap.insert(bioStructObj, glWidget);
    splitter->addWidget(glWidget);

    emit si_bioStruct3DGLWidgetAdded(glWidget);

    return glWidget;
}

bool BioStruct3DSplitter::eventFilter(QObject* o, QEvent* e) {
    Q_UNUSED(o); Q_UNUSED(e);
#ifdef Q_WS_X11
    BioStruct3DGLWidget* glw = qobject_cast<BioStruct3DGLWidget*>(o);
    //first variant of fix of QT4 bug: GL widget is frozen after minimize/maximize
    if (e->type() == QEvent::Hide) {
        glw->makeCurrent(); //these 2 lines can be removed
        glFinish();
        return true;
    }
    //second variant of fix of QT4 bug: GL widget is frozen after minimize/maximize
    if (e->type() == QEvent::WindowActivate) {
    //  ((QGLContext*)glw->context())->create();
    }
#endif
    return false;
}

void BioStruct3DSplitter::addObject(BioStruct3DObject* bioStructObj) {
    setVisible(true);
    addBioStruct3DGLWidget(bioStructObj);
}

void BioStruct3DSplitter::dragEnterEvent(QDragEnterEvent *event) 
{
    const QMimeData* md = event->mimeData();
    const GObjectMimeData* gomd = qobject_cast<const GObjectMimeData*>(md);
    if (gomd != NULL) {
        GObject* obj = gomd->objPtr.data();
        if (obj->getGObjectType() == GObjectTypes::BIOSTRUCTURE_3D) {
            BioStruct3DObject* bioStrucObj = qobject_cast<BioStruct3DObject*>(gomd->objPtr.data());
            if (biostrucViewMap.contains(bioStrucObj)) 
                event->acceptProposedAction();
        }
    }
}

void BioStruct3DSplitter::dropEvent(QDropEvent *event) 
{
    const GObjectMimeData* gomd = qobject_cast<const GObjectMimeData*>(event->mimeData());
    BioStruct3DObject* bioStrucObj = qobject_cast<BioStruct3DObject*>(gomd->objPtr.data());
    Q_ASSERT(bioStrucObj != NULL);
    addBioStruct3DGLWidget(bioStrucObj);

}

#define SPLITTER_STATE_MAP_NAME  "BIOSTRUCT3DSPLITTER_MAP"
#define WIDGET_STATE_LIST_NAME  "WIDGET_STATE_MAP"
#define OBJECT_ID_NAME "OBJECT_ID"

BioStruct3DSplitter::~BioStruct3DSplitter() {
    uiLog.trace("BioStruct3DSplitter deleted");
}

void BioStruct3DSplitter::saveState( QVariantMap& m )
{
    QVariantMap splitterStateMap = m.value(SPLITTER_STATE_MAP_NAME).toMap();
    QVariantList glWidgetStateList = m.value(WIDGET_STATE_LIST_NAME).toList();
    
    foreach (BioStruct3DGLWidget* glWidget, biostrucViewMap) {
        QVariantMap widgetState = glWidget->getState();
        glWidgetStateList.append(widgetState);
    }

    splitterStateMap[WIDGET_STATE_LIST_NAME] = glWidgetStateList;
    m[SPLITTER_STATE_MAP_NAME] = splitterStateMap;
                        
}


void BioStruct3DSplitter::updateState( const QVariantMap& m )
{
    const QVariantList glWidgetStateList = m.value(SPLITTER_STATE_MAP_NAME).toMap().value(WIDGET_STATE_LIST_NAME).toList();
    
    foreach (BioStruct3DGLWidget* glWidget, biostrucViewMap) {
        delete glWidget;
    }
    biostrucViewMap.clear();
      
    QListIterator<QVariant> iter(glWidgetStateList);
    iter.toBack();
    while (iter.hasPrevious()) {
        QVariantMap state = iter.previous().toMap();
        QString objName = state.value(OBJECT_ID_NAME).value<QString>();
        BioStruct3DObject* obj = findBioStruct3DObjByName(objName);
        if (obj == NULL)
            continue;
        BioStruct3DGLWidget* glWidget = addBioStruct3DGLWidget(obj);
        glWidget->setState(state);
    }
}

bool BioStruct3DSplitter::removeObject(BioStruct3DObject* obj) {
    QMultiMap<BioStruct3DObject*, BioStruct3DGLWidget*>::iterator i = biostrucViewMap.find(obj);
    while (i != biostrucViewMap.end() && i.key() == obj) {
        BioStruct3DGLWidget* child = (*i);
        delete child;
        ++i;
    }
    biostrucViewMap.remove(obj);
    bool isEmpty = biostrucViewMap.isEmpty();
    return isEmpty;
}


BioStruct3DObject* BioStruct3DSplitter::findBioStruct3DObjByName( const QString& objName )
{
    // TODO: optimize this by caching objects
    QList<Document*> docs = AppContext::getProject()->getDocuments();
    foreach (Document* doc, docs) {
        QList<GObject*> biostructObjs = doc->findGObjectByType(GObjectTypes::BIOSTRUCTURE_3D);
        if (!biostructObjs.empty()) {
            Q_ASSERT(biostructObjs.size() == 1);
            BioStruct3DObject* obj = qobject_cast<BioStruct3DObject*>(biostructObjs.first());
            Q_ASSERT(obj != NULL);
            if (obj->getGObjectName() == objName) {
                return obj;
            }
        }
    }
    
    return NULL;
}

QSplitter* BioStruct3DSplitter::getParentSplitter()
{
    if (parentSplitter == NULL) {
        QWidget* widget = parentWidget();
        Q_ASSERT(widget != NULL);
        parentSplitter = qobject_cast<QSplitter* > (widget);
    }
    
    return parentSplitter;
}

void BioStruct3DSplitter::adaptSize( int numVisibleWidgets )
{
    if (numVisibleWidgets > 0) {
        isViewCollapsed = false;
        setMaximumHeight(1000);
        getParentSplitter();
        // TODO: Simplify resizing
        int index = parentSplitter->indexOf(this);
        QList<int> sizes = parentSplitter->sizes();
        for(int i = 0; i < sizes.count(); ++i) {
            if (sizes.at(i) >= splitterHeight) {
                sizes[i] = sizes.at(i) - splitterHeight;
                break;
            }
        }
        sizes[index] = splitterHeight;
        parentSplitter->setSizes(sizes);
    } else {
        splitterHeight = splitter->height();
        setFixedHeight(HEADER_HEIGHT);
        isViewCollapsed = true;
    }
}

const QList<QAction* > BioStruct3DSplitter::getSettingsMenuActions() const
{
    QList<QAction*> actions;

    return actions;
}

QList<BioStruct3DGLWidget*> BioStruct3DSplitter::getChildWidgets()
{
    return biostrucViewMap.values();
}

int BioStruct3DSplitter::getNumVisibleWidgets()
{
    int numVisibleWidgets = 0;
    foreach (BioStruct3DGLWidget* widget, biostrucViewMap) {
        if (widget->isVisible()) {
            ++numVisibleWidgets;
        }
    }
    return numVisibleWidgets;
}

void BioStruct3DSplitter::addModelFromObject( BioStruct3DObject* obj )
{   
    QList<GObject*> objects = dnaView->getObjects();
    if (objects.contains(obj)) {
        addBioStruct3DGLWidget(obj);
    } else {
      dnaView->addObject(obj);
    }
}

GLFrameManager* BioStruct3DSplitter::getGLFrameManager()
{
    return glFrameManager.get();
}

///////////////////////////////////////////////////////////////////////////////////////////
// Header widget

SplitterHeaderWidget::SplitterHeaderWidget(BioStruct3DSplitter* sp) : splitter(sp)
{
    setFixedHeight(HEADER_HEIGHT);
    setMinimumHeight(HEADER_HEIGHT);
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    registerWebUrls();
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(10,0,0,0);
    layout->setSpacing(10);

    // Menu toolbar
    HBar* toolbar = new HBar(this);
    toolbar->layout()->setSpacing(10);
    toolbar->layout()->setMargin(0);
    
    QLabel* pixLabel= new QLabel(this);
    QFont f = pixLabel->font();
    if (f.pixelSize() > HEADER_HEIGHT) {
        f.setPixelSize(HEADER_HEIGHT-8);
    }
    QIcon objIcon(":biostruct3d_view/images/logo.png");
    Q_ASSERT(!objIcon.isNull());
    QPixmap pix = objIcon.pixmap(QSize(32, 32), QIcon::Active);
    pixLabel->setPixmap(pix);
    pixLabel->setFont(f);
    
    
    layout->addWidget(pixLabel);
    layout->addWidget(new QLabel(tr("3D Structure Viewer")));
    layout->addStretch();
    layout->addWidget(toolbar);
    setLayout(layout);
    
    // Build header menu
    
    QLabel* activeWidgetLabel = new QLabel(this);
    activeWidgetLabel->setText(tr("Active view:"));
    toolbar->addWidget(activeWidgetLabel);

    activeWidgetBox = new QComboBox(this);
    toolbar->addWidget(activeWidgetBox);
    connect(splitter, SIGNAL(si_bioStruct3DGLWidgetAdded(BioStruct3DGLWidget* )), this, SLOT(sl_addBioStruct3DGLWidget(BioStruct3DGLWidget*)));
    
    restoreDefaultsButton = new QToolButton(this);
    restoreDefaultsButton->setToolTip(tr("Restore Default View"));
    restoreDefaultsButton->setIcon(QIcon(":biostruct3d_view/images/restore.png"));
    restoreDefaultsButton->setFixedWidth(20);
    toolbar->addWidget(restoreDefaultsButton);
    connect(restoreDefaultsButton, SIGNAL(pressed()), this, SLOT(sl_restoreDefaults()));
    
    zoomInButton = new QToolButton(this);
    zoomInButton->setToolTip(tr("Zoom In"));
    zoomInButton->setIcon(QIcon(":core/images/zoom_in.png"));
    zoomInButton->setFixedWidth(20);
    toolbar->addWidget(zoomInButton);
    connect(zoomInButton, SIGNAL(pressed()), this, SLOT(sl_zoomIn()));


    zoomOutButton = new QToolButton(this);
    zoomOutButton->setIcon(QIcon(":core/images/zoom_out.png"));
    zoomOutButton->setToolTip(tr("Zoom Out"));
    zoomOutButton->setFixedWidth(20);
    toolbar->addWidget(zoomOutButton);
    connect(zoomOutButton, SIGNAL(pressed()), this, SLOT(sl_zoomOut()));

    syncLockButton = new QToolButton(this);
    syncLockButton->setIcon(QIcon(":biostruct3d_view/images/lock.png"));
    syncLockButton->setToolTip(tr("Synchronize 3D Structure Views"));
    syncLockButton->setFixedWidth(20);
    syncLockButton->setCheckable(true);
    toolbar->addWidget(syncLockButton);
    connect(syncLockButton, SIGNAL(toggled(bool)), this, SLOT(sl_toggleSyncLock(bool)));
    
    displayMenuButton = new QToolButton(this);
    displayMenuButton->setText(tr("Display"));
    toolbar->addWidget(displayMenuButton);
    connect(displayMenuButton, SIGNAL(pressed()), this, SLOT(sl_showDisplayMenu()));
    
    if (webActionMap.count() != 0) {
        webMenuButton = new QToolButton(this);
        webMenuButton->setText(tr("Links"));
        toolbar->addWidget(webMenuButton);
        connect(webMenuButton, SIGNAL(pressed()), this, SLOT(sl_showWebMenu()));
    }

    addModelButton = new QToolButton(this);
    addModelButton->setText(tr("Add"));
    toolbar->addWidget(addModelButton);
    connect(addModelButton, SIGNAL(pressed()), this, SLOT(sl_addModel()));
    
    // TODO: add settings functionality
    //settingsMenuButton = new QToolButton(this);
    //settingsMenuButton->setText(tr("settings"));
    // toolbar->addWidget(settingsMenuButton);
   
    widgetStateMenuButton = new QToolButton(this);
    widgetStateMenuButton->setIcon(QIcon(":core/images/adv_widget_menu.png"));
    widgetStateMenuButton->setFixedWidth(20);
    toolbar->addWidget(widgetStateMenuButton);
    connect(widgetStateMenuButton, SIGNAL(pressed()), this, SLOT(sl_showStateMenu()));

}

void SplitterHeaderWidget::sl_showStateMenu()
{
    QPointer<QToolButton> widgetStateMenuButtonPtr(widgetStateMenuButton);

    QMenu m;
    
    
    foreach(BioStruct3DGLWidget* widget, widgets) {
        QAction* action = showWidgetActionMap.value(widget);
        m.addAction(action);
    }
    
    m.addAction(splitter->getCloseSplitterAction());
    m.exec(QCursor::pos());

    if (!widgetStateMenuButtonPtr.isNull()) { //if not self closed
        widgetStateMenuButtonPtr->setDown(false);
    }
}

void SplitterHeaderWidget::sl_showDisplayMenu()
{
    QPointer<QToolButton> displayMenuButtonPtr(displayMenuButton);

    BioStruct3DGLWidget* activeWidget = getActiveWidget(); 
    
    QMenu* displayMenu = activeWidget->getDisplayMenu();
    displayMenu->exec(QCursor::pos());

    if (!displayMenuButtonPtr.isNull()) { //if not self closed
        displayMenuButtonPtr->setDown(false);
    }
}

#define ZOOM_DELTA 2.0f

void SplitterHeaderWidget::sl_zoomIn()
{
    BioStruct3DGLWidget* w = getActiveWidget();
    w->zoom(-ZOOM_DELTA);
}

void SplitterHeaderWidget::sl_zoomOut()
{
    BioStruct3DGLWidget* w = getActiveWidget();
    w->zoom(ZOOM_DELTA);
}

void SplitterHeaderWidget::sl_showSettingsMenu()
{
    QPointer<QToolButton> settingsMenuButtonPtr(settingsMenuButton);
    
    QMenu m;
    
    const QList<QAction* > settingsMenuActions = splitter->getSettingsMenuActions();
    foreach (QAction* action, settingsMenuActions) {
        m.addAction(action);
    }
    m.exec(QCursor::pos());
    
    if (!settingsMenuButtonPtr.isNull()) { //if not self closed
        settingsMenuButtonPtr->setDown(false);
    }


}

void SplitterHeaderWidget::sl_restoreDefaults()
{
    BioStruct3DGLWidget* w = getActiveWidget();
    w->restoreDefaultSettigns();
}

void SplitterHeaderWidget::sl_addBioStruct3DGLWidget( BioStruct3DGLWidget* glWidget )
{
    int index = activeWidgetBox->count() + 1;
    const char* pdbId = glWidget->getBioStruct3D().pdbId.constData();
    
    QString name = QString("%1: %2").arg(index).arg(pdbId) ;
    activeWidgetBox->addItem(name);
    connect(glWidget, SIGNAL(si_widgetClosed(BioStruct3DGLWidget*)), this, SLOT(sl_closeWidget3DGLWidget(BioStruct3DGLWidget*)));
    glWidget->installEventFilter(this);

    // Add show widget action
    QString actionName( tr("Show %1").arg(pdbId) );
    QAction* action = new QAction(actionName, glWidget);
    action->setCheckable(true);
    action->setChecked(true);
    connect(action, SIGNAL(triggered(bool)), SLOT(sl_toggleBioStruct3DWidget(bool)));
    showWidgetActionMap.insert(glWidget, action);

    widgets.append(glWidget);
    enableToolbar();
}

bool SplitterHeaderWidget::eventFilter(QObject *obj, QEvent *event)
{   

    
    
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::Wheel) {
        BioStruct3DGLWidget* glWidget = qobject_cast<BioStruct3DGLWidget*>(obj);
        if (glWidget) {
            setActiveView(glWidget);
        } 
    } 

    return QObject::eventFilter(obj, event);
}



BioStruct3DGLWidget* SplitterHeaderWidget::getActiveWidget()
{
    int idx = activeWidgetBox->currentIndex();
    BioStruct3DGLWidget* glWidget = widgets[idx];
    glWidget->makeCurrent();
    return glWidget;
}

void SplitterHeaderWidget::sl_closeWidget3DGLWidget(BioStruct3DGLWidget* glWidget) {
    int index = widgets.indexOf(glWidget);
    assert(index >= 0);
    widgets.removeAt(index);
    activeWidgetBox->removeItem(index);
    splitter->getGLFrameManager()->removeGLWidgetFrame(glWidget);
    updateWidgetBox();
    showWidgetActionMap.remove(glWidget);
    if (widgets.isEmpty()) {
        // TODO: maybe it's possible not to close splitter?
        //splitter->adaptSize(0);
        splitter->getCloseSplitterAction()->trigger();
        return;
    }
    updateToolbar();
}

void SplitterHeaderWidget::enableToolbar() {
    activeWidgetBox->setEnabled(true);
    restoreDefaultsButton->setEnabled(true);
    zoomInButton->setEnabled(true);
    zoomOutButton->setEnabled(true);
    displayMenuButton->setEnabled(true);
    widgetStateMenuButton->setEnabled(true);
    syncLockButton->setEnabled(splitter->getGLFrameManager()->getGLFrames().count() > 1);

}


void SplitterHeaderWidget::updateWidgetBox() {
    activeWidgetBox->clear();
    int index = 0;
    foreach(BioStruct3DGLWidget* widget, widgets) {
        ++index;
        const char* pdbId = widget->getBioStruct3D().pdbId.constData();
        QString name = QString("%1: %2").arg(index).arg(pdbId);
        activeWidgetBox->addItem(name);
    }

}

void SplitterHeaderWidget::updateToolbar() {
    int numVisibleWidgets = 0;
    foreach (BioStruct3DGLWidget* w, widgets) {
        numVisibleWidgets += w->isVisible() ? 1 : 0;
    }
    activeWidgetBox->setEnabled(numVisibleWidgets > 0);
    restoreDefaultsButton->setEnabled(numVisibleWidgets > 0);
    zoomInButton->setEnabled(numVisibleWidgets > 0);
    zoomOutButton->setEnabled(numVisibleWidgets > 0);
    syncLockButton->setEnabled(numVisibleWidgets > 1);
    displayMenuButton->setEnabled(numVisibleWidgets);
    widgetStateMenuButton->setEnabled(!widgets.isEmpty());
}

void SplitterHeaderWidget::sl_toggleBioStruct3DWidget( bool show )
{
    BioStruct3DGLWidget* glWidget = qobject_cast<BioStruct3DGLWidget*> (sender()->parent());
    Q_ASSERT(glWidget != NULL);

    glWidget->setVisible(show);

    int numVisibleWidgets = splitter->getNumVisibleWidgets();

    if ( (show == true && numVisibleWidgets == 1) ||
        (show == false && numVisibleWidgets == 0) ) {
            splitter->adaptSize(numVisibleWidgets);
    }

    updateToolbar();

}

void SplitterHeaderWidget::sl_addModel()
{
    QPointer<QToolButton> addModelButtonPtr(addModelButton);
    
    if (!addModelButtonPtr.isNull()) { //if not self closed
        addModelButtonPtr->setDown(false);
    }
    
    ProjectTreeControllerModeSettings settings;
    settings.objectTypesToShow.append(GObjectTypes::BIOSTRUCTURE_3D);
    QList<GObject*> objects = ProjectTreeItemSelectorDialog::selectObjects(settings, this);
    
    if (!objects.isEmpty()) {
        foreach(GObject* obj, objects) {
            Task* t = new AddModelToSplitterTask(obj, splitter);
            AppContext::getTaskScheduler()->registerTopLevelTask(t);
        }
    }

}

void SplitterHeaderWidget::sl_showWebMenu()
{
    QPointer<QToolButton> webMenuButtonPtr(webMenuButton);
    
    QMenu m;
    QString pdbId(getActiveWidget()->getBioStruct3D().pdbId);
    
    const QList<QAction* >& webMenuActions = webActionMap.keys();
    foreach (QAction* action, webMenuActions) {
        QString wikiName = action->text().split(": ", QString::SkipEmptyParts).last();
        action->setText(pdbId+": "+wikiName);
        m.addAction(action);
    }
    m.exec(QCursor::pos());

    if (!webMenuButtonPtr.isNull()) { //if not self closed
        webMenuButtonPtr->setDown(false);
    }

    
}

void SplitterHeaderWidget::registerWebUrls()
{
//    QSignalMapper *signalMapper = new QSignalMapper(this);
//    connect(signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(sl_openBioStructUrl(const QString&)));
    
    DBLinksFile linksFile;
    if (!linksFile.load()) {
        return;
    }

    QList<DBLink> links = linksFile.getLinks();
    foreach (const DBLink& link, links) {
        QAction* webAction = new QAction(link.name,this);
        webActionMap.insert(webAction, link.url);
        connect(webAction, SIGNAL(triggered(bool)), this, SLOT(sl_openBioStructUrl()));
    }

    //connect(signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(sl_openBioStructUrl(const QString&)));
    
    //connect(signalMapper, SIGNAL(mapped(const QString &)),
    //    this, SIGNAL(triggered(const QString &)));



}

void SplitterHeaderWidget::sl_openBioStructUrl()
{
    QAction* webAction = qobject_cast<QAction*>(QObject::sender());
    if (webAction == NULL)
        return;
    const QString& urlHeader = webActionMap.value(webAction); 
    QString pdbId(getActiveWidget()->getBioStruct3D().pdbId.toLower());
    QString urlName = urlHeader.arg(pdbId);
    QUrl url(urlName);
    QDesktopServices::openUrl(urlName);

}

void SplitterHeaderWidget::setActiveView( BioStruct3DGLWidget* glWidget )
{
    int index = 0;
    foreach (BioStruct3DGLWidget* widget, widgets) {
        if (widget == glWidget ) {
            widget->makeCurrent();
            activeWidgetBox->setCurrentIndex(index);
            break;
        }
        ++index;    
    }
    
}

void SplitterHeaderWidget::sl_toggleSyncLock( bool toggle )
{
    splitter->getGLFrameManager()->setSyncLock(toggle, widgets[0]);
}
///////////////////////////////////////////////////////////////////////////////////////////
// DBLinksFile

#define DB_LINKS_FILE_NAME "biostruct3d_plugin/BioStruct3DLinks.txt"

bool DBLinksFile::load()
{
    QFile file( QString(PATH_PREFIX_DATA)+ ":" + DB_LINKS_FILE_NAME );
    if(!file.exists() || !file.open(QIODevice::ReadOnly)){
        ioLog.error(SplitterHeaderWidget::tr("File not found: %1").arg(DB_LINKS_FILE_NAME));
        return false;
    } 

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if(line.length()==0 || line.startsWith("#")) {
            continue;
        }
        QStringList fields = line.split("|");
        if (fields.size() != 2) {
            ioLog.error(SplitterHeaderWidget::tr("Illegal entry: %1").arg(line));
            continue;
        }
        DBLink linkInfo(fields[0], fields[1].trimmed());
        links.append(linkInfo);
        
    }
    file.close();

    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// AddModelToSplitterTask

AddModelToSplitterTask::AddModelToSplitterTask( GObject* o, BioStruct3DSplitter* s ) :
    Task("", TaskFlags_FOSCOE), doc(NULL), obj(o), bObj(NULL), splitter(s)
{
    setTaskName(tr("Add 3d model '%1' to BioStruct3DSplitter").arg(o->getGObjectName())); 
}

void AddModelToSplitterTask::prepare()
{
    if (obj->isUnloaded()) {
        doc = obj->getDocument();
        addSubTask(new LoadUnloadedDocumentTask(doc));
    } 
}

void AddModelToSplitterTask::run()
{
    if (doc == NULL) {
        bObj = qobject_cast<BioStruct3DObject*> (obj);
    } else {
        Q_ASSERT(doc->isLoaded());
        QList<GObject*> lst = doc->findGObjectByType(GObjectTypes::BIOSTRUCTURE_3D);
        Q_ASSERT(!lst.isEmpty());
        bObj = qobject_cast<BioStruct3DObject*> (lst.first());
    }
    

}

Task::ReportResult AddModelToSplitterTask::report()
{
    if ( (bObj == NULL) || ( propagateSubtaskError() == true)  ) {
        stateInfo.setError(tr("Model %1 wasn't added").arg(obj->getGObjectName()));
    } else {
        splitter->addModelFromObject(bObj);
    }

    return ReportResult_Finished;
}
} //namespace
