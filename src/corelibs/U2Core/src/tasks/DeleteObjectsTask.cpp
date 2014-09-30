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
#include <U2Core/GObject.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SafePoints.h>

#include "DeleteObjectsTask.h"

namespace U2 {

namespace {
    class ConnectionHelper {
    public:
        ~ConnectionHelper() {
            qDeleteAll(opBlocks);
            qDeleteAll(connections);
        }

        DbiConnection * getConnection(const U2DbiRef &dbiRef, U2OpStatus &os) {
            foreach (DbiConnection *con, connections) {
                if (con->dbi->getDbiRef() == dbiRef) {
                    return con;
                }
            }

            DbiConnection *con = new DbiConnection(dbiRef, os);
            connections.append(con);
            CHECK_OP(os, NULL);

            opBlocks.append(new DbiOperationsBlock(dbiRef, os));
            CHECK_OP(os, NULL);

            return con;
        }

        QList<DbiConnection*> connections;
        QList<DbiOperationsBlock*> opBlocks;
    };
}

//////////////////////////////////////////////////////////////////////////
// DeleteObjectsTask
//////////////////////////////////////////////////////////////////////////

DeleteObjectsTask::DeleteObjectsTask(const QList<GObject *> &objs)
    : Task(tr("Delete objects"), TaskFlag_None)
{
    tpm = Progress_Manual;
    foreach (GObject *obj, objs) {
        CHECK_EXT(NULL != obj, stateInfo.setError("Invalid object detected!"), );
        objectRefs.append(obj->getEntityRef());
    }
}

namespace {
    class ProgressUpdater {
        U2OpStatus &os;
        const int size;
        const int percentStep;
        int ticksCount;
        int currentStep;
    public:
        ProgressUpdater(U2OpStatus &os, int size)
            : os(os), size(size), percentStep(size / 100), ticksCount(0), currentStep(0)
        {
            os.setProgress(0);
        }
        inline void tick() {
            SAFE_POINT_EXT(size > 0, os.setError("Division by zero"), );
            ticksCount++;
            currentStep++;
            if (currentStep >= percentStep) {
                currentStep = 0;
                os.setProgress(ticksCount*100 / size);
            }
        }
    };
}

void DeleteObjectsTask::run() {
    ConnectionHelper helper;

    ProgressUpdater progressUpdater(stateInfo, objectRefs.size());
    foreach (const U2EntityRef &objRef, objectRefs) {
        progressUpdater.tick();
        if (!objRef.isValid()) {
            continue;
        }

        const U2DbiRef dbiRef = objRef.dbiRef;
        if (!dbiRef.isValid()) {
            continue;
        }

        DbiConnection *con = helper.getConnection(dbiRef, stateInfo);
        CHECK_OP(stateInfo, );

        if (con->dbi->getFeatures().contains(U2DbiFeature_RemoveObjects)) {
            con->dbi->getObjectDbi()->removeObject(objRef.entityId, stateInfo);
            CHECK_OP(stateInfo, );
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// DeleteFoldersTask
//////////////////////////////////////////////////////////////////////////

DeleteFoldersTask::DeleteFoldersTask(const QList<Folder> &folders)
    : Task(tr("Delete objects"), TaskFlag_None)
{
    tpm = Progress_Manual;
    foreach (const Folder &folder, folders) {
        Document *doc = folder.getDocument();
        CHECK_EXT(NULL != doc, stateInfo.setError("Invalid document detected!"), );
        const U2DbiRef dbiRef = doc->getDbiRef();
        CHECK_EXT(dbiRef.isValid(), stateInfo.setError("Invalid DBI reference detected!"), );
        dbi2Path.insert(doc->getDbiRef(), folder.getFolderPath());
    }
}

void DeleteFoldersTask::run() {
    ConnectionHelper helper;

    ProgressUpdater progressUpdater(stateInfo, dbi2Path.size());
    foreach (const U2DbiRef &dbiRef, dbi2Path.keys()) {
        foreach (const QString &path, dbi2Path.values(dbiRef)) {
            DbiConnection *con = helper.getConnection(dbiRef, stateInfo);
            CHECK_OP(stateInfo, );

            con->dbi->getObjectDbi()->removeFolder(path, stateInfo);
        }
        progressUpdater.tick();
    }
}

} // namespace U2
