#include "SmithWatermanReportCallback.h"

namespace U2 {

SmithWatermanReportCallbackImpl::SmithWatermanReportCallbackImpl(
                                AnnotationTableObject* _aobj,
                                const QString& _annotationName,
                                const QString& _annotationGroup,
                                QObject* pOwn): 
    QObject(pOwn), 
    annotationName(_annotationName), annotationGroup(_annotationGroup), 
    aObj(_aobj), autoReport(_aobj != NULL)
{
}

QString SmithWatermanReportCallbackImpl::report(const QList<SmithWatermanResult>& results) {    
    if (autoReport && aObj.isNull()) {
        return tr("Annotation object not found.");
    }

    if (autoReport && aObj->isStateLocked()) {
        return tr("Annotation table is read-only");
    }

    foreach (const SmithWatermanResult& res , results) {
        anns.append(res.toAnnotation(annotationName));
    }

    if (autoReport) {
        QList<Annotation*> annotations;
        foreach(const SharedAnnotationData& ad, anns) {
            annotations.append(new Annotation(ad));
        }
        aObj->addAnnotations(annotations, annotationGroup);
    }
    return QString();
}

} // namespace
