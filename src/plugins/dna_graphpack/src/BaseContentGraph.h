#ifndef _U2_GRAPHPACK_BASE_CONTENT_H_
#define _U2_GRAPHPACK_BASE_CONTENT_H_

#include "DNAGraphPackPlugin.h"

#include <U2View/GSequenceGraphView.h>

#include <QtGui/QAction>
#include <QtCore/QList>
#include <QtCore/QBitArray>

namespace U2 {

class AnnotatedDNAView;

class BaseContentGraphFactory : public GSequenceGraphFactory {
    Q_OBJECT
public:
    enum GType { GC, AG };
    BaseContentGraphFactory(GType t, QObject* p);
    virtual QList<GSequenceGraphData*> createGraphs(GSequenceGraphView* v);
    virtual GSequenceGraphDrawer* getDrawer(GSequenceGraphView* v);
    virtual bool isEnabled(DNASequenceObject* o) const;
private:
    QBitArray map;
};

class BaseContentGraphAlgorithm : public GSequenceGraphAlgorithm {
public:
    BaseContentGraphAlgorithm(const QBitArray& map);
    virtual ~BaseContentGraphAlgorithm() {}

    virtual void calculate(QVector<float>& res, DNASequenceObject* o, const U2Region& r, const GSequenceGraphWindowData* d);

private:
    void windowStrategyWithoutMemorize(QVector<float>& res, const QByteArray& seq, int startPos, const GSequenceGraphWindowData* d, int nSteps);
    void sequenceStrategyWithMemorize(QVector<float>& res, const QByteArray& seq, const U2Region& vr, const GSequenceGraphWindowData* d);
    int matchOnStep(const QByteArray& seq, int begin, int end);
    QBitArray map;
};

} // namespace
#endif
