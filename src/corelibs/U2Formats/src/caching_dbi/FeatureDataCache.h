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

#ifndef _U2_FEATURE_DATA_CACHE_H_
#define _U2_FEATURE_DATA_CACHE_H_

#include <U2Core/U2Feature.h>
#include <U2Core/U2DbiDataCache.h>

namespace U2 {

struct FeatureWithKeys {
                            FeatureWithKeys( );
                            FeatureWithKeys( const U2Feature &f, const QList<U2FeatureKey> &k );

    U2Feature               feature;
    QList<U2FeatureKey>     keys;
};

class FeatureDataCache :    public U2DbiDataCache<U2DataId, FeatureWithKeys> {
public :
                            FeatureDataCache( qint64 maxCacheSizeInBytes );

private :
    int                     estimateDataSizeInBytes( const U2DataId &id ) const;
};

} // namespace U2

#endif // _U2_ANNOTATION_DATA_CACHE_H_
