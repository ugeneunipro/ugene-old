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

#include "ProjectUtils.h"
#include "GTGlobals.h"
#include "GUIDialogUtils.h"
#include "api/GTKeyboardDriver.h"
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Gui/ObjectViewModel.h>
#include <QtGui/QMainWindow>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>

namespace U2 {

void ProjectUtils::openProject(U2OpStatus& os, const GUrl& path, const QString& projectName, const QString& documentName) {

    openFiles(os, path);
    AppUtils::checkUGENETitle(os, projectName);
    DocumentUtils::checkDocument(os, documentName);
}

void ProjectUtils::openFiles(U2OpStatus &os, const QList<QUrl> &urls, const OpenFileSettings& s) {

    switch (s.openMethod) {
        case OpenFileSettings::DRAGDROP:
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

void ProjectUtils::openFiles(U2OpStatus &os, const GUrl &path, const OpenFileSettings& s) {
    openFiles(os, QList<QUrl>() << path.getURLString(), s);
}

void ProjectUtils::exportProject(U2OpStatus &os, const QString &projectFolder, const QString &projectName) {

    GUIDialogUtils::openExportProjectDialog(os);
    GUIDialogUtils::ExportProjectDialogFiller filler(os, projectFolder, projectName);
    GUIDialogUtils::waitForDialog(os, &filler);
}

void ProjectUtils::exportProjectCheck(U2OpStatus &os, const QString &projectName) {

    GUIDialogUtils::openExportProjectDialog(os);
    GUIDialogUtils::ExportProjectDialogChecker filler(os, projectName);
    GUIDialogUtils::waitForDialog(os, &filler);
}

void ProjectUtils::saveProjectAs(U2OpStatus &os, const QString &projectName, const QString &projectFolder, const QString &projectFile) {

    GUIDialogUtils::openSaveProjectAsDialog(os);
    GUIDialogUtils::SaveProjectAsDialogFiller filler(os, projectName, projectFolder, projectFile);
    GUIDialogUtils::waitForDialog(os, &filler);
}

void ProjectUtils::closeProject(U2OpStatus &os, const CloseProjectSettings& settings) {

    GTGlobals::clickMenuAction(os, ACTION_PROJECTSUPPORT__CLOSE_PROJECT, MWMENU_FILE);
    GUIDialogUtils::MessageBoxDialogFiller filler(os, settings.saveOnCloseButton);
    GUIDialogUtils::waitForDialog(os, &filler, false);
}

void ProjectUtils::closeProjectByHotkey(U2OpStatus &os) {

    GTKeyboardDriver::keyClick(os, 'q', GTKeyboardDriver::key["ctrl"]);
}

void ProjectUtils::checkProject(U2OpStatus &os, CheckType checkType) {

    CHECK_SET_ERR(AppContext::getProject() != NULL, "There is no project");

    switch (checkType) {
        case EMPTY :
            CHECK_SET_ERR(AppContext::getProject()->getDocuments().isEmpty() == true, "Project is not empty");
        default:
            break;
    }
}

Document* ProjectUtils::checkDocumentExists(U2OpStatus &os, const GUrl &url) {

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

void ProjectUtils::checkDocumentActive(U2OpStatus &os, Document *doc) {

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

void ProjectUtils::openFilesDrop(U2OpStatus &os, const QList<QUrl>& urls) {

    QWidget* widget = AppContext::getMainWindow()->getQMainWindow();
    QPoint widgetPos(widget->width()/2, widget->height()/2);

    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls(urls);

    Qt::DropActions dropActions = Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
    Qt::MouseButtons mouseButtons = Qt::LeftButton;

    QDragEnterEvent* dragEnterEvent = new QDragEnterEvent(widgetPos, dropActions, mimeData, mouseButtons, 0);
    GTGlobals::sendEvent(widget, dragEnterEvent);

    QDropEvent* dropEvent = new QDropEvent(widgetPos, dropActions, mimeData, mouseButtons, 0);
    GTGlobals::sendEvent(widget, dropEvent);
}

} // U2
