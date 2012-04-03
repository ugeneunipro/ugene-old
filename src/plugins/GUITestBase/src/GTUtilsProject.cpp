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

#include "GTUtilsProject.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTSequenceReadingModeDialogUtils.h"
#include "api/GTMenu.h"
#include "GTUtilsDialog.h"
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Gui/ObjectViewModel.h>
#include <QtGui/QMainWindow>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>

namespace U2 {

void GTUtilsProject::openProject(U2OpStatus& os, const GUrl& path, const QString& projectName, const QString& documentName) {

    openFiles(os, path);
    GTUtilsApp::checkUGENETitle(os, projectName);
    GTUtilsDocument::checkDocument(os, documentName);
}

void GTUtilsProject::openFiles(U2OpStatus &os, const QList<QUrl> &urls, const OpenFileSettings& s) {

    switch (s.openMethod) {
        case OpenFileSettings::DragDrop:
        default:
            openFilesDrop(os, urls);
    }

    GTGlobals::sleep(500);

    checkProject(os);
    foreach (QUrl path, urls) {
        checkDocumentExists(os, path.toString());
    }

    GTGlobals::sleep(1000);
}

void GTUtilsProject::openFiles(U2OpStatus &os, const GUrl &path, const OpenFileSettings& s) {
    openFiles(os, QList<QUrl>() << path.getURLString(), s);
}

void GTUtilsProject::exportProject(U2OpStatus &os, const QString &projectFolder, const QString &projectName) {

    GTUtilsDialog::openExportProjectDialog(os);
    GTUtilsDialog::ExportProjectDialogFiller filler(os, projectFolder, projectName);
    GTUtilsDialog::waitForDialog(os, &filler);
}

void GTUtilsProject::exportProjectCheck(U2OpStatus &os, const QString &projectName) {

    GTUtilsDialog::openExportProjectDialog(os);
    GTUtilsDialog::ExportProjectDialogChecker filler(os, projectName);
    GTUtilsDialog::waitForDialog(os, &filler);
}

void GTUtilsProject::saveProjectAs(U2OpStatus &os, const QString &projectName, const QString &projectFolder, const QString &projectFile) {

    GTUtilsDialog::openSaveProjectAsDialog(os);
    GTUtilsDialog::SaveProjectAsDialogFiller filler(os, projectName, projectFolder, projectFile);
    GTUtilsDialog::waitForDialog(os, &filler);
}

void GTUtilsProject::closeProject(U2OpStatus &os, const CloseProjectSettings& settings) {

    QMenu *m = GTMenu::showMainMenu(os, MWMENU_FILE);
    GTMenu::clickMenuItem(os, m, ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTUtilsDialog::MessageBoxDialogFiller filler(os, settings.saveOnCloseButton);
    GTUtilsDialog::waitForDialog(os, &filler, false);
}

void GTUtilsProject::closeProjectByHotkey(U2OpStatus &os) {

    GTKeyboardDriver::keyClick(os, 'q', GTKeyboardDriver::key["ctrl"]);
}

void GTUtilsProject::checkProject(U2OpStatus &os, CheckType checkType) {

    CHECK_SET_ERR(AppContext::getProject() != NULL, "There is no project");

    switch (checkType) {
        case Empty:
            CHECK_SET_ERR(AppContext::getProject()->getDocuments().isEmpty() == true, "Project is not empty");
        default:
            break;
    }
}

Document* GTUtilsProject::checkDocumentExists(U2OpStatus &os, const GUrl &url) {

    Project *project = AppContext::getProject();
    if (!project) {
        return NULL;
    }

    QString projectUrl = project->getProjectURL();
    if (url == projectUrl) {
        return NULL; // url of a project
    }

    // check if document with url exists and was shown
    Document *doc = project->findDocumentByURL(url);

    CHECK_SET_ERR_RESULT(doc != NULL, "There is no document", NULL);
    CHECK_SET_ERR_RESULT(doc->isLoaded(), "Document is not loaded", NULL);

    return doc;
}

void GTUtilsProject::checkDocumentActive(U2OpStatus &os, Document *doc) {

    Q_UNUSED(os);
    if (!doc) {
        return;
    }

    MWMDIWindow *activeWindow = AppContext::getMainWindow()->getMDIManager()->getActiveWindow();
    CHECK_SET_ERR(activeWindow != NULL, "There is no active window");

    const QList<GObject*>& docObjects = doc->getObjects();
    CHECK_SET_ERR(!docObjects.isEmpty(), "Document has no objects");

    QList<GObjectViewWindow*> viewsList = GObjectViewUtils::findViewsWithAnyOfObjects(docObjects);
    CHECK_SET_ERR(!viewsList.isEmpty(), "View is not loaded");

    MWMDIWindow *documentWindow = viewsList.first();
    CHECK_SET_ERR(documentWindow == activeWindow, "documentWindow is not active");
}

void GTUtilsProject::openFilesDrop(U2OpStatus &os, const QList<QUrl>& urls) {

    QWidget* widget = AppContext::getMainWindow()->getQMainWindow();
    QPoint widgetPos(widget->width()/2, widget->height()/2);

    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls(urls);

    Qt::DropActions dropActions = Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
    Qt::MouseButtons mouseButtons = Qt::LeftButton;
    GTSequenceReadingModeDialogUtils ob(os);

    if (urls.size() > 1) {
        GTUtilsDialog::preWaitForDialog(os, &ob);
    }

    QDragEnterEvent* dragEnterEvent = new QDragEnterEvent(widgetPos, dropActions, mimeData, mouseButtons, 0);
    GTGlobals::sendEvent(widget, dragEnterEvent);

    QDropEvent* dropEvent = new QDropEvent(widgetPos, dropActions, mimeData, mouseButtons, 0);
    GTGlobals::sendEvent(widget, dropEvent);
}

} // U2
