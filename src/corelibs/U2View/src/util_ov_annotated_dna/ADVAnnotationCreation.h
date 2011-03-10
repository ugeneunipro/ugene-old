#ifndef _U2_ADVANNOTATION_CREATION_H_
#define _U2_ADVANNOTATION_CREATION_H_

#include <U2Core/Task.h>
#include <U2Core/AnnotationTableObject.h>

#include <QtGui/QAction>
#include <QtCore/QPointer>

namespace U2 {

class CreateAnnotationsTask;
class AnnotatedDNAView;
class DNASequenceObject;

class U2VIEW_EXPORT ADVAnnotationCreation : public QObject {
    Q_OBJECT
public:
    ADVAnnotationCreation(AnnotatedDNAView* v);

    QAction* getCreateAnnotationAction() const {return createAction;}

private slots:
    void sl_createAnnotation();

private:
    AnnotatedDNAView* ctx;
    QAction* createAction;
};


class U2VIEW_EXPORT  ADVCreateAnnotationsTask : public Task {
    Q_OBJECT
public:
    ADVCreateAnnotationsTask(AnnotatedDNAView* _ctx, const GObjectReference& aobjRef, 
                            const QString& group, const QList<SharedAnnotationData>& data);
        
    ReportResult report();

private:
    QPointer<AnnotatedDNAView> ctx;
    CreateAnnotationsTask* t;
};


} // namespace



#endif
