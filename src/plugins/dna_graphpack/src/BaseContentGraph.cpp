/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "BaseContentGraph.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include "DNAGraphPackPlugin.h"
#include <U2Algorithm/RollingArray.h>


/* TRANSLATOR U2::BaseContentGraphFactory */

namespace U2 {

static QString nameByType(BaseContentGraphFactory::GType t) {
    if (t == BaseContentGraphFactory::AG) {
        return BaseContentGraphFactory::tr("ag_content_graph");
    }
    return BaseContentGraphFactory::tr("gc_content_graph");

}

BaseContentGraphFactory::BaseContentGraphFactory(GType t, QObject* p)
: GSequenceGraphFactory(nameByType(t), p) , map(256, false)
{
    if ( t == BaseContentGraphFactory::AG) {
        map['A']=map['G']=true;
    } else {
        map['G']=map['C']=true;
    }
}

bool BaseContentGraphFactory::isEnabled(const U2SequenceObject* o) const {
    const DNAAlphabet* al = o->getAlphabet();
    return al->isNucleic();
}

QList<GSequenceGraphData*> BaseContentGraphFactory::createGraphs(GSequenceGraphView* v) {
    Q_UNUSED(v);
    QList<GSequenceGraphData*> res;
    assert(isEnabled(v->getSequenceObject()));
    GSequenceGraphData* d = new GSequenceGraphData(getGraphName());
    d->ga = new BaseContentGraphAlgorithm(map);
    res.append(d);
    return res;
}


//////////////////////////////////////////////////////////////////////////
// BaseContentGraphAlgorithm

BaseContentGraphAlgorithm::BaseContentGraphAlgorithm(const QBitArray& _map)  :  map(_map)
{
}

void BaseContentGraphAlgorithm::windowStrategyWithoutMemorize(QVector<float>& res, const QByteArray& seq, int startPos, const GSequenceGraphWindowData* d, int nSteps)
{
    for (int i = 0; i < nSteps; i++) {
        int start = startPos + i * d->step;
        int end = start + d->window;
        int base_count = 0;
        for (int x = start; x < end; x++) {
            char c = seq[x];
            if (map[(uchar)c]) {
                base_count++;
            }
        }
        res.append((base_count / (float)(d->window))*100);
    }
}

int BaseContentGraphAlgorithm::matchOnStep(const QByteArray& seq, int begin, int end)
{
    int res = 0;
    for (int j = begin; j < end; ++j) {
        char c = seq[j];
        if (map[(uchar)c]) {
            ++res;
        }
    }
    return res;
}
void BaseContentGraphAlgorithm::sequenceStrategyWithMemorize(QVector<float>& res, const QByteArray& seq, const U2Region& vr, const GSequenceGraphWindowData* d)
{
    int rsize = d->window / d->step;
    RollingArray<int> ra(rsize);
    int endPos = vr.endPos();
    int globalCount = 0;
    int nextI;
    int firstValue = vr.startPos + d->window - d->step;
    for (int i = vr.startPos; i < endPos; i = nextI) {
        nextI = i + d->step;
        int result = matchOnStep(seq, i, nextI);
        globalCount += result;
        ra.push_back_pop_front(result);
        if (i >= firstValue)    {
            int v = ra.get(0);
            res.append(globalCount / (float)(d->window)*100);
            globalCount -= v;
        }
    }
}

void BaseContentGraphAlgorithm::calculate(QVector<float>& res, U2SequenceObject* o, const U2Region& vr, const GSequenceGraphWindowData* d) {
    assert(d!=NULL);
    int nSteps = GSequenceGraphUtils::getNumSteps(vr, d->window, d->step);
    res.reserve(nSteps);
    const QByteArray& seq = getSequenceData(o);
    int startPos = vr.startPos;


//    if (d->window % d->step != 0)
        windowStrategyWithoutMemorize(res, seq, startPos, d, nSteps);
//    else
//        sequenceStrategyWithMemorize(res, seq, vr, d);
}

} // namespace

