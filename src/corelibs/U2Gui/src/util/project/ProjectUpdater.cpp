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

#include <U2Core/DocumentModel.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/ObjectViewModel.h>

#include "DocumentFolders.h"
#include "ProjectUtils.h"

#include "ProjectUpdater.h"

namespace U2 {

ProjectUpdater::ProjectUpdater()
    : QThread(), mutex(QMutex::Recursive), stopped(0)
{
    moveToThread(this);
}

void ProjectUpdater::run() {
    while (0 == stopped) {
        readData();
        msleep(U2ObjectDbi::OBJECT_ACCESS_UPDATE_INTERVAL);
    }
}

void ProjectUpdater::stop() {
    stopped = 1;
}

void ProjectUpdater::invalidate(const Document *doc) {
    QMutexLocker lock(&mutex);
    valid[doc->getDbiRef().dbiId] = false;
}

bool ProjectUpdater::takeData(Document *doc, DocumentFoldersUpdate &result) {
    QMutexLocker lock(&mutex);
    CHECK(valid.value(doc->getDbiRef().dbiId, true), false);
    CHECK(data.contains(doc->getDbiRef().dbiId), false);
    result = data.take(doc->getDbiRef().dbiId);
    return true;
}

void ProjectUpdater::addDocument(Document *doc) {
    QMutexLocker lock(&mutex);
    docs << doc;
}

void ProjectUpdater::removeDocument(Document *doc) {
    QMutexLocker lock(&mutex);
    docs.removeAll(doc);
    data.remove(doc->getDbiRef().dbiId);
}

QList<U2DbiRef> ProjectUpdater::getDbiRefs() {
    QMutexLocker lock(&mutex);
    QList<U2DbiRef> result;
    foreach (Document *doc, docs) {
        if (!ProjectUtils::isConnectedDatabaseDoc(doc)) {
            continue;
        }
        result << doc->getDbiRef();
    }
    return result;
}

void ProjectUpdater::readData() {
    updateAccessedObjects();

    QList<U2DbiRef> refs = getDbiRefs();
    foreach (const U2DbiRef &dbiRef, refs) {
        bool repeat = false;
        do {
            {
                QMutexLocker lock(&mutex);
                if (!valid.value(dbiRef.dbiId, true)) {
                    data.take(dbiRef.dbiId);
                    valid[dbiRef.dbiId] = true;
                }
            }
            U2OpStatus2Log os;
            DocumentFoldersUpdate update(dbiRef, os);
            if (!os.hasError()) {
                QMutexLocker lock(&mutex);
                if (valid.value(dbiRef.dbiId, true)) {
                    data[dbiRef.dbiId] = update;
                    repeat = false;
                } else {
                    // repeat only if document is in the list
                    repeat = getDbiRefs().contains(dbiRef);
                }
            }
        } while (repeat);
    }
}

void ProjectUpdater::updateAccessedObjects() {
    const QList<GObjectViewWindow *> activeViews = GObjectViewUtils::getAllActiveViews();
    QMap<U2DbiRef, DbiConnection *> dbiRef2Connections; // when changing the code below, watch out mem leaks
    U2OpStatus2Log os;
    foreach (GObjectViewWindow *view, activeViews) {
        foreach (GObject *object, view->getObjects()) {
            Document *doc = object->getDocument();
            if (NULL != doc && doc->isStateLocked()) {
                continue;
            }
            const U2EntityRef ref = object->getEntityRef();
            if (!dbiRef2Connections.contains(ref.dbiRef)) {
                dbiRef2Connections.insert(ref.dbiRef, new DbiConnection(ref.dbiRef, os));
            }
            DbiConnection *con = dbiRef2Connections.value(ref.dbiRef);
            con->dbi->getObjectDbi()->updateObjectAccessTime(ref.entityId, os);
        }
    }
    qDeleteAll(dbiRef2Connections.values());
}

} // U2
