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

#ifndef _U2_FEATURE_DBI_H_
#define _U2_FEATURE_DBI_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2Feature.h>

namespace U2 {

enum ComparisonOp {
    ComparisonOp_Invalid,
    ComparisonOp_EQ,
    ComparisonOp_NEQ,
    ComparisonOp_GT,
    ComparisonOp_GET,
    ComparisonOp_LT,
    ComparisonOp_LET
};

enum OrderOp {
    OrderOp_None,
    OrderOp_Asc,
    OrderOp_Desc
};

class FeatureQuery {
public:
    FeatureQuery() : topLevelOnly(false),  keyNameOrderOp(OrderOp_None),
        keyValueCompareOp(ComparisonOp_Invalid), keyValueOrderOp(OrderOp_None), 
        intersectRegion(-1, 0), startPosOrderOp(OrderOp_None) {}
    
    U2DataId        sequenceId;

    U2DataId        parentFeatureId;
    bool            topLevelOnly;
    
    QString         keyName;
    OrderOp         keyNameOrderOp;

    QString         keyValue;
    ComparisonOp    keyValueCompareOp;
    OrderOp         keyValueOrderOp;
    
    /** Indicates the region to intersect
        Special values: 
            startPos = -1 and len = 0 :   filter is disabled
    */
    U2Region        intersectRegion;
    OrderOp         startPosOrderOp;
};
/**
    An interface to obtain 'read' access to sequence features
*/
class U2CORE_EXPORT U2FeatureDbi : public U2ChildDbi {
protected:
    U2FeatureDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi){}

public:
    
    /** 
        Reads feature data by id 
    */
    virtual U2Feature getFeature(const U2DataId& featureId, U2OpStatus& os) = 0;
    
    /**
        Counts features that matched the query.
    */
    virtual qint64 countFeatures(const FeatureQuery& q, U2OpStatus& os) = 0;
    
    /**
        Returns features that matched the query. Returns NULL if error occurs
    */
    virtual U2DbiIterator<U2Feature>* getFeatures(const FeatureQuery& q, U2OpStatus& os) = 0;

    
    
    /**
        Creates new feature in database. Uses all fields in 'feature' param and assign database id to it as the result
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void createFeature(U2Feature& feature, const QList<U2FeatureKey>& keys, U2OpStatus& os) = 0;



    /**
        Adds key to feature
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void addKey(const U2DataId& featureId, const U2FeatureKey& key, U2OpStatus& os) = 0;
    
    /**
        Removes all feature keys with a specified name
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void removeAllKeys(const U2DataId& featureId, const QString& keyName, U2OpStatus& os) = 0;
    
    /**
        Removes all feature keys with a specified name and value
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void removeAllKeys(const U2DataId& featureId, const U2FeatureKey& key, U2OpStatus& os) = 0;

    /**
        Updates feature location. Features with U2Region(0,0) have no specified location
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void updateLocation(const U2DataId& featureId, const U2FeatureLocation& location, U2OpStatus& os) = 0;



    /**
        Removes the feature from database
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void removeFeature(const U2DataId& featureId, U2OpStatus& os) = 0;
    
};

} //namespace

#endif
