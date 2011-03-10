#ifndef _U2_SMITH_WATERMAN_REPORT_CALLBACK_H_
#define _U2_SMITH_WATERMAN_REPORT_CALLBACK_H_

#include <U2Algorithm/SmithWatermanResult.h>

#include <QObject>
#include <U2Core/AnnotationTableObject.h>


namespace U2 {

class U2ALGORITHM_EXPORT SmithWatermanReportCallback {
public:
    virtual QString report(const QList<SmithWatermanResult>& ) { return QString(); }
    virtual ~SmithWatermanReportCallback() {}
};

class U2ALGORITHM_EXPORT SmithWatermanReportCallbackImpl:  
                                        public QObject,
                                        public SmithWatermanReportCallback {
    Q_OBJECT
public:
    SmithWatermanReportCallbackImpl(AnnotationTableObject* _aobj,
                                    const QString& _annotationName,
                                    const QString& _annotationGroup, 
                                    QObject* pOwn = 0);

    virtual QString report(const QList<SmithWatermanResult>& result);    
    const QList<SharedAnnotationData>& getAnotations() const {return anns;}
private:    

    QString annotationName;
    QString annotationGroup;
    QPointer<AnnotationTableObject> aObj;
    QList<SharedAnnotationData> anns;
    bool autoReport;
};

} // namespace

#endif
