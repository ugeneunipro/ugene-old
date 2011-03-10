#ifndef _U2_GRAPHPACK_CUMULATIVE_SKEW_H_
#define _U2_GRAPHPACK_CUMULATIVE_SKEW_H_

#include "DNAGraphPackPlugin.h"

#include <U2View/GSequenceGraphView.h>

#include <QtGui/QAction>
#include <QtCore/QList>
#include <QtCore/QBitArray>

namespace U2 {

class CumulativeSkewGraphFactory : public GSequenceGraphFactory {
    Q_OBJECT
public:
    enum GCumulativeSkewType { GC, AT };
    CumulativeSkewGraphFactory(GCumulativeSkewType t, QObject* p);
    virtual QList<GSequenceGraphData*> createGraphs(GSequenceGraphView* v);
    virtual GSequenceGraphDrawer* getDrawer(GSequenceGraphView* v);
    virtual bool isEnabled(DNASequenceObject* o) const;
private:
    QPair<char, char> cumPair;
};

class CumulativeSkewGraphAlgorithm : public GSequenceGraphAlgorithm {
public:
    CumulativeSkewGraphAlgorithm(const QPair<char, char>& _p);
    virtual ~CumulativeSkewGraphAlgorithm() {}

    float getValue(int begin, int end, const QByteArray& seq);
    virtual void calculate(QVector<float>& res, DNASequenceObject* o, const U2Region& r, const GSequenceGraphWindowData* d);

private:
    QPair<char, char> p;
};

} // namespace
#endif
