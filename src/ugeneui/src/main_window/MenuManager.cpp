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

#include "MenuManager.h"

namespace U2 {
#define STATIC_MENU_MODEL 1

MWMenuManagerImpl::MWMenuManagerImpl(QObject* p, QMenuBar* mb) : QObject(p) 
{
	menuBar = mb;
    menuBar->setObjectName("mw_menu_bar");
	createTopLevelMenu(MWMENU_FILE, tr("mw_menu_file"));
	createTopLevelMenu(MWMENU_ACTIONS, tr("mw_menu_actions"), MWMENU_FILE);
	createTopLevelMenu(MWMENU_SETTINGS, tr("mw_menu_settings"), MWMENU_ACTIONS);
	createTopLevelMenu(MWMENU_TOOLS, tr("mw_menu_tools"), MWMENU_SETTINGS);
	createTopLevelMenu(MWMENU_WINDOW, tr("mw_menu_window"), MWMENU_TOOLS);
	createTopLevelMenu(MWMENU_HELP, tr("mw_menu_help"), MWMENU_WINDOW);
}

QMenu* MWMenuManagerImpl::getTopLevelMenu(const QString& sysName) const {
	foreach(QMenu* m, toplevelMenus) {
		if (m->menuAction()->objectName() == sysName) {
			return m;
		}
	}
	return NULL;
}


QMenu* MWMenuManagerImpl::createTopLevelMenu(const QString& sysName, const QString& title, const QString& afterSysName) {
    QMenu* qmenu = getTopLevelMenu(sysName);
	assert(!qmenu);
    if (qmenu) {
        return qmenu;
    }
	QMenu* menuBefore = getTopLevelMenu(afterSysName);
	if (menuBefore == NULL) {
		menuBefore = getTopLevelMenu(MWMENU_TOOLS);
	}
	qmenu = new QMenu(title, menuBar);
    //qmenu->setObjectName(sysName);//??? need refactoring...
	qmenu->menuAction()->setObjectName(sysName);
	int insertPos = toplevelMenus.indexOf(menuBefore) + 1;
	if (insertPos == 0) {
		insertPos = toplevelMenus.size();
	}
	toplevelMenus.insert(insertPos, qmenu);

#ifdef STATIC_MENU_MODEL
    menuBar->addMenu(qmenu);
//#else
    if (MWMENU_WINDOW != sysName) {
	    qmenu->installEventFilter(this);
        qmenu->setEnabled(false);
    }
#endif
    return qmenu;
}

bool MWMenuManagerImpl::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::ActionAdded || event->type() == QEvent::ActionRemoved)  {
            QMenu* menu = qobject_cast<QMenu*>(obj);
            assert(menu!=NULL);
#ifndef Q_OS_MAC
            menu->setEnabled(!menu->isEmpty());
#endif
            //updateTopLevelMenuVisibility(menu);
	}
	return QObject::eventFilter(obj, event);
}

void MWMenuManagerImpl::updateTopLevelMenuVisibility(QMenu* m) {
    const QList<QAction*>& activeActions = menuBar->actions();
    if (m->isEmpty() && activeActions.contains(m->menuAction()) && m->menuAction()->objectName()!=MWMENU_WINDOW) {
        unlinkTopLevelMenu(m);
    } else if (!m->isEmpty() && !activeActions.contains(m->menuAction())){
        linkTopLevelMenu(m);
    }
}

void MWMenuManagerImpl::unlinkTopLevelMenu(QMenu* m){
    Q_UNUSED(m);
    //assert(menuBar->actions().contains(m->menuAction()));
    //menuBar->removeAction(m->menuAction());
}

void MWMenuManagerImpl::linkTopLevelMenu(QMenu* m) {
    assert(!menuBar->actions().contains(m->menuAction()));
    const QList<QAction*>& activeActions = menuBar->actions();
    QAction* nextActiveAction = NULL;
	for (int i = toplevelMenus.indexOf(m)+1; i < toplevelMenus.size();i++) {
		QMenu* tmpM = toplevelMenus.at(i);
		if (activeActions.contains(tmpM->menuAction())) {
			nextActiveAction = tmpM->menuAction();
			break;
		}
	}
	menuBar->insertAction(nextActiveAction, m->menuAction());
}


}//namespace
