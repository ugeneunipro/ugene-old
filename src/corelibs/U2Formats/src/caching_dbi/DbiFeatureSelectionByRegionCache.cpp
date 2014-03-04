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

#include "DbiFeatureSelectionByRegionCache.h"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// FeatureRegionFilterParams ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

FeatureRegionFilterParams::FeatureRegionFilterParams( const U2Region &region, bool contains,
    const U2DataId &parentId, const U2DataId &seqId, const QString &name )
    : SubfeatureFilterParams( parentId, seqId, name ), region( region ), contains( contains )
{

}

bool FeatureRegionFilterParams::operator ==( const FeatureRegionFilterParams &other ) const {
    return SubfeatureFilterParams::operator==( other ) && region == other.region
        && contains == other.contains;
}

//////////////////////////////////////////////////////////////////////////
/// DbiFeatureSelectionByRegionCache /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DbiFeatureSelectionByRegionCache::DbiFeatureSelectionByRegionCache(
    U2DbiDataCache<U2DataId, FeatureWithKeys> *mainCache )
    : DbiCustomSelectionCache<U2DataId, FeatureWithKeys, FeatureRegionFilterParams, U2Feature>( mainCache )
{

}

bool DbiFeatureSelectionByRegionCache::satisfies( const FeatureRegionFilterParams &filter,
    const U2DataId &featureId )
{
    U2OpStatusImpl os;
    const FeatureWithKeys &data = cache->getData( featureId, os );
    SAFE_POINT_OP( os , false );
    bool result = (
        ( filter.parentId.isEmpty( ) || filter.parentId == data.feature.parentFeatureId )
        && ( filter.seqId.isEmpty( ) || filter.seqId == data.feature.sequenceId )
        && ( filter.name.isEmpty( ) || filter.name == data.feature.name ) );
    if ( result ) {
        result &= ( filter.contains && filter.region.contains( data.feature.location.region ) )
            || ( !filter.contains && filter.region.intersects( data.feature.location.region ) );
    }
    return result;
}

U2Feature DbiFeatureSelectionByRegionCache::excerpt( const FeatureWithKeys &data ) const {
    return data.feature;
}

}
