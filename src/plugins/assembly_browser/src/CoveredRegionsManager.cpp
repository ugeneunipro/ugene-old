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
#include <algorithm>
#include <iterator>

namespace U2 {

CoveredRegionsManager::CoveredRegionsManager(const U2Region & visibleRegion_, const QVector<qint64> & coverageInfo) :
visibleRegion(visibleRegion_)  {
    assert(!coverageInfo.empty());
    assert(!visibleRegion.isEmpty());

    //convert coverage info to covered regions
    //splitting visible region to coverageInfo.size() "covered" regions
    double step = double(visibleRegion.length)/coverageInfo.size(); //precise length of the region
    for(int i = 0; i < coverageInfo.size(); ++i) {
        U2Region region(step*i, qint64(step)); 
        CoveredRegion coveredRegion(region, coverageInfo[i]);
        allRegions.push_back(coveredRegion);
    }

};

QList<CoveredRegion> CoveredRegionsManager::getCoveredRegions(qint64 coverageLevel) const {
    QList<CoveredRegion> coveredRegions;
    foreach(CoveredRegion cr, allRegions) {
        if(cr.coverage >= coverageLevel) {
            coveredRegions.push_back(cr);
        }
    }
    return coveredRegions;
}

QList<CoveredRegion> CoveredRegionsManager::getTopCoveredRegions(int topMax, qint64 coverageLevel/*=0*/) const {
    assert(topMax > 0);
    QMultiMap<qint64, CoveredRegion> topCovered;

    int end = qMin(topMax, allRegions.size());
    for(int i = 0; i < end; ++i) {
        const CoveredRegion & cr = allRegions.at(i);
        if(cr.coverage >= coverageLevel) {
            topCovered.insert(cr.coverage, cr);
        }
        if(topCovered.size() > topMax) {
            topCovered.erase(topCovered.begin());
        }
    }
    assert(topCovered.size() <= topMax);
    QList<CoveredRegion> topCoveredList = topCovered.values();
    QList<CoveredRegion> result;
    //reverse copy, since topCoveredList is sorted in the ascending order
    std::copy(topCoveredList.begin(), topCoveredList.end(), std::front_inserter(result));
    return result;
}

}
