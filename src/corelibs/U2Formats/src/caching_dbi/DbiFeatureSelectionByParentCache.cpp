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

#include "DbiFeatureSelectionByParentCache.h"

uint qHash( const U2::SubfeatureFilterParams &c ) {
    const int magicNumber = 0x9e3779b9;
    uint seed = 0;
    seed ^= qHash( c.parentId ) + magicNumber + ( seed << 8 ) + ( seed >> 2 );
    seed ^= qHash( c.seqId ) + magicNumber + ( seed << 7 ) + ( seed >> 3 );
    seed ^= qHash( c.name ) + magicNumber + ( seed << 6 ) + ( seed >> 4 );
    return seed;
}

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// SubfeatureFilterParams ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

SubfeatureFilterParams::SubfeatureFilterParams( const U2DataId &parentId, const U2DataId &seqId,
    const QString &name )
    : parentId( parentId ), seqId( seqId ), name( name )
{

}

bool SubfeatureFilterParams::operator ==( const SubfeatureFilterParams &other ) const {
    return parentId == other.parentId && seqId == other.seqId && name == other.name;
}

//////////////////////////////////////////////////////////////////////////
/// DbiFeatureSelectionByParentCache /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DbiFeatureSelectionByParentCache::DbiFeatureSelectionByParentCache(
    U2DbiDataCache<U2DataId, FeatureWithKeys> *mainCache )
    : DbiCustomSelectionCache<U2DataId, FeatureWithKeys, SubfeatureFilterParams, U2Feature>( mainCache )
{

}

bool DbiFeatureSelectionByParentCache::satisfies( const SubfeatureFilterParams &filter,
    const U2DataId &featureId )
{
    U2OpStatusImpl os;
    const FeatureWithKeys &data = cache->getData( featureId, os );
    SAFE_POINT_OP( os , false );
    bool result = ( filter.parentId == data.feature.parentFeatureId
        && ( filter.seqId.isEmpty( ) || filter.seqId == data.feature.sequenceId )
        && ( filter.name.isEmpty( ) || filter.name == data.feature.name ) );
    return result;
}

U2Feature DbiFeatureSelectionByParentCache::excerpt( const FeatureWithKeys &data ) const {
    return data.feature;
}

}
