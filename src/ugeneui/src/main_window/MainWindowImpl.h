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

#ifndef _U2_MAINWINDOW_IMPL_
#define _U2_MAINWINDOW_IMPL_

#include <U2Gui/MainWindow.h>
#include <U2Gui/Notification.h>

#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QMenu>
#include <QtGui/QMdiArea>

class QMdiArea;
class QToolBar;

namespace U2 {

class MWDockManagerImpl;
class MWMenuManagerImpl;
class MWToolBarManagerImpl;

//workaround for QMdiArea issues
class FixedMdiArea : public QMdiArea {
    Q_OBJECT
public:
    FixedMdiArea(QWidget * parent = 0);
    void setViewMode(QMdiArea::ViewMode mode);
    QMdiSubWindow * addSubWindow( QWidget * widget);
private slots:
    //Workaround for QTBUG-17428
    void sysContextMenuAction(QAction*);
    void closeSubWindow(int);
};

class MainWindowImpl: public MainWindow {
    Q_OBJECT
public:
	MainWindowImpl();
	~MainWindowImpl();
    
    virtual QMenu* getTopLevelMenu(const QString& sysName) const;
    virtual QToolBar* getToolbar(const QString& sysName) const;

    virtual MWMDIManager*           getMDIManager() const {return mdiManager;}
    virtual MWDockManager*          getDockManager() const {return dockManager;}
    virtual QMainWindow*            getQMainWindow() const {return mw;}
	virtual NotificationStack*		getNotificationStack() const {return nStack;}

	virtual void setWindowTitle(const QString& title);
    
    void show();
    void close();

	void runClosingTask();
    void setShutDownInProcess(bool flag) {shutDownInProcess = flag;}

private slots:
	void sl_exitAction();
    void sl_aboutAction();
    void sl_checkUpdatesAction();
    void sl_visitWeb();
    void sl_viewOnlineDocumentation();
    void sl_openManualAction();
    void sl_openWDManualAction();
    void sl_openQDManualAction();

private:
    void createActions();
    void prepareGUI();
    void openManual(const QString& name);

    QMainWindow*			mw;
    FixedMdiArea*			mdi;
    
    MWMenuManagerImpl*  	menuManager;
	MWToolBarManagerImpl*	toolbarManager;
    MWMDIManager*			mdiManager;
    MWDockManager*			dockManager;

	NotificationStack*		nStack;

    QAction*				exitAction;
    QAction*				aboutAction;
    QAction*				checkUpdateAction;
    QAction*                visitWebAction;
    QAction*                viewOnlineDocumentation;
    QAction*                openManualAction;
    QAction*                openWDManualAction;
    QAction*                openQDManualAction;

    bool                    shutDownInProcess;
};

}//namespace

#endif
