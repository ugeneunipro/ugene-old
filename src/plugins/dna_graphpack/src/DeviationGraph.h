#ifndef _U2_GRAPHPACK_DEVIATION_H_
#define _U2_GRAPHPACK_DEVIATION_H_

#include "DNAGraphPackPlugin.h"

#include <U2View/GSequenceGraphView.h>

#include <QtGui/QAction>
#include <QtCore/QList>
#include <QtCore/QBitArray>
#include <QtCore/QPair>

namespace U2 {

class DeviationGraphFactory : public GSequenceGraphFactory {
    Q_OBJECT
public:
    enum GDeviationType { GC, AT };
    DeviationGraphFactory(GDeviationType t, QObject* p);
    virtual QList<GSequenceGraphData*> createGraphs(GSequenceGraphView* v);
    virtual GSequenceGraphDrawer* getDrawer(GSequenceGraphView* v);
    virtual bool isEnabled(DNASequenceObject* o) const;
private:
    QPair<char, char> devPair;
};

class DeviationGraphAlgorithm : public GSequenceGraphAlgorithm {
public:
    DeviationGraphAlgorithm(const QPair<char, char>& _p);
    virtual ~DeviationGraphAlgorithm() {}

    virtual void calculate(QVector<float>& res, DNASequenceObject* o, const U2Region& r, const GSequenceGraphWindowData* d);

private:
    void windowStrategyWithoutMemorize(QVector<float>& res, const QByteArray& seq, int startPos, const GSequenceGraphWindowData* d, int nSteps);
    void sequenceStrategyWithMemorize(QVector<float>& res, const QByteArray& seq, const U2Region& vr, const GSequenceGraphWindowData* d);
    QPair<int, int> matchOnStep(const QByteArray& seq, int begin, int end);
    QPair<char, char> p;
};

} // namespace
#endif
