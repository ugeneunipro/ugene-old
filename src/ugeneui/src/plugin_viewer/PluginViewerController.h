#ifndef _U2_PLUGIN_VIEWER_CONTROLLER_H_

#include "ui/ui_PluginViewerWidget.h"

#include <U2Core/PluginModel.h>
#include <U2Gui/MainWindow.h>

#include <QtGui/QtGui>

namespace U2 {

class PlugViewPluginItem;
class PlugViewServiceItem;

class PluginViewerController : public QObject {
    Q_OBJECT
public:
    PluginViewerController();
    ~PluginViewerController();

protected:
	bool eventFilter(QObject *obj, QEvent *event);

private slots:

    void sl_show();
    void sl_addPlugin();
    void sl_enablePlugin();
    void sl_disablePlugin();
    void sl_enableService();
    void sl_disableService();

    void sl_treeCurrentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous);
    void sl_treeCustomContextMenuRequested(const QPoint & pos);

//plugin support monitoring
    void sl_onPluginAdded(Plugin* p);
    void sl_pluginRemoveFlagChanged(Plugin*);

    void sl_onServiceStateChanged(Service* s, ServiceState oldState);
    void sl_onServiceRegistered(Service* s);
    void sl_onServiceUnregistered(Service* s);
    void sl_taskStateChanged();

private: 
    void connectStaticActions();
    void connectVisualActions();
	void disconnectVisualActions();
    void buildItems();
    void createWindow();
	void updateActions();
    void updateState();

	PlugViewPluginItem* findPluginItem(Plugin* p) const;
	PlugViewServiceItem* findServiceItem(Service* s) const;
	PlugViewServiceItem* getCurrentServiceItem() const;
    PlugViewPluginItem* getCurrentPluginItem() const;

    Ui::PluginViewWidget ui;
    
    MWMDIWindow* mdiWindow;
    
    QAction*    addPluginAction;
    
    QAction*    enablePluginAction;
    QAction*    disablePluginAction;
    
    QAction*    enableServiceAction;
    QAction*    disableServiceAction;
    
    bool        showServices;

};


class PlugViewTreeItem : public QTreeWidgetItem {
public:
    PlugViewTreeItem(PlugViewTreeItem *parent) : QTreeWidgetItem(parent) {}
    
    bool isRootItem() {return parent() == NULL;}
    virtual void updateVisual() = 0;
    virtual bool isPluginItem() const {return false;}
    virtual bool isServiceItem() const {return false;}

};

class PlugViewPluginItem: public PlugViewTreeItem {
public:
    PlugViewPluginItem(PlugViewTreeItem *parent, Plugin* p, bool showServices);
    virtual bool isPluginItem() const {return true;}
    void updateVisual();
    Plugin* plugin;
    bool showServices;
};

class PlugViewServiceItem : public PlugViewTreeItem {
public:
    PlugViewServiceItem(PlugViewPluginItem *parent, Service* s);
    virtual bool isServiceItem() const {return true;}
    void updateVisual();

    Service* service;
};
}//namespace


#endif
