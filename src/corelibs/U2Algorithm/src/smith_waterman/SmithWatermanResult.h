#ifndef _U2_SMITH_WATERMAN_RESULT_H_
#define _U2_SMITH_WATERMAN_RESULT_H_

#include <QList>
#include <U2Core/U2Region.h>
#include <U2Core/AnnotationData.h>


namespace U2 {

struct U2ALGORITHM_EXPORT SmithWatermanResult {
    SharedAnnotationData toAnnotation(const QString& name) const {
        SharedAnnotationData data;
        data = new AnnotationData;
        data->name = name;
        data->location->regions << region;
        data->setStrand(strand);
        data->qualifiers.append(U2Qualifier("score", QString::number(score)));
        return data;
    }
    bool operator<(const SmithWatermanResult& op1) const {
        if (score < op1.score) {
            return true;
        } else {
            return false;
        }
    }
    U2Strand strand;
    bool trans;
    float score;
    U2Region region;
};

class U2ALGORITHM_EXPORT SmithWatermanResultListener {
public:
    virtual void pushResult(const SmithWatermanResult& r) {
        result.append(r);
    }
    virtual void pushResult( const QList<SmithWatermanResult>& r ) {
        result.append( r );
    }
    virtual QList<SmithWatermanResult> popResults() {
        QList<SmithWatermanResult> res = result;
        result.clear();
        return res;
    }
    virtual QList<SmithWatermanResult> getResults() const {
        return result;
    }
    virtual ~SmithWatermanResultListener() {}

private:
    QList<SmithWatermanResult> result;
};

} // namespace

#endif
