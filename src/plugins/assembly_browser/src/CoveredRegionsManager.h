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

#ifndef __COVERED_REGIONS_MANAGER_H__
#define __COVERED_REGIONS_MANAGER_H__

#include <QtCore/QVector>
#include "CoverageInfo.h"

namespace U2 {

struct CoveredRegion {
    CoveredRegion(const U2Region & region_, qint64 coverage_) : region(region_), coverage(coverage_) {}
    U2Region region;
    qint64 coverage;
};

class CoveredRegionsManager {
public:
    CoveredRegionsManager() : levelOfInterest(0) {

    }
    CoveredRegionsManager(const U2Region & visibleRegion_, const QVector<qint64> & coverageInfo_) :
    visibleRegion(visibleRegion_), coverageInfo(coverageInfo_), levelOfInterest(0) {
    };

    inline bool isEmpty() const {return coverageInfo.empty();}

    void setDesiredCoverageLevel(int coverageLevel);

    inline int getSize() const {
        return coverageInfo.size();
    }

    inline QList<CoveredRegion> getCoveredRegions() const {
        return coveredRegions;
    }

private:
    void findCoveredRegions();

    U2Region visibleRegion;
    QVector<qint64> coverageInfo;
    qint64 levelOfInterest;
    QList<CoveredRegion> coveredRegions;
};


}

#endif
