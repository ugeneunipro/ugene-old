#include "CumulativeSkew.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include "DNAGraphPackPlugin.h"

/* TRANSLATOR U2::CumulativeSkewGraphFactory */


/**
 *A Grigoriev  It is used to predict origin and terminus locations in
 * bacterial and archaeal genomes
 **/

namespace U2 {
static QString nameByType(CumulativeSkewGraphFactory::GCumulativeSkewType t) {
    if (t == CumulativeSkewGraphFactory::AT) {
        return CumulativeSkewGraphFactory::tr("at_cumulative_skew_not_optimal_version");
    }
    return CumulativeSkewGraphFactory::tr("gc_cumulative_skew_not_optimal_version");

}

CumulativeSkewGraphFactory::CumulativeSkewGraphFactory(GCumulativeSkewType t, QObject* p)
: GSequenceGraphFactory(nameByType(t), p)
{
    if ( t == CumulativeSkewGraphFactory::AT) {
        cumPair.first = 'A'; cumPair.second = 'T';
    } else {
        cumPair.first = 'G'; cumPair.second = 'C';
    }
}

bool CumulativeSkewGraphFactory::isEnabled(DNASequenceObject* o) const {
    DNAAlphabet* al = o->getAlphabet();
    return al->isNucleic();
}

QList<GSequenceGraphData*> CumulativeSkewGraphFactory::createGraphs(GSequenceGraphView* v) {
    Q_UNUSED(v);
    QList<GSequenceGraphData*> res;
    assert(isEnabled(v->getSequenceObject()));
    GSequenceGraphData* d = new GSequenceGraphData(getGraphName());
    d->ga = new CumulativeSkewGraphAlgorithm(cumPair);
    res.append(d);
    return res;
}

GSequenceGraphDrawer* CumulativeSkewGraphFactory::getDrawer(GSequenceGraphView* v) {
    GSequenceGraphWindowData wd(199, 199);
    return new GSequenceGraphDrawer(v, wd);
}


//////////////////////////////////////////////////////////////////////////
// CumulativeSkewGraphAlgorithm

CumulativeSkewGraphAlgorithm::CumulativeSkewGraphAlgorithm(const QPair<char, char>& _p)  :  p(_p)
{
}

float CumulativeSkewGraphAlgorithm::getValue(int begin, int end, const QByteArray& seq)
{
    int leap = end - begin;
    int first = 0;
    int second = 0;
    float resultValue = 0;
    int len;
    for (int window = 0; window < end; window += leap)    {
        first = 0;
        second = 0;
        if (window + leap > end) len = window - end; else len = leap;
        for (int i = 0; i < len; ++i)    {
            char c = seq[window + i];
            if (c == p.first) {
                first++; continue;
            }
            if (c == p.second) {
                second++;
            }
        }
        if (first + second > 0)
            resultValue += (float)(first - second)/(first + second);
    }
    return resultValue;
}

void CumulativeSkewGraphAlgorithm::calculate(QVector<float>& res, DNASequenceObject* o, const U2Region& vr, const GSequenceGraphWindowData* d) {
    assert(d!=NULL);
    int nSteps = GSequenceGraphUtils::getNumSteps(vr, d->window, d->step);
    res.reserve(nSteps);

    const QByteArray& seq = o->getSequence();

    for (int i = 0; i < nSteps; i++)    {
        int start = vr.startPos + i * d->step;
        int end = start + d->window;
        float result = getValue(start, end, seq);
        res.append(result);
    }
}

} // namespace

