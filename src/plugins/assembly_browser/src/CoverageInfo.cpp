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

#include <algorithm>

namespace U2 {

CalcCoverageInfoTask::CalcCoverageInfoTask(const CalcCoverageInfoTaskSettings & settings_) :
BackgroundTask<CoverageInfo>("Calculate assembly coverage", TaskFlag_None), settings(settings_)
{
    tpm = Progress_Manual;
};

// to calc sum in QVector
struct SumCounter {
    qint64 sum;
    SumCounter() : sum(0) {}
    void operator()(qint64 x) {sum += x;}
};

void CalcCoverageInfoTask::run() {
    // calculate coverage
    U2AssemblyCoverageStat stat;
    stat.coverage.resize(settings.regions);
    U2OpStatusImpl os;
    settings.model->calculateCoverageStat(settings.visibleRange, stat, os, stateInfo);
    if(os.hasError()) {
        setError(os.getError());
        return;
    }
    
    // fill the result
    result.coverageInfo = stat.coverage;
    result.maxCoverage = *std::max_element(result.coverageInfo.constBegin(), result.coverageInfo.constEnd());
    result.minCoverage = *std::min_element(result.coverageInfo.constBegin(), result.coverageInfo.constEnd());

    SumCounter counter = std::for_each(result.coverageInfo.constBegin(), result.coverageInfo.constEnd(), SumCounter());
    result.averageCoverage = double(counter.sum) / settings.regions;
}

}
