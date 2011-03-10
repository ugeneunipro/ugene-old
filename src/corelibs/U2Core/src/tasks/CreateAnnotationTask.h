#ifndef _U2_CREATE_ANNOTATION_TASK_H_
#define _U2_CREATE_ANNOTATION_TASK_H_

#include <U2Core/Task.h>

#include <U2Core/AnnotationTableObject.h>

namespace U2 {

class AnnotationTableObject;
class AnnotationData;

class U2CORE_EXPORT CreateAnnotationsTask : public Task {
    Q_OBJECT
public:
    // Adds annotations to the object. Waits object to be unlocked if needed
    // Works only in a context of active project
    CreateAnnotationsTask(AnnotationTableObject* o, const QString& group, SharedAnnotationData data);
    CreateAnnotationsTask(AnnotationTableObject* o, const QString& group, QList<SharedAnnotationData> data);
    CreateAnnotationsTask(const GObjectReference& ref, const QString& group, QList<SharedAnnotationData> data);

    ReportResult report();
    const QList<Annotation*>& getAnnotations() const {return annotations;}

private:
    GObjectReference            aRef;
    QPointer<AnnotationTableObject>  aobj;
    QString                     groupName;
    QList<SharedAnnotationData> aData;
    QList<Annotation*>          annotations;
    int                         pos;
};

}//namespace

#endif
