#ifndef _U2_GRAPHPACK_ENTROPY_H_
#define _U2_GRAPHPACK_ENTROPY_H_

#include "DNAGraphPackPlugin.h"

#include <U2View/GSequenceGraphView.h>

#include <QtGui/QAction>
#include <QtCore/QList>
#include <QtCore/QBitArray>

namespace U2 {

class AnnotatedDNAView;

class EntropyGraphFactory : public GSequenceGraphFactory {
    Q_OBJECT
public:
    EntropyGraphFactory(QObject* p);
    virtual QList<GSequenceGraphData*> createGraphs(GSequenceGraphView* v);
    virtual GSequenceGraphDrawer* getDrawer(GSequenceGraphView* v);
    virtual bool isEnabled(DNASequenceObject* o) const;
};

class EntropyGraphAlgorithm : public GSequenceGraphAlgorithm {
public:
    EntropyGraphAlgorithm();
    virtual ~EntropyGraphAlgorithm() {}

    virtual void calculate(QVector<float>& res, DNASequenceObject* o, const U2Region& r, const GSequenceGraphWindowData* d);
};

} // namespace
#endif
