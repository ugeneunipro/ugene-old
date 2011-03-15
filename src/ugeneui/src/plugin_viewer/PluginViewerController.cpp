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

#include "PluginViewerController.h"

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/Settings.h>
#include <U2Core/Task.h>
#include <U2Gui/GUIUtils.h>
#include <PluginDescriptor.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
//controller

/* TRANSLATOR U2::PluginViewerController */

#define PLUGIN_VIEW_SETTINGS QString("pluginview/")

PluginViewerController::PluginViewerController() {
    showServices = false; //'true' mode is not functional anymore after service<->plugin model refactoring
    mdiWindow = NULL;
    addPluginAction = NULL;
    enablePluginAction = NULL;
    disableServiceAction = NULL;

    connectStaticActions();    
   
    if (AppContext::getSettings()->getValue(PLUGIN_VIEW_SETTINGS + "isVisible", false).toBool()) {
        createWindow();
    }
}

PluginViewerController::~PluginViewerController() {
    AppContext::getPluginSupport()->disconnect(this);

    AppContext::getSettings()->setValue(PLUGIN_VIEW_SETTINGS + "isVisible", mdiWindow!=NULL);

    if (mdiWindow) {
        AppContext::getMainWindow()->getMDIManager()->closeMDIWindow(mdiWindow);
        assert(mdiWindow == NULL); // must be NULLED on close event
    }
}

void PluginViewerController::createWindow() {
    assert(mdiWindow == NULL);

    mdiWindow = new MWMDIWindow(tr("plugin_view_window_title"));
    ui.setupUi(mdiWindow);
    ui.treeWidget->setColumnWidth(1, 200); //todo: save geom

    if (!showServices) {
        ui.treeWidget->hideColumn(0);
    }

    QList<int> sizes; sizes<<200<<500;
    ui.splitter->setSizes(sizes);
    ui.treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    
    connectVisualActions();
    buildItems();

    ui.treeWidget->setSortingEnabled(true);
    ui.treeWidget->sortByColumn(1, Qt::AscendingOrder);

    mdiWindow->installEventFilter(this);
    MWMDIManager* mdiManager = AppContext::getMainWindow()->getMDIManager();
    mdiManager->addMDIWindow(mdiWindow);

    updateState();
}

void PluginViewerController::connectStaticActions() {
    //add actions to menu and toolbar
    MainWindow* mw = AppContext::getMainWindow();
    QMenu* pluginsMenu = mw->getTopLevelMenu(MWMENU_SETTINGS);
    
    QAction* viewPluginsAction = new QAction(QIcon(":ugene/images/plugins.png"), tr("Plugins..."), this);
    connect(viewPluginsAction, SIGNAL(triggered()), SLOT(sl_show()));
    viewPluginsAction->setObjectName(ACTION__PLUGINS_VIEW);
    pluginsMenu->addAction(viewPluginsAction);

    addPluginAction = new QAction(tr("add_plugin_label"), this);
    connect(addPluginAction, SIGNAL(triggered()), SLOT(sl_addPlugin()));

    enablePluginAction = new QAction(tr("enable_plugin_action"), this);
    connect(enablePluginAction, SIGNAL(triggered()), SLOT(sl_enablePlugin()));

    disablePluginAction = new QAction(tr("disable_plugin_action"), this);
    connect(disablePluginAction, SIGNAL(triggered()), SLOT(sl_disablePlugin()));

    enableServiceAction =  new QAction(tr("enable_service_label"), this);
    connect(enableServiceAction, SIGNAL(triggered()), SLOT(sl_enableService()));

    disableServiceAction = new QAction(tr("disable_service_label"), this);
    connect(disableServiceAction, SIGNAL(triggered()), SLOT(sl_disableService()));
}

void PluginViewerController::connectVisualActions() {
    //connect to plugin support signals
    PluginSupport* ps = AppContext::getPluginSupport();
    connect(ps, SIGNAL(si_pluginAdded(Plugin*)), SLOT(sl_onPluginAdded(Plugin*)));
    connect(ps, SIGNAL(si_pluginRemoveFlagChanged(Plugin*)), SLOT(sl_pluginRemoveFlagChanged(Plugin*)));
    if (showServices) {
        ServiceRegistry* sr = AppContext::getServiceRegistry();
        connect(sr, SIGNAL(si_serviceStateChanged(Service*, ServiceState)), SLOT(sl_onServiceStateChanged(Service*, ServiceState)));
        connect(sr, SIGNAL(si_serviceRegistered(Service*)), SLOT(sl_onServiceRegistered(Service*)));
        connect(sr, SIGNAL(si_serviceUnregistered(Service*)), SLOT(sl_onServiceUnregistered(Service*)));
    }

    connect(ui.treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),SLOT(sl_treeCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
    connect(ui.treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)),SLOT(sl_treeCustomContextMenuRequested(const QPoint&)));
}

void PluginViewerController::disconnectVisualActions() {
    AppContext::getPluginSupport()->disconnect(this);
    AppContext::getServiceRegistry()->disconnect(this);
    
    //ui.treeWidget and all button/menues are disconnected automatically -> widget is deleted
}

void PluginViewerController::updateActions() {
    PlugViewTreeItem* item = static_cast<PlugViewTreeItem*>(ui.treeWidget->currentItem());
    bool isPlugin = item!=NULL && item->isPluginItem();
    Plugin* p = isPlugin ? (static_cast<PlugViewPluginItem*>(item))->plugin : NULL;
    
    bool isRemoved = isPlugin && AppContext::getPluginSupport()->getRemoveFlag(p);
    disablePluginAction->setEnabled(isPlugin && !isRemoved);
    enablePluginAction->setEnabled(isPlugin && isRemoved);

    bool isService = item!=NULL && item->isServiceItem();
    Service* s = isService ? (static_cast<PlugViewServiceItem*>(item))->service : NULL;
    bool isServiceEnabled = isService && s->isEnabled();

    enableServiceAction->setEnabled(isService && !isServiceEnabled);
    disableServiceAction->setEnabled(isService && isServiceEnabled);
}


void PluginViewerController::buildItems() {
    const QList<Plugin*>& plugins = AppContext::getPluginSupport()->getPlugins();
    foreach(Plugin* p, plugins) {
        sl_onPluginAdded(p);
    }
}


PlugViewPluginItem* PluginViewerController::findPluginItem(Plugin* p) const {
    int nPlugins = ui.treeWidget->topLevelItemCount();
    for (int i=0;i<nPlugins;i++) {
        PlugViewPluginItem* item = static_cast<PlugViewPluginItem*>(ui.treeWidget->topLevelItem(i));
        if (item->plugin == p) {
            return item;
        }
    }
    return NULL;
}

PlugViewServiceItem* PluginViewerController::findServiceItem(Service* /*s*/) const {
    /*PlugViewPluginItem* pi = findPluginItem(s->getPlugin());
    if (!pi) {
        return NULL;
    }
    int nServices = pi->childCount();
    for (int i=0;i<nServices;i++) {
        PlugViewServiceItem* item = (PlugViewServiceItem*)pi->child(i);
        if (item->service == s) {
            return item;
        }
    }*/
    return NULL;
}

bool PluginViewerController::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Close && obj == mdiWindow) {
        mdiWindow = NULL;
        disconnectVisualActions();
    }
    return QObject::eventFilter(obj, event);
}

void PluginViewerController::sl_onPluginAdded(Plugin* p) {
    assert(findPluginItem(p)==NULL);
    
    QTreeWidget* treeWidget = ui.treeWidget;
    PlugViewPluginItem* pluginItem = new PlugViewPluginItem(NULL, p, showServices);
    if (showServices) {
        const QList<Service*>& services = p->getServices();
        //this method is called for default state init also -> look for registered plugin services
        ServiceRegistry* sr = AppContext::getServiceRegistry();
        QList<Service*> registered = sr->getServices();
        foreach(Service* s, services) {
            if (registered.contains(s)) {
                PlugViewTreeItem* serviceItem = new PlugViewServiceItem(pluginItem, s);
                pluginItem->addChild(serviceItem);
            }
        }
    }
    treeWidget->addTopLevelItem(pluginItem);
    pluginItem->setExpanded(true);
}

void PluginViewerController::sl_onServiceStateChanged(Service* s, ServiceState oldState) {
    Q_UNUSED(oldState);
    assert(showServices);
    PlugViewServiceItem* si = findServiceItem(s);
    assert(si!=NULL);
    si->updateVisual();
    updateState();
}

void PluginViewerController::sl_onServiceRegistered(Service* /*s*/) {
    /*assert(showServices);
    PlugViewPluginItem* pluginItem = findPluginItem(s->getPlugin());
    assert(pluginItem!=NULL);
    PlugViewTreeItem* serviceItem = findServiceItem(s);
    assert(serviceItem == NULL);
    serviceItem = new PlugViewServiceItem(pluginItem, s);
    pluginItem->addChild(serviceItem);*/
}

void PluginViewerController::sl_onServiceUnregistered(Service* s) {
    assert(showServices);
    PlugViewServiceItem* item = findServiceItem(s);
    assert(item!=NULL);
    delete item;
}


void PluginViewerController::sl_show() {
    if (mdiWindow == NULL) {
        createWindow();
    } else {
        AppContext::getMainWindow()->getMDIManager()->activateWindow(mdiWindow);
    }
}

void PluginViewerController::sl_addPlugin() {
    QString caption = tr("add_plugin_caption");
    QString lastDir = AppContext::getSettings()->getValue(PLUGIN_VIEW_SETTINGS + "addDir").toString();
    
    QString ext=tr("genome_browser2_plugin_files")+" (*."+PLUGIN_FILE_EXT+")";

    QString pluginFilePath = QFileDialog::getOpenFileName(ui.treeWidget, caption, lastDir, ext);
    if (pluginFilePath.isEmpty())  {
        return;
    }
    QFileInfo fi(pluginFilePath);
    QString newLastDir = fi.absoluteDir().absolutePath();
    AppContext::getSettings()->setValue(PLUGIN_VIEW_SETTINGS + "addDir", newLastDir);
    Task* task = AppContext::getPluginSupport()->addPluginTask(pluginFilePath);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
    connect(task, SIGNAL(si_stateChanged()), SLOT(sl_taskStateChanged()));
}

void PluginViewerController::sl_taskStateChanged() {
    Task* t = qobject_cast<Task*>(sender());
    assert(t!=NULL);
    if (t->isFinished() && t->hasErrors()) {
        QMessageBox::critical(ui.treeWidget, tr("add_plugin_error_caption"), t->getError());
    }
}


void PluginViewerController::sl_enablePlugin() {
    PlugViewPluginItem* pi = getCurrentPluginItem();
    assert(pi!=NULL);
    AppContext::getPluginSupport()->setRemoveFlag(pi->plugin, false);
}

void PluginViewerController::sl_disablePlugin() {
    PlugViewPluginItem* pi = getCurrentPluginItem();
    assert(pi!=NULL);
    AppContext::getPluginSupport()->setRemoveFlag(pi->plugin, true);
}

void PluginViewerController::sl_pluginRemoveFlagChanged(Plugin* p) {
    PlugViewPluginItem* pi = findPluginItem(p);
    assert(pi!=NULL);
    pi->updateVisual();
    updateState();
}

PlugViewServiceItem* PluginViewerController::getCurrentServiceItem() const {
    PlugViewTreeItem* item = static_cast<PlugViewTreeItem*>(ui.treeWidget->currentItem());
    assert(item!=NULL && item->isServiceItem());
    PlugViewServiceItem* si = static_cast<PlugViewServiceItem*>(item);
    return si;
}

PlugViewPluginItem* PluginViewerController::getCurrentPluginItem() const {
    PlugViewTreeItem* item = static_cast<PlugViewTreeItem*>(ui.treeWidget->currentItem());
    assert(item!=NULL && item->isPluginItem());
    PlugViewPluginItem* pi = static_cast<PlugViewPluginItem*>(item);
    return pi;
}


void PluginViewerController::sl_enableService() {
    assert(showServices);
    PlugViewServiceItem* si = getCurrentServiceItem();
    if (si == NULL || si->service->isEnabled()) {
        return;
    }
    Task* task = AppContext::getServiceRegistry()->enableServiceTask(si->service);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
}


void PluginViewerController::sl_disableService() {
    PlugViewServiceItem* si = getCurrentServiceItem();
    if (si == NULL || si->service->isDisabled()) {
        return;
    }
    Task* task = AppContext::getServiceRegistry()->disableServiceTask(si->service);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
}



void PluginViewerController::updateState() {
    updateActions();
    ui.infoView->clear();
    PlugViewTreeItem* item = static_cast<PlugViewTreeItem*>(ui.treeWidget->currentItem());
    if (item == NULL) {
        return;
    }
    QString text;
    if (item->isPluginItem()) {
        PlugViewPluginItem* p = static_cast<PlugViewPluginItem*>(item);
        text = "<b>"+p->plugin->getName()+"</b><p>";
        text += p->plugin->getDescription();
    } else {
        assert(item->isServiceItem());
        PlugViewServiceItem* s = static_cast<PlugViewServiceItem*>(item);
        text = s->service->getDescription();
    }
    ui.infoView->setText(text);
}

void PluginViewerController::sl_treeCurrentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous) {
    Q_UNUSED(current); Q_UNUSED(previous);
    updateState();
}


void PluginViewerController::sl_treeCustomContextMenuRequested(const QPoint & pos) {
    Q_UNUSED(pos);

    assert(mdiWindow);
    QMenu* menu = new QMenu(ui.treeWidget);    
    
    menu->addAction(addPluginAction);

    if (disablePluginAction->isEnabled()) {
        menu->addAction(disablePluginAction);
    }

    if (enablePluginAction->isEnabled()) {
        menu->addAction(enablePluginAction);
    }

    if (showServices) {
        if (enableServiceAction->isEnabled()) {
            menu->addAction(enableServiceAction);
        }

        if (disableServiceAction->isEnabled()) {
            menu->addAction(disableServiceAction);
        }
    }


    menu->exec(QCursor::pos());
}

//////////////////////////////////////////////////////////////////////////
// TreeItems

PlugViewPluginItem::PlugViewPluginItem(PlugViewTreeItem *parent, Plugin* p, bool _showServices) 
: PlugViewTreeItem(parent), plugin(p), showServices(_showServices) 
{
    updateVisual();
}

void PlugViewPluginItem::updateVisual() {
    setData(0, Qt::DisplayRole, PluginViewerController::tr("item_type_plugin_label"));
    setData(1, Qt::DisplayRole, plugin->getName());

    bool toRemove = AppContext::getPluginSupport()->getRemoveFlag(plugin);
    QString state = toRemove ?
        PluginViewerController::tr("removed_after_restart") 
        : PluginViewerController::tr("item_state_on");

    setData(2, Qt::DisplayRole, state);
    setData(3, Qt::DisplayRole, plugin->getDescription());

    setIcon(showServices ? 0 : 1, QIcon(":ugene/images/plugins.png"));

    GUIUtils::setMutedLnF(this, toRemove);
    
}

PlugViewServiceItem::PlugViewServiceItem(PlugViewPluginItem *parent, Service* s) : PlugViewTreeItem(parent), service(s) {
    updateVisual();
}

void PlugViewServiceItem::updateVisual() {
    setData(0, Qt::DisplayRole, PluginViewerController::tr("item_type_service_label"));
    setData(1, Qt::DisplayRole, service->getName());
    setData(2, Qt::DisplayRole, service->isEnabled() 
            ? PluginViewerController::tr("item_state_on") 
            : PluginViewerController::tr("item_state_off"));
    setData(3, Qt::DisplayRole, service->getDescription());
    setIcon(0, QIcon(":ugene/images/service.png"));
}

}//namespace
