#ifndef _U2_SEC_STRUCT_PREDICT_UTILS_H_
#define _U2_SEC_STRUCT_PREDICT_UTILS_H_

#include <U2View/ADVUtils.h>
#include <U2Core/AnnotationData.h>

namespace U2 {

class AnnotatedDNAView;

class SecStructPredictViewAction: public ADVGlobalAction {
    Q_OBJECT
public:
    SecStructPredictViewAction(AnnotatedDNAView* v);
    ~SecStructPredictViewAction();
    static ADVGlobalAction* createAction(AnnotatedDNAView* view);    
protected slots:
    void sl_execute();
};

class U2VIEW_EXPORT SecStructPredictUtils {
public:
    static QString getStructNameForCharTag(char tag);
    static QList<SharedAnnotationData> saveAlgorithmResultsAsAnnotations(const QByteArray& predicted, const QString& annotationName);
};



} //namespace

#endif // _U2_SEC_STRUCT_PREDICT_UTILS_H_
