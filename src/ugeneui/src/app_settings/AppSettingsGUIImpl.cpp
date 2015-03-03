/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "AppSettingsGUIImpl.h"
#include "AppSettingsDialogController.h"

#include "format_settings/FormatSettingsGUIController.h"
#include "network_settings/NetworkSettingsGUIController.h"
#include "user_apps_settings/UserApplicationsSettingsGUIController.h"
#include "resource_settings/ResourceSettingsGUIController.h"
#include "directories_settings/DirectoriesSettingsGUIController.h"

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMenu>
#else
#include <QtWidgets/QMenu>
#endif

namespace U2 {
AppSettingsGUIImpl::AppSettingsGUIImpl(QObject* p) : AppSettingsGUI(p)
{
    registerBuiltinPages();
    QMenu* m = AppContext::getMainWindow()->getTopLevelMenu(MWMENU_SETTINGS);
    
    QAction* settingsdialogAction = new QAction(QIcon(":ugene/images/preferences.png"), tr("Preferences..."), this);
    connect(settingsdialogAction, SIGNAL(triggered()), SLOT(sl_showSettingsDialog()));
    settingsdialogAction->setObjectName("action__settings");
#ifdef Q_OS_MAC
    settingsdialogAction->setMenuRole(QAction::ApplicationSpecificRole);
#endif
    m->addAction(settingsdialogAction);
    //m->addAction(tr("app_settings"), this, SLOT(sl_showSettingsDialog()));
}

AppSettingsGUIImpl::~AppSettingsGUIImpl() {
    foreach(AppSettingsGUIPageController* page, pages) {
        delete page;
    }
}

bool AppSettingsGUIImpl::registerPage(AppSettingsGUIPageController* page, const QString& beforePage) {
    AppSettingsGUIPageController* c = findPageById(page->getPageId());
    if (c != NULL) {
        return false;
    }
    if (!beforePage.isEmpty()) {
        AppSettingsGUIPageController* before = findPageById(beforePage);        
        if (before!=NULL) {
            int i = pages.indexOf(before);
            pages.insert(i, page);
            return true;
        }
    }
    pages.append(page);
    return true;
}

bool AppSettingsGUIImpl::unregisterPage(AppSettingsGUIPageController* page) {
    AppSettingsGUIPageController* c = findPageById(page->getPageId());
    if (c == NULL) {
        return false;
    }
    pages.removeOne(page);
    return true;
}

void AppSettingsGUIImpl::showSettingsDialog(const QString& pageId) const {
    QWidget *p = (QWidget*)(AppContext::getMainWindow()->getQMainWindow());
    AppSettingsDialogController c(pageId,p);
    c.exec();
}

AppSettingsGUIPageController* AppSettingsGUIImpl::findPageById(const QString& pageId) const {
    foreach(AppSettingsGUIPageController* page, pages) {
        if (page->getPageId() == pageId) {
            return page;
        }
    }
    return NULL;
}

void AppSettingsGUIImpl::registerBuiltinPages() {
    registerPage(new UserApplicationsSettingsPageController());
    registerPage(new ResourceSettingsGUIPageController());
    registerPage(new NetworkSettingsPageController());
    registerPage(new FormatSettingsGUIPageController());
    registerPage(new DirectoriesSettingsPageController());
}

}//namespace
