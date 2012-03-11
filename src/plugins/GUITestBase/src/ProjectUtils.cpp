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
#include "QtUtils.h"
#include <U2Core/ProjectModel.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/U2SafePoints.h>

#include "GUIDialogUtils.h"

namespace U2 {

void ProjectUtils::openFiles(U2OpStatus &os, const QList<QUrl> &urls, const OpenFileSettings& settings) {

    switch (settings.openMethod) {
        case OpenFileSettings::DRAGDROP:
        default:
            openFilesDrop(os, urls);
    }

//    QtUtils::sleep(3000); // TODO:
//    checkProjectExists(os);

//     foreach (QUrl path, urls) {
//         GUrl pathStr = path.toString();
//         checkDocumentExists(os, pathStr);
//     }
}

void ProjectUtils::saveProjectAs(U2OpStatus &os, const QString &projectName, const QString &projectFolder, const QString &projectFile, bool overwriteExisting) {

    GUIDialogUtils::openSaveProjectAsDialog(os);
    GUIDialogUtils::fillInSaveProjectAsDialog(os, projectName, projectFolder, projectFile);

    QMessageBox::StandardButton b = overwriteExisting ? QMessageBox::Yes : QMessageBox::No;
    GUIDialogUtils::clickMessageBoxButton(os, b);

    GUIDialogUtils::fillInSaveProjectAsDialog(os, projectName, projectFolder, projectFile, true);
}

void ProjectUtils::closeProject(U2OpStatus &os, const CloseProjectSettings& settings) {

    QtUtils::clickMenuAction(os, ACTION_PROJECTSUPPORT__CLOSE_PROJECT, MWMENU_FILE);
    QtUtils::sleep(500);

//     switch (settings.saveOnClose) {
//         case CloseProjectSettings::YES:
//             GUIDialogUtils::clickMessageBoxButton(os, QMessageBox::Yes);
//             break;
// 
//         case CloseProjectSettings::NO:
//             GUIDialogUtils::clickMessageBoxButton(os, QMessageBox::No);
//             break;
// 
//         default:
//         case CloseProjectSettings::CANCEL:
//             GUIDialogUtils::clickMessageBoxButton(os, QMessageBox::Cancel);
//             break;
//     }
}

void ProjectUtils::closeProjectByHotkey(U2OpStatus &os) {

    QtUtils::keyClick(os, MWMENU, Qt::Key_Q, Qt::ControlModifier);
    QtUtils::sleep(1000);
}

void ProjectUtils::checkProjectExists(U2OpStatus &os) {

    // check if project exists
    CHECK_SET_ERR(AppContext::getProject() != NULL, "There is no project");
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

void ProjectUtils::openFilesDrop(U2OpStatus &os, const QList<QUrl> &urls) {

    QWidget* widget = AppContext::getMainWindow()->getQMainWindow();
    QPoint widgetPos(widget->width()/2, widget->height()/2);

    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls(urls);

    Qt::DropActions dropActions = Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
    Qt::MouseButtons mouseButtons = Qt::LeftButton;

    QDragEnterEvent* dragEnterEvent = new QDragEnterEvent(widgetPos, dropActions, mimeData, mouseButtons, 0);
    QtUtils::sendEvent(widget, dragEnterEvent);

    QDropEvent* dropEvent = new QDropEvent(widgetPos, dropActions, mimeData, mouseButtons, 0);
    QtUtils::sendEvent(widget, dropEvent);
}

} // U2
