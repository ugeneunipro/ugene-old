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

#include "AssemblyPackAlgorithm.h"

#include <U2Core/Timer.h>

#include <memory>

namespace U2 {

#define TAIL_SIZE 50000

static qint64 selectProw(qint64* tails, qint64 start, qint64 end ){
    for (int i = 0; i < TAIL_SIZE; i++) {
        if (tails[i] < start) {
            tails[i] = end;
            return i;
        }
    }
    return -1;
}

#define PACK_TRACE_CHECKPOINT 100000

void AssemblyPackAlgorithm::pack(PackAlgorithmAdapter& adapter, U2AssemblyPackStat& stat, U2OpStatus& os) {
    //Algorithm idea: 
    //  select * reads ordered by start position
    //  keep tack (tail) of used rows to assign packed row for reads (N elements)
    //  if all elements are used -> assign -1 to read and postprocess it later

    GTIMER(c1, t1, "AssemblyPackAlgorithm::pack");
    quint64 t0 = GTimer::currentTimeMicros();
    int nPacked = 0;

    stat.maxProw = 0;
    QVarLengthArray<qint64, TAIL_SIZE> tails;
    qFill(tails.data(), tails.data() + TAIL_SIZE, -1);
    std::auto_ptr< U2DbiIterator<PackAlgorithmData> > allReadsIterator(adapter.selectAllReads(os));
    int peakEnd = -1; // used to assign prow for reads when TAIL_SIZE is not enough
    int peakRow = TAIL_SIZE;
    while (allReadsIterator->hasNext() && !os.isCoR()) {
        PackAlgorithmData read = allReadsIterator->next();
        int prow = selectProw(tails.data(), read.leftmostPos, read.leftmostPos + read.effectiveLen);
        if (prow == -1) {
            if (read.leftmostPos > peakEnd) {
                peakRow = TAIL_SIZE;
            }
            prow = peakRow;
            peakRow++;
            peakEnd = read.leftmostPos + read.effectiveLen;
        }
        adapter.assignProw(read.readId, prow, os);
        stat.maxProw = qMax(prow, stat.maxProw);
        
        if ((++nPacked % PACK_TRACE_CHECKPOINT) == 0) {
            perfLog.trace(QString("Assembly: number packed reads so far: %1").arg(nPacked));
        }
    }

    t1.stop();
    perfLog.trace(QString("Assembly: algorithm pack time: %1 seconds").arg((GTimer::currentTimeMicros() - t0) / float(1000*1000)));
}

} //namespace
