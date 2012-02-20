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
#include <QDropEvent>
#include <QUrl>
#include <U2Core/ProjectModel.h>
#include <U2Core/Task.h>
#include <U2Test/GUITestBase.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/U2SafePoints.h>

#include "GUIDialogUtils.h"

namespace U2 {

void ProjectUtils::openFile(U2OpStatus &os, const GUrl &path, const OpenFileSettings& settings) {

	switch (settings.openMethod) {
		case OpenFileSettings::DRAGDROP:
		default:
			openFileDrop(os, path);
	}

	QtUtils::sleep(3000);

	checkProjectExists(os);
	Document* doc = checkDocumentExists(os, path);
	checkDocumentActive(os, doc);
}

void ProjectUtils::checkDocumentExists(U2OpStatus &os, const QString &documentName) {

	Project* p = AppContext::getProject();
	if (!p) {
		return;
	}

	QList<Document*> docs = p->getDocuments();
	foreach (Document *d, docs) {
		if (d && (d->getName() == documentName)) {
			return;
		}
	}

	os.setError("There is no document with name " + documentName);
}

void ProjectUtils::saveProjectAs(U2OpStatus &os, const QString &projectName, const QString &projectFolder, const QString &projectFile, bool overwriteExisting) {

	QtUtils::clickMenuAction(os, ACTION_PROJECTSUPPORT__SAVE_AS_PROJECT, MWMENU_FILE);

	GUIDialogUtils::fillInSaveProjectAsDialog(os, projectName, projectFolder, projectFile);

	QMessageBox::StandardButton b = overwriteExisting ? QMessageBox::Yes : QMessageBox::No;
	GUIDialogUtils::clickMessageBoxButton(os, b);

	GUIDialogUtils::fillInSaveProjectAsDialog(os, projectName, projectFolder, projectFile, true);
}

void ProjectUtils::closeProject(U2OpStatus &os, const CloseProjectSettings& settings) {

	QtUtils::clickMenuAction(os, ACTION_PROJECTSUPPORT__CLOSE_PROJECT, MWMENU_FILE);
	QtUtils::sleep(500);

	switch (settings.saveOnClose) {
		case CloseProjectSettings::YES:
			GUIDialogUtils::clickMessageBoxButton(os, QMessageBox::Yes);
			break;

		case CloseProjectSettings::NO:
 			GUIDialogUtils::clickMessageBoxButton(os, QMessageBox::No);
			break;

		default:
		case CloseProjectSettings::CANCEL:
			GUIDialogUtils::clickMessageBoxButton(os, QMessageBox::Cancel);
			break;
	}
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

void ProjectUtils::openFileDrop(U2OpStatus &os, const GUrl &path) {

	QWidget* widget = AppContext::getMainWindow()->getQMainWindow();
	QPoint widgetPos(widget->width()/2, widget->height()/2);

	QList<QUrl> urls;
	urls << path.getURLString();

	QMimeData *mimeData = new QMimeData();
	mimeData->setUrls(urls);

	Qt::DropActions dropActions = Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
	Qt::MouseButtons mouseButtons = Qt::LeftButton;

	QDragEnterEvent* dragEnterEvent = new QDragEnterEvent(widgetPos, dropActions, mimeData, mouseButtons, 0);
	QtUtils::sendEvent(widget, dragEnterEvent);

	QDropEvent* dropEvent = new QDropEvent(widgetPos, dropActions, mimeData, mouseButtons, 0);
	QtUtils::sendEvent(widget, dropEvent);
}


QPoint ProjectUtils::getTreeViewItemPosition(U2OpStatus &os, int num) {

	QTreeWidget *tree = static_cast<QTreeWidget*>(QtUtils::findWidgetByName(os, "documentTreeWidget"));
	CHECK_SET_ERR_RESULT(tree != NULL, QString("Tree Item not found").arg("documentTreeWidget"), QPoint());

	QTreeWidgetItem *item = tree->topLevelItem(num);
	CHECK_SET_ERR_RESULT(item != NULL, QString("Item %d not found").arg(QString::number(num)), QPoint());

	return tree->visualItemRect(item).center();
}


} // U2
