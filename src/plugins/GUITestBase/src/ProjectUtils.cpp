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

void ProjectUtils::OpenFilesGUIAction::addSubTests() {

    switch (s.openMethod) {
        case OpenFileSettings::DRAGDROP:
        default:
            add( new OpenFilesDropGUIAction(urls) );
    }

    add( new CheckProjectGUIAction() );
    foreach (QUrl path, urls) {
        add( new CheckDocumentExistsGUIAction(path.toString()) );
    }
}

ProjectUtils::SaveProjectAsGUIAction::SaveProjectAsGUIAction(const QString &projectName, const QString &projectFolder, const QString &projectFile, bool overwriteExisting) {

    add( new GUIDialogUtils::OpenSaveProjectAsDialogGUIAction() );
    add( new GUIDialogUtils::FillInSaveProjectAsDialogGUIAction(projectName, projectFolder, projectFile) );

    QMessageBox::StandardButton b = overwriteExisting ? QMessageBox::Yes : QMessageBox::No;
    add( new GUIDialogUtils::ClickMessageBoxButtonGUIAction(b) );

    add( new GUIDialogUtils::FillInSaveProjectAsDialogGUIAction(projectName, projectFolder, projectFile, true) );
}

ProjectUtils::CloseProjectGUIAction::CloseProjectGUIAction(const CloseProjectSettings& settings) {

    add( new QtUtils::ClickMenuActionGUIAction(ACTION_PROJECTSUPPORT__CLOSE_PROJECT, MWMENU_FILE) );

    switch (settings.saveOnClose) {
        case CloseProjectSettings::YES:
            add( new GUIDialogUtils::ClickMessageBoxButtonGUIAction(QMessageBox::Yes) );
            break;

        case CloseProjectSettings::NO:
            add( new GUIDialogUtils::ClickMessageBoxButtonGUIAction(QMessageBox::No) );
            break;

        default:
        case CloseProjectSettings::CANCEL:
            add( new GUIDialogUtils::ClickMessageBoxButtonGUIAction(QMessageBox::Cancel) );
            break;
    }
}

void ProjectUtils::closeProjectByHotkey(U2OpStatus &os) {

    QtUtils::keyClick(os, MWMENU, Qt::Key_Q, Qt::ControlModifier);
    QtUtils::sleep(1000);
}

void ProjectUtils::CheckProjectGUIAction::execute(U2OpStatus &os) {

    CHECK_SET_ERR(AppContext::getProject() != NULL, "There is no project");

    switch (checkType) {
        case EMPTY :
            CHECK_SET_ERR(AppContext::getProject()->getDocuments().isEmpty() == true, "Project is not empty");
    }
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

void ProjectUtils::OpenFilesDropGUIAction::execute(U2OpStatus &os) {

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
