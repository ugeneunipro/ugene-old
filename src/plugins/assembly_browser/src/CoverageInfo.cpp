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

#include "CoverageInfo.h"

#include <U2Core/U2OpStatusUtils.h>
#include "AssemblyModel.h"

namespace U2 {

CalcCoverageInfoTask::CalcCoverageInfoTask(const CalcCoverageInfoTaskSettings & settings_) :
BackgroundTask<CoverageInfo>("Calculate assembly coverage", TaskFlag_None), settings(settings_)
{
    tpm = Progress_Manual;
};

void CalcCoverageInfoTask::run() {
    const int numOfRegions = settings.regions;
    result.coverageInfo.resize(settings.regions);

    double basesPerRegion = double(settings.visibleRange.length) / numOfRegions;
    qint64 maxReadsPerRegion = 0;
    qint64 minReadsPerRegion = qint64(1) << 62;
    qint64 sum = 0;
    qint64 start = settings.visibleRange.startPos;

    for(int i = 0 ; i < numOfRegions; ++i) {
        //jump to next region
        start = settings.visibleRange.startPos + basesPerRegion * i;

        //check cancel and update progress
        if(stateInfo.cancelFlag) {
            return;
        }
        stateInfo.progress = double(i) / numOfRegions * 100.;

        //get region coverage info from DB
        U2OpStatusImpl status;
        qint64 readsPerRegion = settings.model->countReadsInAssembly(U2Region(start, qRound64(basesPerRegion)), status);
        if(status.hasError()) {
            stateInfo.setError(status.getError());
            return;
        }
        result.coverageInfo[i] = readsPerRegion;
                      
        //update min and max
        if(maxReadsPerRegion < readsPerRegion) {
            maxReadsPerRegion = readsPerRegion;
        }
        if(minReadsPerRegion > readsPerRegion) {
            minReadsPerRegion = readsPerRegion;
        }
        sum += readsPerRegion;
    }

    result.maxCoverage = maxReadsPerRegion;
    result.minCoverage = minReadsPerRegion;

    U2OpStatusImpl status;
    result.averageCoverage = double(sum) / numOfRegions;
}

}
