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

#ifndef _EXPERTDISCOVERY_GRAPHS_HEADER_
#define _EXPERTDISCOVERY_GRAPHS_HEADER_


#include "ExpertDiscoveryData.h"
#include <U2View/GraphMenu.h>
#include <U2View/GSequenceGraphView.h>

namespace U2{

/**
 * Algorithm used to calculate data of a graph
 */
class ExpertDiscoveryScoreGraphAlgorithm : public GSequenceGraphAlgorithm
{
public:
    ExpertDiscoveryScoreGraphAlgorithm(ExpertDiscoveryData& data, int _edSeqNumber, SequenceType sType);
    virtual ~ExpertDiscoveryScoreGraphAlgorithm();

    virtual void calculate(
       QVector<float>&,
       U2SequenceObject*,
       const U2Region&,
       const GSequenceGraphWindowData*);
private:
    int edSeqNumber;
    ExpertDiscoveryData& edData;
    SequenceType edSeqType;
};


/**
 * Factory used to draw a DNA ExpertDiscovery score graph
 */
class ExpertDiscoveryScoreGraphFactory : public GSequenceGraphFactory
{
    Q_OBJECT
public:
    ExpertDiscoveryScoreGraphFactory(QObject*, ExpertDiscoveryData& data, int _edSeqNumber, SequenceType sType);
    virtual QList<GSequenceGraphData*> createGraphs(GSequenceGraphView*);
    virtual GSequenceGraphDrawer* getDrawer(GSequenceGraphView*);
    virtual bool isEnabled(U2SequenceObject*) const;

private:
    int edSeqNumber;
    ExpertDiscoveryData& edData;
    SequenceType edSeqType;

    /** Default size of "window" on a graph */
    static const int DEFAULT_WINDOW_SIZE = 2;

    /** Default size of "step" on a graph */
    static const int DEFAULT_WINDOW_STEP = 1;
};



}//namespace

#endif  