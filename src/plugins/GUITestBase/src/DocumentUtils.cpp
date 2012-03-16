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

#include "DocumentUtils.h"
#include "QtUtils.h"

#include <U2Core/U2SafePoints.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/ProjectModel.h>
#include "ProjectTreeViewUtils.h"

#include "api/GTKeyboardDriver.h"

namespace U2 {

Document* DocumentUtils::getDocument(U2OpStatus &os, const QString& documentName) {

    Project* p = AppContext::getProject();
    CHECK_SET_ERR_RESULT(p != NULL, "Project does not exist", NULL);

    QList<Document*> docs = p->getDocuments();
    foreach (Document *d, docs) {
        if (d && (d->getName() == documentName)) {
            return d;
        }
    }

    return NULL;
}

void DocumentUtils::checkDocumentExists(U2OpStatus &os, const QString &documentName, const GObjectViewFactoryId &id) {

    Document *d = getDocument(os, documentName);
    CHECK_SET_ERR(d != NULL, "There is no document with name " + documentName);

    if (id.isEmpty()) {
        return;
    }

    GObjectView* view = getDocumentGObjectView(os, d);
    CHECK_SET_ERR(view != NULL, "GObjectView* is NULL");

    GObjectViewFactoryId viewFactoryId = view->getFactoryId();
    CHECK_SET_ERR(viewFactoryId == id, "View's GObjectViewFactoryId is " + viewFactoryId + ", not " + id);
}

DocumentUtils::RemoveDocumentGUIAction::RemoveDocumentGUIAction(const QString &documentName) {

    add( new ProjectTreeViewUtils::ClickGUIAction(documentName) );
    add( new GTKeyboardDriver::KeyClickGUIAction(GTKeyboardDriver::key["delete"]) );
}

GObjectView* DocumentUtils::getDocumentGObjectView(U2OpStatus &os, Document* d) {

    CHECK_SET_ERR_RESULT(d != NULL, "Document* is NULL", NULL);

    QList<GObjectView*> gObjectViews = getAllGObjectViews();
    foreach (GObjectView *view, gObjectViews) {
        if (view->containsDocumentObjects(d)) {
            return view;
        }
    }

    return NULL;
}

QList<GObjectView*> DocumentUtils::getAllGObjectViews() {

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

}
