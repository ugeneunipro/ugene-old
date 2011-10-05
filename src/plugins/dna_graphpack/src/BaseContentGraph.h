/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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
    virtual bool isEnabled(U2SequenceObject* o) const;
private:
    QBitArray map;
};

class BaseContentGraphAlgorithm : public GSequenceGraphAlgorithm {
public:
    BaseContentGraphAlgorithm(const QBitArray& map);
    virtual ~BaseContentGraphAlgorithm() {}

    virtual void calculate(QVector<float>& res, U2SequenceObject* o, const U2Region& r, const GSequenceGraphWindowData* d);

private:
    void windowStrategyWithoutMemorize(QVector<float>& res, const QByteArray& seq, int startPos, const GSequenceGraphWindowData* d, int nSteps);
    void sequenceStrategyWithMemorize(QVector<float>& res, const QByteArray& seq, const U2Region& vr, const GSequenceGraphWindowData* d);
    int matchOnStep(const QByteArray& seq, int begin, int end);
    QBitArray map;
};

} // namespace
#endif
