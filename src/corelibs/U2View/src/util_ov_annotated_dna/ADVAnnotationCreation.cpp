#include "ADVAnnotationCreation.h"

#include "AnnotatedDNAView.h"
#include "ADVSequenceObjectContext.h"
#include "AnnotationsTreeView.h"

#include <U2Gui/CreateAnnotationDialog.h>
#include <U2Gui/CreateAnnotationWidgetController.h>//for CreateAnnotationModel
#include <U2Core/CreateAnnotationTask.h>

#include <U2Core/GObjectUtils.h>
#include <U2Core/AnnotationSelection.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/Task.h>

#include <U2Core/LoadDocumentTask.h>

#include <QtCore/QFileInfo>

namespace U2 {

ADVCreateAnnotationsTask::ADVCreateAnnotationsTask(AnnotatedDNAView* _ctx, const GObjectReference& aobjRef, 
                                                   const QString& group, const QList<SharedAnnotationData>& data)  
: Task(tr("Create annotations task"), TaskFlags_NR_FOSCOE), ctx(_ctx)
{
    LoadUnloadedDocumentTask::addLoadingSubtask(this, LoadDocumentTaskConfig(true, aobjRef, new LDTObjectFactory(this)));
    t  = new CreateAnnotationsTask(aobjRef, group, data);
    addSubTask(t);
}

Task::ReportResult ADVCreateAnnotationsTask::report() {
    if (!hasErrors() && !ctx.isNull()) {
        Annotation* a = t->getAnnotations().first();
        assert(a!=NULL);

        AnnotationTableObject* ao = a->getGObject();
        if (!ctx->getAnnotationObjects().contains(ao)) { 
            //for documents loaded during annotation creation object is added here
            QString err = ctx->addObject(ao);
            if (!err.isEmpty()) {
                setError(err);
            }
        }
        if (!hasErrors()) {
            ctx->getAnnotationsSelection()->clear();
            ctx->getAnnotationsSelection()->addToSelection(a);
        }
    }
    return ReportResult_Finished;
}

//////////////////////////////////////////////////////////////////////////
/// ADVAnnotationCreation

ADVAnnotationCreation::ADVAnnotationCreation(AnnotatedDNAView* c) : QObject(c) {
    ctx = c;
    createAction = new QAction(QIcon(":core/images/create_annotation_icon.png"), tr("New annotation..."), this);
    createAction->setObjectName("create_annotation_action");
    createAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    createAction->setShortcutContext(Qt::WindowShortcut);
    connect(createAction, SIGNAL(triggered()), SLOT(sl_createAnnotation()));
}

void ADVAnnotationCreation::sl_createAnnotation() {
    ADVSequenceObjectContext* seqCtx = ctx->getSequenceInFocus();
    assert(seqCtx!=NULL);
    CreateAnnotationModel m;
    m.useUnloadedObjects = true;
    m.sequenceObjectRef = GObjectReference(seqCtx->getSequenceObject());
    m.sequenceLen = seqCtx->getSequenceObject()->getSequenceLen();
    if (!seqCtx->getSequenceSelection()->isEmpty()) {
        m.data->location->regions << seqCtx->getSequenceSelection()->getSelectedRegions();
    }
    
    //setup default object and group if possible from AnnotationsTreeView
    AnnotationsTreeView* tv = ctx->getAnnotationsView();
    AVItem* ai = tv->currentItem();
    if (ai!=NULL && !ai->isReadonly()) {
        AnnotationTableObject* aobj = ai->getAnnotationTableObject();
        if (seqCtx->getAnnotationGObjects().contains(aobj)) {
            m.annotationObjectRef = aobj;
            AnnotationGroup* ag = ai->getAnnotationGroup();
            if (ag!=aobj->getRootGroup()) {
                m.groupName = ag->getGroupPath();
            }
        }
    }

    CreateAnnotationDialog d(ctx->getWidget(), m);
    int rc = d.exec();
    if (rc != QDialog::Accepted) {
        return;
    }

    QList<SharedAnnotationData> data;
    data << m.data;
    ADVCreateAnnotationsTask* t = new ADVCreateAnnotationsTask(ctx, m.annotationObjectRef, m.groupName, data);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

} // namespace
