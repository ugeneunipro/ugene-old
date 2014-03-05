/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "MDIManagerImpl.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/Log.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>

#include <QtCore/QSet>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QHBoxLayout>
#include <QtGui/QToolBar>
#else
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QToolBar>
#endif
//#include <QtGui/QApplication>
#include <QtGui/QtEvents>

namespace U2 {

#define SETTINGS_DIR QString("main_window/mdi/")

static QString getWindowName(MDIItem* mdiItem) {
    if (mdiItem == NULL) { 
        return "<no window>";
    }
    return mdiItem->w->windowTitle();
}

MWMDIManagerImpl::~MWMDIManagerImpl() {
}

void MWMDIManagerImpl::prepareGUI() {
	mdiContentOwner = NULL;

	connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), SLOT(sl_onSubWindowActivated(QMdiSubWindow*)));
	
	windowMapper = new QSignalMapper(this);
	connect(windowMapper, SIGNAL(mapped(QWidget *)), this, SLOT(sl_setActiveSubWindow(QWidget *)));


	//prepare Window menu
	closeAct = new QAction(tr("close_active_view"), this);
    closeAct->setObjectName("Close active view");
    closeAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
	closeAct->setStatusTip(tr("close_active_view_statustip"));
	connect(closeAct, SIGNAL(triggered()), mdiArea, SLOT(closeActiveSubWindow()));

	closeAllAct = new QAction(tr("close_all_mdi_windows"), this);
    closeAllAct->setObjectName("Close all windows");
	closeAllAct->setStatusTip(tr("close_all_mdi_windows_statustip"));
	connect(closeAllAct, SIGNAL(triggered()), mdiArea, SLOT(closeAllSubWindows()));

    tileAct = new QAction(QIcon(":ugene/images/window_tile.png"), tr("tile_mdi_windows"), this);
    tileAct->setObjectName("Tile windows");
	tileAct->setStatusTip(tr("tile_mdi_windows_statustip"));
	connect(tileAct, SIGNAL(triggered()), mdiArea, SLOT(tileSubWindows()));

	cascadeAct = new QAction(QIcon(":ugene/images/window_cascade.png"), tr("cascade_mdi_windows"), this);
    cascadeAct->setObjectName("Cascade windows");
	cascadeAct->setStatusTip(tr("cascade_mdi_windows_statustip"));
	connect(cascadeAct, SIGNAL(triggered()), mdiArea, SLOT(cascadeSubWindows()));

	nextAct = new QAction(QIcon(":ugene/images/window_next.png"), tr("next_mdi_window"), this);
    nextAct->setObjectName("Next window");
	nextAct->setStatusTip(tr("next_mdi_window_statustip"));
	nextAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Tab));
	connect(nextAct, SIGNAL(triggered()), mdiArea, SLOT(activateNextSubWindow()));

	previousAct = new QAction(QIcon(":ugene/images/window_prev.png"), tr("previous_mdi_window"), this);
    previousAct->setObjectName("Previous window");
	previousAct->setStatusTip(tr("previous_mdi_window_statustip"));
	previousAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab));
	connect(previousAct, SIGNAL(triggered()),mdiArea, SLOT(activatePreviousSubWindow()));

	separatorAct = new QAction("-", this);
	separatorAct->setSeparator(true);

	defaultIsMaximized = AppContext::getSettings()->getValue(SETTINGS_DIR + "maximized", true).toBool();

	QMenu* windowMenu = mw->getTopLevelMenu(MWMENU_WINDOW);
	connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(sl_updateWindowMenu()));

	updateState();
    clearMDIContent(true);

    sl_updateWindowLayout();
    connect(AppContext::getAppSettings()->getUserAppsSettings(), SIGNAL(si_windowLayoutChanged()), SLOT(sl_updateWindowLayout()));
}

bool MWMDIManagerImpl::eventFilter(QObject *obj, QEvent *event) {
	QEvent::Type t = event->type();
	if ( t == QEvent::Close) {
		QMdiSubWindow* qw = qobject_cast<QMdiSubWindow*>(obj);
		MDIItem* item = getMDIItem(qw);
        uiLog.trace(QString("Processing close window request for '%1'").arg(getWindowName(item)));
        
        //check if user really wants to close the window, ignore event if not
        if (!onCloseEvent(item->w)) {
            uiLog.trace(QString("Ignoring close window request for '%1'").arg(getWindowName(item)));
            event->ignore();
            return true;
        }
        
        // here we sure that window will be closed
        emit si_windowClosing(item->w);

        if (item == mdiContentOwner) { // if 'current' window is closed -> clear MDI
            clearMDIContent(true);
        }
        items.removeAll(item);
        delete item;
        updateState();
	} else if (t == QEvent::WindowStateChange) {
		QMdiSubWindow* qw = qobject_cast<QMdiSubWindow*>(obj);
		defaultIsMaximized = qw->isMaximized();
	}
	return QObject::eventFilter(obj, event);
}


void MWMDIManagerImpl::updateState() {
	updateActions();
	sl_updateWindowMenu();
	
	AppContext::getSettings()->setValue(SETTINGS_DIR + "maximized", defaultIsMaximized);
}

void MWMDIManagerImpl::updateActions() {
	bool hasMDIWindows = !items.empty();

	closeAct->setEnabled(hasMDIWindows);
	closeAllAct->setEnabled(hasMDIWindows);
	tileAct->setEnabled(hasMDIWindows);
	cascadeAct->setEnabled(hasMDIWindows);
	nextAct->setEnabled(hasMDIWindows);
	previousAct->setEnabled(hasMDIWindows);
	separatorAct->setVisible(hasMDIWindows);
}

void MWMDIManagerImpl::sl_updateWindowMenu() {
	QMenu* windowMenu = mw->getTopLevelMenu(MWMENU_WINDOW);
	windowMenu->clear();//TODO: avoid cleaning 3rd party actions
	windowMenu->addAction(closeAct);
	windowMenu->addAction(closeAllAct);
    if (mdiArea->viewMode() == QMdiArea::SubWindowView) {
	    windowMenu->addSeparator();
	    windowMenu->addAction(tileAct);
	    windowMenu->addAction(cascadeAct);
    }
	windowMenu->addSeparator();
	windowMenu->addAction(nextAct);
	windowMenu->addAction(previousAct);
	windowMenu->addAction(separatorAct);

	separatorAct->setVisible(!items.isEmpty());

	MDIItem* currentItem = getCurrentMDIItem();
	for (int i = 0; i < items.size(); ++i) {
		MDIItem* item = items.at(i);
		QString text;
		if (i < 9) {
			text = QString("&%1 %2").arg(i + 1).arg(item->w->windowTitle());
		} else {
			text = QString("%1 %2").arg(i + 1).arg(item->w->windowTitle());
		}
		QAction *action  = windowMenu->addAction(text);
		action->setCheckable(true);
		action->setChecked(item == currentItem);
		connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
		windowMapper->setMapping(action, item->qw);
	}
}

MDIItem* MWMDIManagerImpl::getCurrentMDIItem() const {
    QMdiSubWindow *currentSubWindow = mdiArea->currentSubWindow();
	if (currentSubWindow != NULL) {
		return getMDIItem(currentSubWindow);
	}
	return NULL;
}

void MWMDIManagerImpl::addMDIWindow(MWMDIWindow* w) {
	bool contains = getWindowById(w->getId())!=NULL;
	if (contains) {
        assert(0); //must never happen
		return;
	}
    w->setParent(mdiArea);
	QMdiSubWindow* qw = mdiArea->addSubWindow(w);
	qw->setWindowTitle(w->windowTitle());
    QIcon icon = w->windowIcon();
    if (icon.isNull()) {
        icon = QIcon(":/ugene/images/ugene_16.png");
    }
    qw->setWindowIcon(icon);
    //qw->setAttribute(Qt::WA_NativeWindow);
	MDIItem* i = new MDIItem(w, qw);
	items.append(i);
	qw->installEventFilter(this);
    
    uiLog.trace(QString("Adding window: '%1'").arg(w->windowTitle()));
    
	updateState();
    
	emit si_windowAdded(w);
    
	if (items.count() == 1 && defaultIsMaximized) {
		qw->showMaximized();
	} else {
		qw->show();	
	}
	qw->raise();
}

QList<MWMDIWindow*> MWMDIManagerImpl::getWindows() const {
	QList<MWMDIWindow*> res;
	foreach(MDIItem* i, items) {
		res.append(i->w);
	}
	return res;
}


bool MWMDIManagerImpl::closeMDIWindow(MWMDIWindow* w) {
	MDIItem* i = getMDIItem(w);
    if(NULL == i)
        return false;
	return i->qw->close();
}

MWMDIWindow* MWMDIManagerImpl::getWindowById(int id) const {
	MDIItem* i = getMDIItem(id);
	return i == NULL ? NULL: i->w;
}

MDIItem* MWMDIManagerImpl::getMDIItem(int id) const {
	foreach(MDIItem* i, items) {
		if (i->w->getId() == id) {
			return i;
		}
	}
	return NULL;
}

MDIItem* MWMDIManagerImpl::getMDIItem(MWMDIWindow* w) const {
	foreach(MDIItem* i, items) {
		if (i->w == w) {
			return i;
		}
	}
	return NULL;
}

MDIItem* MWMDIManagerImpl::getMDIItem(QMdiSubWindow* qw) const {
	foreach(MDIItem* i, items) {
		if (i->qw == qw) {
			return i;
		}
	}
	return NULL;
}

void MWMDIManagerImpl::activateWindow(MWMDIWindow* w)  {
	MDIItem* i = getMDIItem(w);
	assert(i);
	if (i==0) {
		return;
	}
    AppContext::setActiveWindowName(w->windowTitle());
	mdiArea->setActiveSubWindow(i->qw);
	updateState();
}

void MWMDIManagerImpl::sl_setActiveSubWindow(QWidget *w) {
	if (!w) {
		return;
	}
	mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(w));
}

void MWMDIManagerImpl::clearMDIContent(bool addCloseAction) {
	//clear toolbar
	mdiContentOwner = NULL;

    QToolBar* tb = mw->getToolbar(MWTOOLBAR_ACTIVEMDI);
    QMenu* m = mw->getTopLevelMenu(MWMENU_ACTIONS);

    //delete submenus inserted to toolbar and menu
    //todo: provide a flag to enable/disable this behavior for MDI window
    QList<QAction*> allMDIActions = tb->actions() + m->actions();
    QSet<QMenu*> toDel;
    foreach(QAction* ma, allMDIActions) {
        QMenu* am = ma->menu();
        if (am!=NULL) {
            toDel.insert(am);
        }
    }
    
	tb->clear();
	m->clear();

    foreach(QMenu* mtd, toDel) {
        delete mtd;
    }

    if (addCloseAction) {
        m->addAction(closeAct);
    }
}


void MWMDIManagerImpl::sl_onSubWindowActivated(QMdiSubWindow *w) {
    //Details: sub-window is activated and deactivated 
    // 1) every time user switches MDI windows
    // 2) every time user switches applications
    // Here we update mdi content only for 1 case and trying to avoid 2 case

    QMdiSubWindow* currentWindow = mdiArea->currentSubWindow();
    if (w == NULL && currentWindow != NULL) { //simple deactivation, current window is not changed
        uiLog.trace(QString("Window deactivation, no MDI context switch, window: '%1'").arg(getWindowName(mdiContentOwner)));
        assert(getMDIItem(currentWindow) == mdiContentOwner);
        emit si_windowActivated(NULL);
        return;
    }
    if (mdiContentOwner != NULL && mdiContentOwner->qw == w) { // simple activation, current window is not changed
        uiLog.trace(QString("Window activation, no MDI context switch, window: '%1'").arg(getWindowName(mdiContentOwner)));
        emit si_windowActivated(mdiContentOwner->w);
        return;
    }
    if (w == NULL) { // currentWindow is NULL here, mdiContentOwner & it's content cleaned in eventFilter(CloseEvent)
        uiLog.trace(QString("Closing active window"));
        clearMDIContent(false);
        emit si_windowActivated(NULL);
        return;
    }
    MDIItem* mdiItem = getMDIItem(w);
    assert(mdiItem != NULL); // we already know about this window
    uiLog.trace(QString("Switching active MDI window from '%1' to '%2'").arg(getWindowName(mdiContentOwner)).arg(getWindowName(mdiItem)));
    // clear old windows menu/tb content
        clearMDIContent(false);

    // add new content to menu/tb
	QToolBar* tb = mw->getToolbar(MWTOOLBAR_ACTIVEMDI);
	mdiContentOwner = mdiItem;
	mdiContentOwner->w->setupMDIToolbar(tb);
	
	QMenu* m = mw->getTopLevelMenu(MWMENU_ACTIONS);
	mdiContentOwner->w->setupViewMenu(m);
	m->addAction(closeAct);
    
    emit si_windowActivated(mdiItem->w);
}


MWMDIWindow* MWMDIManagerImpl::getActiveWindow() const {
	MDIItem* i = getCurrentMDIItem();
	if (i == NULL) {
		return NULL;
	}
	return i->w;
}

void MWMDIManagerImpl::sl_updateWindowLayout() {
    if (AppContext::getAppSettings()->getUserAppsSettings()->tabbedWindowLayout()) {
        mdiArea->setViewMode(QMdiArea::TabbedView);
    } else {
        mdiArea->setViewMode(QMdiArea::SubWindowView);
    }
}

}//namespace
