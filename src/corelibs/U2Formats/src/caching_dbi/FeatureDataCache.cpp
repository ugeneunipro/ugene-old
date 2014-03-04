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

#include "FeatureDataCache.h"

namespace U2 {

FeatureWithKeys::FeatureWithKeys( ) {

}

FeatureWithKeys::FeatureWithKeys( const U2Feature &f, const QList<U2FeatureKey> &k )
    : feature( f ), keys( k )
{

}

FeatureDataCache::FeatureDataCache( qint64 maxCacheSizeInBytes )
    : U2DbiDataCache<U2DataId, FeatureWithKeys>( Discrete )
{
    U2OpStatusImpl os;
    init( maxCacheSizeInBytes, os );
    SAFE_POINT_OP( os, );
}

int FeatureDataCache::estimateDataSizeInBytes( const U2DataId & ) const {
    // don't take into account size of possible feature keys

    const int minSizeOfDbId = 10;
    const int countOfDbIdField = 3; // feature's ID, seq ID, parent feature's ID
    const int maxFeatureNameLength = 100;
    return sizeof( U2Feature ) + countOfDbIdField * sizeof( char ) * minSizeOfDbId
        + maxFeatureNameLength * sizeof( QChar );
}

} // namespace U2
