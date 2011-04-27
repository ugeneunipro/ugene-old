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

#include "CoveredRegionsManager.h"

namespace U2 {

void CoveredRegionsManager::setDesiredCoverageLevel(int loi) {
    levelOfInterest = loi;
    findCoveredRegions();
}

void CoveredRegionsManager::findCoveredRegions() {
    coveredRegions.clear();
    double step = double(visibleRegion.length)/coverageInfo.size();
    for(int i = 0; i < coverageInfo.size(); ++i) {
        qint64 * p = &coverageInfo[0];
        U2Region region(step*i, qint64(step));
        //coreLog.info(QString("region: (%1, %2), coverage: %3").arg(region.startPos, region.endPos(), coverageInfo[i]));
        if(coverageInfo[i] >= levelOfInterest) {
            //    U2Region region(step*i, qint64(step));
            CoveredRegion coveredRegion(region, coverageInfo[i]);
            coveredRegions.push_back(coveredRegion);
        }
    }
}

}
