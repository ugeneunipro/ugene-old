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
#include <U2Core/Folder.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SafePoints.h>

#include "ImportDocumentToDatabaseTask.h"
#include "ImportObjectToDatabaseTask.h"

namespace U2 {

ImportDocumentToDatabaseTask::ImportDocumentToDatabaseTask(Document* document, const U2DbiRef& dstDbiRef, const QString& dstFolder, const ImportToDatabaseOptions& options) :
    Task(tr("Import document %1 to the database").arg(NULL != document ? document->getName() : ""), TaskFlag_NoRun),
    document(document),
    dstDbiRef(dstDbiRef),
    dstFolder(dstFolder),
    options(options)
{
    CHECK_EXT(NULL != document, setError(tr("Invalid document to import")), );
    CHECK_EXT(dstDbiRef.isValid(), setError(tr("Invalid database reference")), );

    setMaxParallelSubtasks(MAX_PARALLEL_SUBTASKS_AUTO);

    if (options.createSubfolderForEachDocument) {
        this->dstFolder = Folder::createPath(dstFolder, document->getName());
    }
}

void ImportDocumentToDatabaseTask::prepare() {
    foreach (GObject* object, document->getObjects()) {
        addSubTask(new ImportObjectToDatabaseTask(object, dstDbiRef, dstFolder));
    }
}

Task::ReportResult ImportDocumentToDatabaseTask::report() {
    CHECK_EXT(!document.isNull(), setError(tr("Source document was removed, can't set object relations")), ReportResult_Finished);

    QStringList errors;
    propagateObjectsRelations(errors);
    if (!errors.isEmpty()) {
        setError(errors.join("\n"));
    }

    return ReportResult_Finished;
}

Document *ImportDocumentToDatabaseTask::getSourceDocument() const {
    return document.data();
}

QStringList ImportDocumentToDatabaseTask::getImportedObjectNames() const {
    QStringList result;
    const QList<GObject*> objects = getImportedObjects();
    foreach (GObject* object, objects) {
        result << object->getGObjectName();
    }

    return result;
}

QStringList ImportDocumentToDatabaseTask::getSkippedObjectNames() const {
    QStringList result;
    foreach (Task* subtask, getSubtasks()) {
        if (subtask->isCanceled() || subtask->hasError()) {
            ImportObjectToDatabaseTask* importObjectTask = qobject_cast<ImportObjectToDatabaseTask*>(subtask);
            if (NULL != importObjectTask) {
                GObject* object = importObjectTask->getSourceObject();
                if (NULL != object) {
                    result << object->getGObjectName();
                }
            }
        }
    }

    return result;
}

QList<GObject *> ImportDocumentToDatabaseTask::getImportedObjects() const {
    QList<GObject*> objects;
    foreach (Task* subtask, getSubtasks()) {
        if (!subtask->isCanceled() && !subtask->hasError()) {
            ImportObjectToDatabaseTask* importObjectTask = qobject_cast<ImportObjectToDatabaseTask*>(subtask);
            if (NULL != importObjectTask) {
                GObject* object = importObjectTask->getDestinationObject();
                if (NULL != object) {
                    objects << object;
                }
            }
        }
    }

    return objects;
}

void ImportDocumentToDatabaseTask::propagateObjectsRelations(QStringList& errors) const {
    const QList<GObject*> srcObjects = document->getObjects();
    const QList<GObject*> dstObjects = getImportedObjects();
    const QString srcDocUrl = document->getURLString();

    foreach(GObject* srcObject, srcObjects) {
        GObject* dstObject = getAppropriateObject(dstObjects, srcObject);
        if (NULL == dstObject) {
            errors << tr("Can't find the appropriate imported object: %1; relation won't be set").arg(srcObject->getGObjectName());
            continue;
        }

        QList<GObjectRelation> relations = srcObject->getObjectRelations();
        foreach (const GObjectRelation& relation, relations) {
            if (srcDocUrl != relation.getDocURL()) {
                // skip this relation: target object is not imported to the database
                continue;
            }

            GObject* srcRelationTargetObject = document->findGObjectByName(relation.ref.objName);
            if (NULL == srcRelationTargetObject) {
                errors << tr("Can't set object relation: target object is not found in the source document (%1)").arg(relation.ref.objName);
                continue;
            }

            GObjectReference dstReference(U2DbiUtils::ref2Url(dstDbiRef), srcRelationTargetObject->getGObjectName(), srcRelationTargetObject->getGObjectType());
            GObjectRelation dstRelation(dstReference, relation.role);
            dstObject->addObjectRelation(dstRelation);
        }
    }
}

GObject *ImportDocumentToDatabaseTask::getAppropriateObject(const QList<GObject *> objects, const GObject *pattern) {
    CHECK(NULL != pattern, NULL);

    foreach (GObject* object, objects) {
        if (object->getGObjectName() == pattern->getGObjectName() &&
            object->getGObjectType() == pattern->getGObjectType()) {
            return object;
        }
    }

    return NULL;
}

}   // namespace U2
