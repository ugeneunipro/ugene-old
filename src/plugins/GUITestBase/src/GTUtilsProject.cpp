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
#include "api/GTLineEdit.h"
#include "api/GTWidget.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsDialogRunnables.h"
#include "GTUtilsAnnotationsTreeView.h"

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2View/ADVConstants.h>
#include <U2Gui/ObjectViewModel.h>
#include <QtGui/QMainWindow>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QHeaderView>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsProject"

void GTUtilsProject::openFiles(U2OpStatus &os, const QList<QUrl> &urls, const OpenFileSettings& s) {

	switch (s.openMethod) {
		case OpenFileSettings::DragDrop:
		default:
			openFilesDrop(os, urls);
	}

	checkProject(os);
}

void GTUtilsProject::openFiles(U2OpStatus &os, const GUrl &path, const OpenFileSettings& s) {
	openFiles(os, QList<QUrl>() << path.getURLString(), s);
}

void GTUtilsProject::exportProject(U2OpStatus &os, const QString &projectFolder, const QString &projectName) {

	GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__EXPORT_PROJECT);
	GTUtilsDialogRunnables::ExportProjectDialogFiller filler(os, projectFolder, projectName);
	GTUtilsDialog::waitForDialog(os, &filler);
}
void GTUtilsProject::createAnnotation(U2OpStatus &os, const QString &groupName, const QString &annotationName, const QString &location) {
 
	GTUtilsDialogRunnables::CreateAnnotationDialogFiller filler(os, groupName, annotationName, location);
	GTUtilsDialog::preWaitForDialog(os, &filler);
	GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
	GTGlobals::sleep(1000);
}

void GTUtilsProject::exportProjectCheck(U2OpStatus &os, const QString &projectName) {

	GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__EXPORT_PROJECT);
	GTUtilsDialogRunnables::ExportProjectDialogChecker filler(os, projectName);
	GTUtilsDialog::waitForDialog(os, &filler);
}

void GTUtilsProject::exportToSequenceFormat(U2OpStatus &os, const QString &projectName, const QString &path, const QString &name, GTGlobals::UseMethod method)
{
	GTUtilsDialogRunnables::PopupChooser popupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_MENU_ACTION << ACTION_PROJECT__EXPORT_AS_SEQUENCES_ACTION, method);
	GTUtilsDialogRunnables::ExportToSequenceFormatFiller filler(os, path, name, method);

	switch (method) {
	case GTGlobals::UseMouse:
	{
		GTUtilsDialog::preWaitForDialog(os, &popupChooser, GUIDialogWaiter::Popup);
		GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, projectName));
		GTUtilsDialog::preWaitForDialog(os, &filler, GUIDialogWaiter::Modal);
		GTMouseDriver::click(os, Qt::RightButton);
		break;
	}

	default:
	case GTGlobals::UseKey:
		///TODO

		break;
	}

	GTGlobals::sleep(500);
}

void GTUtilsProject::exportSequenceAsAlignment(U2OpStatus &os, const QString projectName, const QString &path, const QString &name,
											   GTUtilsDialogRunnables::ExportSequenceAsAlignmentFiller::FormatToUse format, bool addDocToProject, GTGlobals::UseMethod method)
{
	GTUtilsDialogRunnables::PopupChooser popupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT, method);
	GTUtilsDialogRunnables::ExportSequenceAsAlignmentFiller filler(os, path, name, format, addDocToProject, method);

	switch (method) {
	case GTGlobals::UseMouse:
	{
		GTUtilsDialog::preWaitForDialog(os, &popupChooser, GUIDialogWaiter::Popup);
		GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, projectName));
		GTUtilsDialog::preWaitForDialog(os, &filler, GUIDialogWaiter::Modal);
		GTMouseDriver::click(os, Qt::RightButton);
		break;
	}

	default:
	case GTGlobals::UseKey:
		///TODO

		break;
	}

	GTGlobals::sleep(500);
}

void GTUtilsProject::exportSequenceOfSelectedAnnotations(U2OpStatus &os, const QString &itemToClick, const QString &path,
														 GTUtilsDialogRunnables::ExportSequenceOfSelectedAnnotationsFiller::FormatToUse format,
														 GTUtilsDialogRunnables::ExportSequenceOfSelectedAnnotationsFiller::MergeOptions options,
														 int gapLength, bool addDocToProject, bool exportWithAnnotations, GTGlobals::UseMethod method)
{
	GTUtilsDialogRunnables::PopupChooser popupChooser(os, QStringList() << "ADV_MENU_EXPORT" << "action_export_sequence_of_selected_annotations", GTGlobals::UseKey);
	GTUtilsDialogRunnables::ExportSequenceOfSelectedAnnotationsFiller filler(os, path, format, options, gapLength, addDocToProject, exportWithAnnotations);
	QPoint pos;

	switch (method) {
	case GTGlobals::UseKey:
		//TODO
	case GTGlobals::UseMouse:
	{
		pos = GTUtilsAnnotationsTreeView::getItemCenter(os, itemToClick);
		GTMouseDriver::moveTo(os, pos);
		GTMouseDriver::doubleClick(os);
		GTGlobals::sleep(200);

		GTUtilsDialog::preWaitForDialog(os, &popupChooser, GUIDialogWaiter::Popup);
		GTUtilsDialog::preWaitForDialog(os, &filler);
		GTMouseDriver::click(os, Qt::RightButton);
		break;
	}
	}

	GTGlobals::sleep(500);
}
void GTUtilsProject::exportAnnotations(U2OpStatus &os, const QString &itemToClick, const QString &exportToFile,
									   GTUtilsDialogRunnables::ExportAnnotationsFiller::fileFormat format,
									   bool saveSequencesUnderAnnotations, bool saveSequenceNames, GTGlobals::UseMethod method)
{
	GTUtilsDialogRunnables::PopupChooser popupChooser(os, QStringList() << "ADV_MENU_EXPORT" << "action_export_annotations", GTGlobals::UseKey);
	GTUtilsDialogRunnables::ExportAnnotationsFiller filler(os, exportToFile, format, saveSequencesUnderAnnotations, saveSequenceNames);
	QPoint pos;

	switch (method) {
	case GTGlobals::UseKey:
		//TODO
	case GTGlobals::UseMouse:
	{
		pos = GTUtilsAnnotationsTreeView::getItemCenter(os, itemToClick);
		GTMouseDriver::moveTo(os, pos);
		GTMouseDriver::doubleClick(os);
		GTGlobals::sleep(200);

		GTUtilsDialog::preWaitForDialog(os, &popupChooser, GUIDialogWaiter::Popup);
		GTUtilsDialog::preWaitForDialog(os, &filler);
		GTMouseDriver::click(os, Qt::RightButton);
		break;
	}
	}

	GTGlobals::sleep(500);
}

void GTUtilsProject::createDocument(U2OpStatus &os, const QString &pasteDataHere, const QString &documentLocation, 
									GTUtilsDialogRunnables::CreateDocumentFiller::documentFormat format, const QString &sequenceName, 
									GTGlobals::UseMethod method)
{
	GTUtilsDialogRunnables::CreateDocumentFiller filler(os, pasteDataHere, documentLocation, format, sequenceName);
	GTUtilsDialog::preWaitForDialog(os, &filler);
	GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), "NewDocumentFromText", GTGlobals::UseKey);
	
	GTGlobals::sleep(1000);
	QPoint pos;
}

void GTUtilsProject::insertSequence(U2OpStatus &os, const QString &pasteDataHere, const QString &documentLocation, 
									GTUtilsDialogRunnables::InsertSequenceFiller::documentFormat format,
									bool saveToNewFile, bool mergeAnnotations, GTGlobals::UseMethod method)
{
	GTUtilsDialogRunnables::InsertSequenceFiller filler(os, pasteDataHere, GTUtilsDialogRunnables::InsertSequenceFiller::Resize, 0, documentLocation, format, saveToNewFile, mergeAnnotations, method);
	GTUtilsDialog::preWaitForDialog(os, &filler);
	GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_ACTIONS), QStringList() <<  ADV_MENU_EDIT << ACTION_EDIT_INSERT_SUBSEQUENCE, GTGlobals::UseKey);
	GTGlobals::sleep(1000);
}

void GTUtilsProject::saveProjectAs(U2OpStatus &os, const QString &projectName, const QString &projectFolder, const QString &projectFile) {

	GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__SAVE_AS_PROJECT);
	GTUtilsDialogRunnables::SaveProjectAsDialogFiller filler(os, projectName, projectFolder, projectFile);
	GTUtilsDialog::waitForDialog(os, &filler);
}

void GTUtilsProject::closeProject(U2OpStatus &os, const CloseProjectSettings& settings) {

	GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
	GTUtilsDialogRunnables::MessageBoxDialogFiller filler(os, settings.saveOnCloseButton);
	GTUtilsDialog::waitForDialog(os, &filler, GUIDialogWaiter::Modal, false);
}

void GTUtilsProject::exitProject(U2OpStatus &os, const ExitProjectSettings& exitSettings) {

	GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION__EXIT);
	GTUtilsDialogRunnables::MessageBoxDialogFiller filler(os, exitSettings.saveNoCloseButton);
	GTUtilsDialog::waitForDialog(os, &filler, GUIDialogWaiter::Modal, false);
}


#define GT_METHOD_NAME "checkProject"
void GTUtilsProject::checkProject(U2OpStatus &os, CheckType checkType) {

	GTGlobals::sleep(500);
	GT_CHECK(AppContext::getProject() != NULL, "There is no project");

	switch (checkType) {
		case Empty:
			GT_CHECK(AppContext::getProject()->getDocuments().isEmpty() == true, "Project is not empty");
		default:
			break;
	}
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setFilter"
void GTUtilsProject::setFilter(U2OpStatus &os, const QString& filter) {

	QLineEdit* nameFilterEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "nameFilterEdit"));
	GT_CHECK(nameFilterEdit != NULL, "nameFilterEdit is NULL");

	GTLineEdit::setText(os, nameFilterEdit, filter);
	GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

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

#undef GT_CLASS_NAME

} // U2
