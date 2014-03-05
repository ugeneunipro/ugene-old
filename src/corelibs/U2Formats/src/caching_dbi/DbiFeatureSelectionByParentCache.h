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

#ifndef _U2_DBI_FEATURE_SELECTION_BY_PARENT_CACHE_H_
#define _U2_DBI_FEATURE_SELECTION_BY_PARENT_CACHE_H_

#include "DbiCustomSelectionCache.h"
#include "FeatureDataCache.h"

namespace U2 {

struct SubfeatureFilterParams {
    SubfeatureFilterParams( const U2DataId &parentId, const U2DataId &seqId, const QString &name );

    bool operator ==( const SubfeatureFilterParams &other ) const;

    U2DataId    parentId;
    U2DataId    seqId;
    QString     name;
};

uint qHash( const SubfeatureFilterParams &c );

class DbiFeatureSelectionByParentCache
    : public DbiCustomSelectionCache<U2DataId, FeatureWithKeys, SubfeatureFilterParams, U2Feature>
{
public :
    DbiFeatureSelectionByParentCache( U2DbiDataCache<U2DataId, FeatureWithKeys> *mainCache );

private :
    bool satisfies( const SubfeatureFilterParams &filter, const U2DataId &featureId );
    U2Feature excerpt( const FeatureWithKeys &data ) const;
};

}

#endif // _U2_DBI_FEATURE_SELECTION_BY_PARENT_CACHE_H_
