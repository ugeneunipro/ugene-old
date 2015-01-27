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
#include <U2Core/Timer.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/U2FeatureUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

CreateAnnotationsTask::CreateAnnotationsTask(AnnotationTableObject *ao, const AnnotationData &data, const QString &g)
    : Task(tr("Create annotations"), TaskFlags_FOSE_COSC), aobj(ao), groupName(g), pos(0)
{
    aData << data;
    AnnotationTableObject *parentObject = getGObject();
    CHECK_EXT(NULL != parentObject, setError(tr("Annotation table has been removed unexpectedly")),);

    aRef.objName = parentObject->getGObjectName();
    tpm = Progress_Manual;
}

CreateAnnotationsTask::CreateAnnotationsTask(AnnotationTableObject *ao, const QList<AnnotationData> &data,  const QString &g)
    : Task(tr("Create annotations"), TaskFlags_FOSE_COSC), aobj(ao), groupName(g), pos(0)
{
    aData = data;
    AnnotationTableObject *parentObject = getGObject();
    CHECK_EXT(NULL != parentObject, setError(tr("Annotation table has been removed unexpectedly")),);

    aRef.objName = parentObject->getGObjectName();
    tpm = Progress_Manual;
}

CreateAnnotationsTask::CreateAnnotationsTask(const GObjectReference &r, const QList<AnnotationData> &data, const QString &g)
    : Task(tr("Create annotations"), TaskFlags_FOSE_COSC), aRef(r), groupName(g), pos(0)
{
    aData << data;
    GObject *ao = (GObjectUtils::selectObjectByReference(aRef, UOF_LoadedAndUnloaded));
    if (NULL != ao && ao->isUnloaded()) {
        addSubTask(new LoadUnloadedDocumentTask(ao->getDocument()));
    }
    tpm = Progress_Manual;
}

void CreateAnnotationsTask::run() {
    AnnotationTableObject *parentObject = getGObject();
    CHECK_EXT(NULL != parentObject, setError(tr("Annotation table has been removed unexpectedly")),);

    const U2DataId rootFeatureId = parentObject->getRootFeatureId();
    const U2DbiRef dbiRef = parentObject->getEntityRef().dbiRef;

    if (groupName.isEmpty()) {
        foreach (const AnnotationData &a, aData) {
            CHECK_OP(stateInfo,);
            const U2DataId groupId = AnnotationGroup(rootFeatureId, parentObject).getSubgroup(a.name, true).id;
            U2Feature feature = U2FeatureUtils::exportAnnotationDataToFeatures(a, rootFeatureId, groupId, dbiRef, stateInfo);
            featuresByGroups[a.name] << feature;
        }
    } else {
        const U2DataId groupId = AnnotationGroup(rootFeatureId, parentObject).getSubgroup(groupName, true).id;
        foreach (const AnnotationData &a, aData) {
            CHECK_OP(stateInfo,);
            featuresByGroups[groupName] << U2FeatureUtils::exportAnnotationDataToFeatures(a, rootFeatureId, groupId, dbiRef, stateInfo);
        }
    }
}

Task::ReportResult CreateAnnotationsTask::report() {
    GTIMER(c1, t1, "CreateAnnotationsTask::report");
    if (hasError() || isCanceled() || featuresByGroups.isEmpty()) {
        return ReportResult_Finished;
    }
    AnnotationTableObject *ao = getGObject();
    if (NULL == ao) {
        setError(tr("Annotation object '%1' not found in active project: %2").arg(aRef.objName).arg(aRef.docUrl));
        return ReportResult_Finished;
    }

    if (ao->isStateLocked()) {
        stateInfo.setDescription(tr("Waiting for object lock released"));
        return ReportResult_CallMeAgain;
    }
    stateInfo.setDescription(QString());

    GTIMER(c2, t2, "CreateAnnotationsTask::report [addAnnotations]");

    foreach (const QString &groupName, featuresByGroups.keys()) {
        AnnotationGroup(ao->getRootFeatureId(), ao).getSubgroup(groupName, false).addFeatures(featuresByGroups[groupName]);
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

} // namespace
