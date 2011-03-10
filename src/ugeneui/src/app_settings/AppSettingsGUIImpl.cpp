#include "AppSettingsGUIImpl.h"
#include "AppSettingsDialogController.h"

#include "network_settings/NetworkSettingsGUIController.h"
#include "user_apps_settings/UserApplicationsSettingsGUIController.h"
#include "resource_settings/ResourceSettingsGUIController.h"

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>

#include <QtGui/QMenu>

namespace U2 {
AppSettingsGUIImpl::AppSettingsGUIImpl(QObject* p) : AppSettingsGUI(p)
{
    registerBuiltinPages();
    QMenu* m = AppContext::getMainWindow()->getTopLevelMenu(MWMENU_SETTINGS);
    
    QAction* settingsdialogAction = new QAction(QIcon(":ugene/images/preferences.png"), tr("Preferences..."), this);
    connect(settingsdialogAction, SIGNAL(triggered()), SLOT(sl_showSettingsDialog()));
    settingsdialogAction->setObjectName("action__settings");
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
}

}//namespace
