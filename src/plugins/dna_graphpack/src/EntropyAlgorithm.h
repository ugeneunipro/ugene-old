/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GRAPHPACK_ENTROPY_H_
#define _U2_GRAPHPACK_ENTROPY_H_

#include "DNAGraphPackPlugin.h"

#include <U2View/GSequenceGraphView.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QAction>
#else
#include <QtWidgets/QAction>
#endif
#include <QtCore/QList>
#include <QtCore/QBitArray>

namespace U2 {

class AnnotatedDNAView;

class EntropyGraphFactory : public GSequenceGraphFactory {
    Q_OBJECT
public:
    EntropyGraphFactory(QObject* p);
    QList<QSharedPointer<GSequenceGraphData> > createGraphs(GSequenceGraphView* v);
    bool isEnabled(const U2SequenceObject* o) const;
};

class EntropyGraphAlgorithm : public GSequenceGraphAlgorithm {
public:
    void calculate(QVector<float>& res, U2SequenceObject* o, const U2Region& r, const GSequenceGraphWindowData* d, U2OpStatus &os);
};

} // namespace
#endif
