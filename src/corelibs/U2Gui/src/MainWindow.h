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

#ifndef _U2_MAIN_WINDOW_H_
#define _U2_MAIN_WINDOW_H_

#include <U2Core/global.h>
#include <U2Core/PluginModel.h>
#include <U2Core/ServiceTypes.h>

#include <QtCore/QEvent>
#include <QtGui/QWidget>

class QAction;
class QMdiSubWindow;
class QDockWidget;
class QMenu;
class QToolBar;
class QMainWindow;

// menu 
#define MWMENU								"mw_menu_bar"
#define MWMENU_FILE                         "mwmenu_file"
#define MWMENU_ACTIONS                      "mwmenu_actions"
#define MWMENU_SETTINGS                     "mwmenu_settings"
#define MWMENU_TOOLS                        "mwmenu_tools"
#define MWMENU_TOOLS_MALIGN                 "mwmenu_tools_malign"
#define MWMENU_TOOLS_ASSEMBLY               "mwmenu_tools_assembly"
#define MWMENU_WINDOW                       "mwmenu_window"
#define MWMENU_HELP                         "mwmenu_help"



// toolbar
#define MWTOOLBAR_MAIN                      "mwtoolbar_main"
#define MWTOOLBAR_ACTIVEMDI                 "mwtoolbar_activemdi"



//actions
#define ACTION__EXIT            "action__exit"
#define ACTION__ABOUT           "action__about"
#define ACTION__LOGVIEW         "action__logview"
#define ACTION__PLUGINS_VIEW    "action__plugins"

#define ACTION_PROJECTSUPPORT__NEW_PROJECT              "action_projectsupport__new_project"
#define ACTION_PROJECTSUPPORT__OPEN_PROJECT             "action_projectsupport__open_project"
#define ACTION_PROJECTSUPPORT__SAVE_PROJECT             "action_projectsupport__save_project"
#define ACTION_PROJECTSUPPORT__SAVE_AS_PROJECT          "action_projectsupport__save_as_project"
#define ACTION_PROJECTSUPPORT__CLOSE_PROJECT            "action_projectsupport__close_project"
#define ACTION_PROJECTSUPPORT__RECENT_PROJECTS_MENU     "action_projectsupport__recent_projects_menu"
#define ACTION_PROJECTSUPPORT__EXPORT_PROJECT           "action_projectsupport__export_project"

#define ACTION_PROJECT__ADD_MENU                    "action_project__add_menu"
#define ACTION_PROJECT__ADD_NEW_DOCUMENT            "action_project__add_new_document"
#define ACTION_PROJECT__ADD_EXISTING_DOCUMENT       "action_project__add_existing_document"

#define ACTION_PROJECT__EDIT_MENU                    "action_project__edit_menu"

#define DOCK_PROJECT_VIEW    "dock_project_view"
#define DOCK_TASK_VIEW       "dock_task_view"
#define DOCK_LOG_VIEW        "dock_log_view"

namespace U2 {

class MWMDIManager;
class MWDockManager;
class NotificationStack;

class U2GUI_EXPORT MainWindow : public QObject {
    Q_OBJECT
public:
    virtual QMenu*       getTopLevelMenu(const QString& sysName) const = 0;
    virtual QToolBar*    getToolbar(const QString& sysName) const  = 0;
    virtual QMainWindow* getQMainWindow() const = 0;

    virtual MWMDIManager*           getMDIManager() const = 0;
    virtual MWDockManager*          getDockManager() const = 0;
    virtual NotificationStack*      getNotificationStack() const = 0;
    
    

    virtual void setWindowTitle(const QString& title) = 0;
};

//////////////////////////////////////////////////////////////////////////
/// MDI

class U2GUI_EXPORT MWMDIWindow : public QWidget {
    Q_OBJECT

    friend class MWMDIManager;
public:

    MWMDIWindow(const QString& _windowName);

    virtual void setupMDIToolbar(QToolBar*) {}
    virtual void setupViewMenu(QMenu*) {}

    int getId() const {return windowId;}

protected:
    /* returns true if a window may be closed, false otherwise */
    virtual bool onCloseEvent() {return true;}

protected:
    int     windowId;
};

class U2GUI_EXPORT MWMDIManager : public QObject {
    Q_OBJECT
public:
    MWMDIManager(QObject* p) : QObject(p){}
    
    /** Adds the specified window to MainWindow layout and activates it. */
    virtual void addMDIWindow(MWMDIWindow* w) = 0;

    virtual bool closeMDIWindow(MWMDIWindow* w) = 0;

    virtual QList<MWMDIWindow*> getWindows() const = 0;

    virtual MWMDIWindow* getWindowById(int id) const = 0;

    virtual void activateWindow(MWMDIWindow* w) = 0;

    virtual MWMDIWindow* getActiveWindow() const = 0;

signals:
    void si_windowAdded(MWMDIWindow* w);
    void si_windowClosing(MWMDIWindow* w);
    void si_windowActivated(MWMDIWindow* w);

protected: 
    /* returns true if the window agreed to close, false otherwise */
    bool onCloseEvent(MWMDIWindow* w) {return w->onCloseEvent();}
};

//////////////////////////////////////////////////////////////////////////
/// DOCK

enum MWDockArea {
    MWDockArea_Left,
    MWDockArea_Right,
    MWDockArea_Bottom,
    MWDockArea_MaxDocks
};

class U2GUI_EXPORT MWDockManager: public QObject {
    Q_OBJECT
public:
    MWDockManager(QObject* p) : QObject(p){}
    
    virtual QAction* registerDock(MWDockArea area, QWidget* w, const QKeySequence& ks = QKeySequence()) = 0;

    virtual QWidget* findWidget(const QString& widgetObjName) = 0;

    virtual QWidget* getActiveWidget(MWDockArea a) = 0;

    virtual QWidget* activateDock(const QString& widgetObjName) = 0;

    virtual QWidget* toggleDock(const QString& widgetObjName) = 0;
};

}//namespace
Q_DECLARE_METATYPE(U2::MainWindow*)
Q_DECLARE_METATYPE(U2::MWMDIManager*)
Q_DECLARE_METATYPE(U2::MWDockManager*)
#endif
