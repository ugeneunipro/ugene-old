/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _DBI_FEATURE_SELECTION_BY_REGION_CACHE_H_
#define _DBI_FEATURE_SELECTION_BY_REGION_CACHE_H_

#include "DbiCustomSelectionCache.h"
#include "DbiFeatureSelectionByParentCache.h"
#include "FeatureDataCache.h"

namespace U2 {

struct FeatureRegionFilterParams : public SubfeatureFilterParams {
    FeatureRegionFilterParams( const U2Region &region, bool contains, const U2DataId &parentId,
        const U2DataId &seqId, const QString &name );

    bool operator ==( const FeatureRegionFilterParams &other ) const;

    U2Region    region;
    bool        contains;
};

class DbiFeatureSelectionByRegionCache
    : public DbiCustomSelectionCache<U2DataId, FeatureWithKeys, FeatureRegionFilterParams, U2Feature>
{
public :
    DbiFeatureSelectionByRegionCache( U2DbiDataCache<U2DataId, FeatureWithKeys> *mainCache );

private :
    bool satisfies( const FeatureRegionFilterParams &filter, const U2DataId &featureId );

    U2Feature excerpt( const FeatureWithKeys &data ) const;
};

}

#endif // _DBI_FEATURE_SELECTION_BY_REGION_CACHE_H_
