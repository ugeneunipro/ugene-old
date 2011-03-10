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
    if (hasErrors() || isCanceled() || aData.size() == 0)  {
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
        stateInfo.setStateDesc(tr("Waiting for object lock released"));
        return ReportResult_CallMeAgain;
    }
    stateInfo.setStateDesc("");
    
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
