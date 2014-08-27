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

#include "ProjectTreeItemSelectorDialog.h"
#include "ProjectTreeItemSelectorDialogImpl.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/U2SafePoints.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPushButton>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QSpacerItem>
#else
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpacerItem>
#endif

#include <U2Gui/HelpButton.h>
#include <U2Gui/ProjectTreeController.h>

namespace U2 {

ProjectTreeItemSelectorDialogImpl::ProjectTreeItemSelectorDialogImpl(QWidget* p, const ProjectTreeControllerModeSettings& s) 
: QDialog(p)
{
    setupUi(this);
    controller = new ProjectTreeController(treeView, s, this);
    connect(controller, SIGNAL(si_doubleClicked(GObject*)), this, SLOT(sl_objectClicked(GObject*)));
    acceptByDoubleClick = false;

}

void ProjectTreeItemSelectorDialogImpl::sl_objectClicked(GObject* obj) {

    Document* d = obj->getDocument();
    assert(d != NULL);
    if (!d->isLoaded()) { 
        QAction* loadSelectedDocumentsAction = controller->getLoadSeletectedDocumentsAction();
        loadSelectedDocumentsAction->trigger();
        return;
    }
   
    if (acceptByDoubleClick) {
         accept();
    }
}

ProjectTreeItemSelectorDialogImpl::~ProjectTreeItemSelectorDialogImpl() {
    delete controller;
}

QList<Document*> ProjectTreeItemSelectorDialog::selectDocuments(const ProjectTreeControllerModeSettings& s, QWidget* p) {
    QList<Document*> res;
    ProjectTreeItemSelectorDialogImpl d(p, s);
    int rc = d.exec();
    if (rc == QDialog::Accepted) {
        const DocumentSelection* ds = d.controller->getDocumentSelection();
        res << ds->getSelectedDocuments();
    }
    return res;    
}

QList<GObject*> ProjectTreeItemSelectorDialog::selectObjects(const ProjectTreeControllerModeSettings& s, QWidget* p) {
    QList<GObject*> res;
    ProjectTreeItemSelectorDialogImpl d(p, s);
    int rc = d.exec();
    if (rc == QDialog::Accepted) {
        const GObjectSelection* os = d.controller->getGObjectSelection();
        res << os->getSelectedObjects();
    }
    return res;
}

void ProjectTreeItemSelectorDialog::selectObjectsAndDocuments(const ProjectTreeControllerModeSettings& s, QWidget* p, QList<Document*>& docList,
    QList<GObject*>& objList)
{
    ProjectTreeItemSelectorDialogImpl d(p, s);
    int rc = d.exec();
    if (rc == QDialog::Accepted) {
        const GObjectSelection* os = d.controller->getGObjectSelection();
        const DocumentSelection* ds = d.controller->getDocumentSelection();

        docList << ds->getSelectedDocuments();
        foreach (GObject* obj, os->getSelectedObjects()) {
            if (!docList.contains(obj->getDocument())) {
                objList << obj;
            }
        }
    }
}

void ProjectTreeItemSelectorDialog::selectObjectsAndFolders(const ProjectTreeControllerModeSettings &s, QWidget *p, QList<Folder> &folderList,
    QList<GObject *> &objList)
{
    ProjectTreeItemSelectorDialogImpl d(p, s);
    int rc = d.exec();
    if (rc == QDialog::Accepted) {
        SAFE_POINT(NULL != d.controller, "Invalid project tree controller", );
        folderList << d.controller->getSelectedFolders(); // add folders selected by a user

        const GObjectSelection* os = d.controller->getGObjectSelection();
        SAFE_POINT(NULL != os, "Invalid object selection", );
        foreach (GObject* obj, os->getSelectedObjects()) {
            bool objectIsAlreadySelected = false;
            foreach (const Folder &selectedFolder, folderList) {
                if (d.controller->isObjectInFolder(obj, selectedFolder)) {
                    objectIsAlreadySelected = true;
                    break;
                }
            }
            if (!objectIsAlreadySelected) {
                objList << obj; // add objects selected by the user that are not located in chosen folders
            }
        }
    }
}

}//namespace
