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

#include "GTUtilsDocument.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTMenu.h"
#include "GTUtilsProjectTreeView.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/qt/MessageBoxFiller.h"

#include <U2Gui/ObjectViewModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QTreeWidgetItem>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QTreeWidgetItem>
#endif


namespace U2 {

const QString GTUtilsDocument::DocumentUnloaded = "Unloaded";

#define GT_CLASS_NAME "GTUtilsDocument"

#define GT_METHOD_NAME "getDocument"
Document* GTUtilsDocument::getDocument(U2OpStatus &os, const QString& documentName) {

    Project* p = AppContext::getProject();
    GT_CHECK_RESULT(p != NULL, "Project does not exist", NULL);

    QList<Document*> docs = p->getDocuments();
    foreach (Document *d, docs) {
        if (d && (d->getName() == documentName)) {
            return d;
        }
    }

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkDocument"
void GTUtilsDocument::checkDocument(U2OpStatus &os, const QString &documentName, const GObjectViewFactoryId &id) {

    GTGlobals::sleep(1000);

    Document *d = getDocument(os, documentName);
    GT_CHECK(d != NULL, "There is no document with name " + documentName);

    if (id.isEmpty()) {
        return;
    }

    GObjectView* view = getDocumentGObjectView(os, d);
    if (id == DocumentUnloaded) {
        GT_CHECK(view == NULL, "GObjectView is not NULL");
        return;
    }

    GT_CHECK(view != NULL, "GObjectView* is NULL");
    GObjectViewFactoryId viewFactoryId = view->getFactoryId();
    GT_CHECK(viewFactoryId == id, "View's GObjectViewFactoryId is " + viewFactoryId + ", not " + id);
}
#undef GT_METHOD_NAME

void GTUtilsDocument::removeDocument(U2OpStatus &os, const QString &documentName, GTGlobals::UseMethod method)
{
    Runnable *popupChooser = new PopupChooser(os, QStringList() << ACTION_PROJECT__REMOVE_SELECTED, method);

    switch (method) {
    case GTGlobals::UseMouse:
    {
        GTUtilsDialog::waitForDialog(os, popupChooser);
        GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, documentName));

        GTMouseDriver::click(os, Qt::RightButton);
        break;
    }

    default:
    case GTGlobals::UseKey:
        GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, documentName));
        GTMouseDriver::click(os);

        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
        break;
    }

    GTGlobals::sleep(500);
}

#define GT_METHOD_NAME "getDocumentGObjectView"
GObjectView* GTUtilsDocument::getDocumentGObjectView(U2OpStatus &os, Document* d) {

    GT_CHECK_RESULT(d != NULL, "Document* is NULL", NULL);

    QList<GObjectView*> gObjectViews = getAllGObjectViews();
    foreach (GObjectView *view, gObjectViews) {
        if (view->containsDocumentObjects(d)) {
            return view;
        }
    }

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isDocumentLoaded"
bool GTUtilsDocument::isDocumentLoaded(U2OpStatus &os, const QString& documentName)
{
    QModelIndex index = GTUtilsProjectTreeView::findIndex(os, GTUtilsProjectTreeView::getTreeView(os), documentName);
    QString s = index.data().toString();
    return !s.contains("unloaded");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "saveDocument"
void GTUtilsDocument::saveDocument(U2OpStatus &os, const QString &documentName) {
    Runnable *popupChooser = new PopupChooser(os, QStringList() << ACTION_PROJECT__SAVE_DOCUMENT, GTGlobals::UseMouse);

    GTUtilsDialog::waitForDialog(os, popupChooser);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, documentName));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "unloadDocument"
void GTUtilsDocument::unloadDocument(U2OpStatus &os, const QString &documentName, bool waitForMessageBox) {
    GT_CHECK_RESULT( isDocumentLoaded(os, documentName), "Document is not loaded", );

    Runnable *popupChooser = new PopupChooser(os, QStringList() << ACTION_PROJECT__UNLOAD_SELECTED, GTGlobals::UseMouse);

    if (waitForMessageBox) {
        MessageBoxDialogFiller *filler = new MessageBoxDialogFiller(os, "Yes");
        GTUtilsDialog::waitForDialog(os, filler);
    }

    GTUtilsDialog::waitForDialog(os, popupChooser);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, documentName));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "loadDocument"
void GTUtilsDocument::loadDocument(U2OpStatus &os, const QString &documentName) {
    GT_CHECK_RESULT( !isDocumentLoaded(os, documentName), "Document is loaded", );

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, documentName));
    GTMouseDriver::doubleClick(os);

    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

QList<GObjectView*> GTUtilsDocument::getAllGObjectViews() {

    QList<GObjectView*> gObjectViews;

    MWMDIManager* mwMDIManager = AppContext::getMainWindow()->getMDIManager();
    QList<MWMDIWindow*> windows = mwMDIManager->getWindows();

    foreach (MWMDIWindow *w, windows) {
        if (GObjectViewWindow* gObjectViewWindow = qobject_cast<GObjectViewWindow*>(w)) {
            if (GObjectView* gObjectView = gObjectViewWindow->getObjectView()) {
                gObjectViews.append(gObjectView);
            }
        }
    }

    return gObjectViews;
}

#undef GT_CLASS_NAME

}
