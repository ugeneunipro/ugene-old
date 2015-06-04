/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "CreateAnnotationTask.h"

#include "LoadDocumentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2FeatureUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

CreateAnnotationsTask::CreateAnnotationsTask(AnnotationTableObject *ao, const QList<SharedAnnotationData> &data,  const QString &g)
    : Task(tr("Create annotations"), TaskFlags_FOSE_COSC), aobj(ao), groupName(g), aData(data)
{
    AnnotationTableObject *parentObject = getGObject();
    CHECK_EXT(NULL != parentObject, setError(tr("Annotation table has been removed unexpectedly")),);

    aRef.objName = parentObject->getGObjectName();
    tpm = Progress_Manual;
}

CreateAnnotationsTask::CreateAnnotationsTask(const GObjectReference &r, const QList<SharedAnnotationData> &data, const QString &g)
    : Task(tr("Create annotations"), TaskFlags_FOSE_COSC), aRef(r), groupName(g), aData(QList<SharedAnnotationData>() << data)
{
    GObject *ao = (GObjectUtils::selectObjectByReference(aRef, UOF_LoadedAndUnloaded));
    if (NULL != ao && ao->isUnloaded()) {
        addSubTask(new LoadUnloadedDocumentTask(ao->getDocument()));
    }
    tpm = Progress_Manual;
}

void CreateAnnotationsTask::run() {
    AnnotationTableObject *parentObject = getGObject();
    CHECK_EXT(NULL != parentObject, setError(tr("Annotation table has been removed unexpectedly")),);

    DbiOperationsBlock opBlock(parentObject->getEntityRef().dbiRef, stateInfo);
    Q_UNUSED(opBlock);
    CHECK_OP(stateInfo, );

    const U2DataId rootFeatureId = parentObject->getRootFeatureId();
    const U2DbiRef dbiRef = parentObject->getEntityRef().dbiRef;

    if (groupName.isEmpty()) {
        foreach (const SharedAnnotationData &a, aData) {
            AnnotationGroup *group = parentObject->getRootGroup()->getSubgroup(a->name, true);
            const U2DataId groupId = group->id;
            const U2Feature feature = U2FeatureUtils::exportAnnotationDataToFeatures(a, rootFeatureId, groupId, dbiRef, stateInfo);
            CHECK_OP(stateInfo, );
            group2Annotations[group] << new Annotation(feature.id, a, group, parentObject);
        }
    } else {
        AnnotationGroup *group = parentObject->getRootGroup()->getSubgroup(groupName, true);
        const U2DataId groupId = group->id;
        foreach (const SharedAnnotationData &a, aData) {
            const U2Feature feature = U2FeatureUtils::exportAnnotationDataToFeatures(a, rootFeatureId, groupId, dbiRef, stateInfo);
            CHECK_OP(stateInfo, );
            group2Annotations[group] << new Annotation(feature.id, a, group, parentObject);
        }
    }
}

Task::ReportResult CreateAnnotationsTask::report() {
    if (hasError() || isCanceled() || group2Annotations.isEmpty()) {
        return ReportResult_Finished;
    }
    AnnotationTableObject *ao = getGObject();
    if (NULL == ao) {
        setError(tr("Annotation object '%1' not found in active project: %2").arg(aRef.objName).arg(aRef.docUrl));
        return ReportResult_Finished;
    }

    foreach (AnnotationGroup *group, group2Annotations.keys()) {
        group->addShallowAnnotations(group2Annotations[group], true);
        resultAnnotations.append(group2Annotations[group]);
    }

    return ReportResult_Finished;
}

AnnotationTableObject * CreateAnnotationsTask::getGObject() const {
    AnnotationTableObject *result = NULL;
    if (aRef.isValid()) {
        SAFE_POINT(aobj.isNull(), "Unexpected annotation table object content!", NULL);
        result = qobject_cast<AnnotationTableObject *>(GObjectUtils::selectObjectByReference(aRef, UOF_LoadedOnly));
    } else {
        result = aobj.data();
    }
    return result;
}

int CreateAnnotationsTask::getAnnotationCount() const {
    return aData.size();
}

QList<Annotation *> CreateAnnotationsTask::getResultAnnotations() const {
    return resultAnnotations;
}

} // namespace
