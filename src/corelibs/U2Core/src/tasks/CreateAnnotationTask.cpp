/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

namespace U2 {


CreateAnnotationsTask::CreateAnnotationsTask(AnnotationTableObject* _ao, const QString& _g, QSharedDataPointer<AnnotationData> _data)
: Task(tr("Create annotations"), TaskFlag_NoRun), aobj(_ao), groupName(_g), pos(0)
{
    aData << _data;
    aRef.objName = _ao->getGObjectName();
}

CreateAnnotationsTask::CreateAnnotationsTask(AnnotationTableObject* _ao, const QString& _g, QList<SharedAnnotationData> _data) 
: Task(tr("Create annotations"), TaskFlag_NoRun), aobj(_ao), groupName(_g), pos(0)
{
    aData = _data;
    aRef.objName = _ao->getGObjectName();
}

CreateAnnotationsTask::CreateAnnotationsTask(const GObjectReference& r, const QString& _g, QList<SharedAnnotationData> _data) 
: Task(tr("Create annotations"), TaskFlag_NoRun), aRef(r), groupName(_g), pos(0)
{
    aData << _data;
    GObject* ao = (GObjectUtils::selectObjectByReference(aRef, UOF_LoadedAndUnloaded));
    if (ao!=NULL && ao->isUnloaded()) {
        addSubTask(new LoadUnloadedDocumentTask(ao->getDocument()));
    }
}

Task::ReportResult CreateAnnotationsTask::report() {
    GTIMER(c1,t1,"CreateAnnotationsTask::report");
    if (hasError() || isCanceled() || aData.size() == 0)  {
        return ReportResult_Finished;
    }
    AnnotationTableObject* ao = NULL;
    if (aRef.isValid()) {
        assert(aobj.isNull());
        ao = qobject_cast<AnnotationTableObject*>(GObjectUtils::selectObjectByReference(aRef, UOF_LoadedOnly));
    } else {
        ao = aobj.data();
    }
    if (ao == NULL) {
        stateInfo.setError(tr("Annotation object '%1' not found in active project: %2").arg(aRef.objName).arg(aRef.docUrl));
        return ReportResult_Finished;
    }

    if (ao->isStateLocked()) {
        stateInfo.setDescription(tr("Waiting for object lock released"));
        return ReportResult_CallMeAgain;
    }
    stateInfo.setDescription("");
    
    int brk = qMin(pos+10000, aData.size());
    for (int i = pos; i < brk; i++)
    {
        annotations.append(new Annotation(aData.at(i)));
    }
    GTIMER(c2,t2,"CreateAnnotationsTask::report [addAnnotations]");
    ao->addAnnotations(annotations.mid(pos), groupName);
    tpm = Progress_Manual;
    stateInfo.progress = 100*brk/aData.size();
    if (brk != aData.size()) {
        pos = brk;
        return ReportResult_CallMeAgain;
    }

    return ReportResult_Finished;
}


} // namespace
