#ifndef _U2_GRAPHPACK_KARLIN_H_
#define _U2_GRAPHPACK_KARLIN_H_

#include "DNAGraphPackPlugin.h"

#include <U2View/GSequenceGraphView.h>

#include <QtGui/QAction>
#include <QtCore/QList>
#include <QtCore/QBitArray>

namespace U2 {

class KarlinGraphFactory : public GSequenceGraphFactory {
    Q_OBJECT
public:
    KarlinGraphFactory(QObject* p);
    virtual QList<GSequenceGraphData*> createGraphs(GSequenceGraphView* v);
    virtual GSequenceGraphDrawer* getDrawer(GSequenceGraphView* v);
    virtual bool isEnabled(DNASequenceObject* o) const;
};

class KarlinGraphAlgorithm : public GSequenceGraphAlgorithm {
public:
    KarlinGraphAlgorithm();
    virtual ~KarlinGraphAlgorithm();

    virtual void calculate(QVector<float>& res, DNASequenceObject* o, const U2Region& r, const GSequenceGraphWindowData* d);
private:
    float getValue (int start, int end, const QByteArray& s);
    void calculateRelativeAbundance (const char* seq, int length, float* results);


    float* global_relative_abundance_values;
    QByteArray mapTrans;
};

} // namespace
#endif
