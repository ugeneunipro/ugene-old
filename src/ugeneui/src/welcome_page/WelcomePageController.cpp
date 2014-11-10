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

#include <QDesktopServices>
#include <QMainWindow>
#include <QMessageBox>

#include <U2Core/AppContext.h>
#include <U2Core/IdRegistry.h>
#include <U2Core/L10n.h>
#include <U2Core/Settings.h>
#include <U2Core/Task.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/MainWindow.h>

#include "project_support/ProjectLoaderImpl.h"

#include "WelcomePageMdi.h"

#include "WelcomePageController.h"

namespace U2 {

WelcomePageController::WelcomePageController()
: QObject(NULL), welcomePage(NULL)
{
    MainWindow *mainWindow = AppContext::getMainWindow();
    SAFE_POINT(NULL != mainWindow, L10N::nullPointerError("Main Window"), );

    QToolBar *toolBar = mainWindow->getToolbar(MWTOOLBAR_MAIN);
    toolBar->addAction(QIcon(":/core/images/todo.png"), tr("Welcome Screen"), this, SLOT(sl_showPage()));

    MWMDIManager *mdiManager = getMdiManager();
    CHECK(NULL != mdiManager, );

    connect(mdiManager, SIGNAL(si_windowClosing(MWMDIWindow*)), SLOT(sl_onMdiClose(MWMDIWindow*)));
}

MWMDIManager * WelcomePageController::getMdiManager() {
    MainWindow *mainWindow = AppContext::getMainWindow();
    SAFE_POINT(NULL != mainWindow, L10N::nullPointerError("Main Window"), NULL);

    MWMDIManager *result = mainWindow->getMDIManager();
    SAFE_POINT(NULL != result, L10N::nullPointerError("MDI Manager"), NULL);
    return result;
}

void WelcomePageController::onPageLoaded() {
    CHECK(NULL != welcomePage, );

    MWMDIManager *mdiManager = getMdiManager();
    CHECK(NULL != mdiManager, );

    if (!mdiManager->getWindows().contains(welcomePage)) {
        sl_onRecentChanged();
        mdiManager->addMDIWindow(welcomePage);
    }
}

void WelcomePageController::sl_showPage() {
    disconnect(AppContext::getTaskScheduler(), SIGNAL(si_noTasksInScheduler()), this, SLOT(sl_showPage()));
    MWMDIManager *mdiManager = getMdiManager();
    CHECK(NULL != mdiManager, );

    if (NULL != welcomePage) {
        if (mdiManager->getWindows().contains(welcomePage)) {
            mdiManager->activateWindow(welcomePage);
        } // else: it means that the page has already been called but it is loading now
        return;
    }

    welcomePage = new WelcomePageMdi(tr("Welcome Page"), this);
    if (welcomePage->isLoaded()) { // it is for the case of synchronous page loading
        onPageLoaded();
    }
}

void WelcomePageController::sl_onMdiClose(MWMDIWindow *mdi) {
    CHECK(mdi == welcomePage, );
    welcomePage = NULL;
}

void WelcomePageController::sl_onRecentChanged() {
    CHECK(NULL != welcomePage, );
    QStringList recentProjects = AppContext::getSettings()->getValue(SETTINGS_DIR + RECENT_PROJECTS_SETTINGS_NAME, QStringList(), true).toStringList();
    QStringList recentFiles = AppContext::getSettings()->getValue(SETTINGS_DIR + RECENT_ITEMS_SETTINGS_NAME, QStringList(), true).toStringList();
    welcomePage->updateRecent(recentProjects, recentFiles);
}

void WelcomePageController::performAction(const QString &actionId) {
    IdRegistry<WelcomePageAction> *welcomePageActions = AppContext::getWelcomePageActionRegistry();
    SAFE_POINT(NULL != welcomePageActions, L10N::nullPointerError("Welcome Page Actions"), );

    WelcomePageAction *action = welcomePageActions->getById(actionId);
    if (NULL != action) {
        action->perform();
    } else if (BaseWelcomePageActions::CREATE_WORKFLOW == actionId) {
        QMessageBox::warning(AppContext::getMainWindow()->getQMainWindow(), L10N::warningTitle(),
            tr("The Workflow Designer plugin is not loaded. You can add it using the menu Settings -> Plugins. Then you need to restart UGENE."));
    } else {
        FAIL("Unknown welcome page action", );
    }
}

void WelcomePageController::openUrl(const QString &urlId) {
    QString url = getUrlById(urlId);
    SAFE_POINT(!url.isEmpty(), "Unknown URL ID: " + urlId, );
    QDesktopServices::openUrl(QUrl(url));
}

void WelcomePageController::openFile(const QString &url) {
    QList<GUrl> urls;
    urls << url;
    Task *t = AppContext::getProjectLoader()->openWithProjectTask(urls);
    CHECK(NULL != t, );
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

QString WelcomePageController::getUrlById(const QString &urlId) {
    if ("facebook" == urlId) {
        return "https://www.facebook.com/groups/ugene";
    }
    if ("twitter" == urlId) {
        return "https://twitter.com/uniprougene";
    }
    if ("linkedin" == urlId) {
        return "https://www.linkedin.com/profile/view?id=200543736";
    }
    if ("google_plus" == urlId) {
        return "https://plus.google.com/u/0/110549832082666204900";
    }
    if ("youtube" == urlId) {
        return "http://www.youtube.com/user/UniproUGENE";
    }
    if ("vkontakte" == urlId) {
        return "http://vk.com/uniprougene";
    }
    if ("mendeley" == urlId) {
        return "http://www.mendeley.com/profiles/ugene-unipro/";
    }
    if ("rss" == urlId) {
        return "http://feeds2.feedburner.com/NewsOfUgeneProject";
    }
    if ("quick_start" == urlId) {
        return "https://ugene.unipro.ru/wiki/display/QSG/Quick+Start+Guide";
    }
    return "";
}

} // U2
