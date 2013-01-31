/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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


#include "U2FeatureUtils.h"

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2FeatureDbi.h>
#include <U2Core/U2FeatureKeys.h>
#include <U2Core/U2SafePoints.h>

#include <QScopedPointer>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// FeatureSynchronizer

FeatureSynchronizer::FeatureSynchronizer(){
    //TODO: clear db?
}

void FeatureSynchronizer::exportAnnotationToFeatures(Annotation* a, const U2DataId& parentFeatureId, const U2DbiRef& dbiRef, U2OpStatus& op){
    //if annotation is in the map - update it
    

    //if not - create a new feature
    DbiConnection connection;
    connection.open(dbiRef, op);
    CHECK_OP(op, );

    const U2Location& location = a->getLocation();
    bool isMultyRegion = location->isMultiRegion();

    U2Feature feature;
    //copy data
    feature.name = a->getAnnotationName();
    feature.parentFeatureId = parentFeatureId;
    if(!isMultyRegion){
        if(!location->isEmpty()){
            feature.location.region = location->regions.first();
            feature.location.strand = location->strand;
        }
    }

    //add qualifiers
    const QVector<U2Qualifier>& quals = a->getQualifiers();
    QList<U2FeatureKey> fKeys;
    foreach(const U2Qualifier& qual, quals){
        fKeys.append(U2FeatureKey(qual.name, qual.value));
    }

    //add groups
    const QList<AnnotationGroup*>& groups = a->getGroups();
    foreach(AnnotationGroup* gr, groups){
        fKeys.append(U2FeatureKey(U2FeatureKeyGroup, gr->getGroupName()));
    }

    //add operation
    if(location->op == U2LocationOperator_Join){
        fKeys.append(U2FeatureKey(U2FeatureKeyOperation, U2FeatureKeyOperationJoin));
    }else{
        fKeys.append(U2FeatureKey(U2FeatureKeyOperation, U2FeatureKeyOperationOrder));
    }
    
    //store to db to get ID
    connection.dbi->getFeatureDbi()->createFeature(feature, fKeys, op);
    CHECK_OP(op, );

    //insert feature in sync table
    syncTable.insert(feature.id, a);

    //add subfeatures
    if(isMultyRegion){
        addSubFeatures(location->regions, location->strand, feature.id, connection.dbi->getFeatureDbi(), op);
    }
    
}
void FeatureSynchronizer::removeFeature (Annotation* a, const U2DataId& rootFeatureId, const U2DbiRef& dbiRef, U2OpStatus& op){
    U2DataId featureId = syncTable.key(a, U2DataId());
    if(featureId.isEmpty()){
        return;
    }

    DbiConnection connection;
    connection.open(dbiRef, op);
    CHECK_OP(op, );
    
    U2Feature feature = connection.dbi->getFeatureDbi()->getFeature(featureId, op);
    CHECK_OP(op, );
    SAFE_POINT(feature.parentFeatureId == rootFeatureId, "Trying to delete feature which has another parent", );

    //remove subfeatures    
    QList<U2Feature> subfeaturesList = U2FeaturesUtils::getSubFeatures(featureId, connection.dbi->getFeatureDbi(), op, true);
    CHECK_OP(op, )
    foreach(const U2Feature& f, subfeaturesList){
        connection.dbi->getFeatureDbi()->removeFeature(f.id, op);
        CHECK_OP(op, )
    }

    //remove feature
    syncTable.remove(featureId);
    connection.dbi->getFeatureDbi()->removeFeature(featureId, op);
    CHECK_OP(op, );
}

void FeatureSynchronizer::removeFeature (const U2DataId& rootFeatureId, const U2DbiRef& dbiRef, U2OpStatus& op){
    DbiConnection connection;
    connection.open(dbiRef, op);
    CHECK_OP(op, );

    QList<U2Feature> subfeaturesList = U2FeaturesUtils::getSubFeatures(rootFeatureId, connection.dbi->getFeatureDbi(), op, true);
    CHECK_OP(op, )
    foreach(const U2Feature& f, subfeaturesList){
        if(syncTable.contains(f.id)){
            syncTable.remove(f.id);
        }
        connection.dbi->getFeatureDbi()->removeFeature(f.id, op);
        CHECK_OP(op, )
    }
}

void FeatureSynchronizer::renameGroup( AnnotationGroup* g, const QString& oldName, const U2DataId& rootFeatureId, const U2DbiRef& dbiRef, U2OpStatus& op ){
    DbiConnection connection;
    connection.open(dbiRef, op);
    CHECK_OP(op, );
    
    foreach(Annotation* a, g->getAnnotations()){
        U2DataId fId = syncTable.key(a, U2DataId());
        if(fId.isEmpty()){
            continue;
        }
        connection.dbi->getFeatureDbi()->removeAllKeys(fId, U2FeatureKey(U2FeatureKeyGroup, oldName), op);
        CHECK_OP(op, );

        connection.dbi->getFeatureDbi()->addKey(fId, U2FeatureKey(U2FeatureKeyGroup, g->getGroupName()), op);
        CHECK_OP(op, );
    }
}

void FeatureSynchronizer::removeGroup( AnnotationGroup* parentGroup, AnnotationGroup* groupToDelete, const U2DataId& rootFeatureId, const U2DbiRef& dbiRef, U2OpStatus& op){
    DbiConnection connection;
    connection.open(dbiRef, op);
    CHECK_OP(op, );

    foreach(Annotation* a, groupToDelete->getAnnotations()){
        U2DataId fId = syncTable.key(a, U2DataId());
        if(fId.isEmpty()){
            continue;
        }
        connection.dbi->getFeatureDbi()->removeAllKeys(fId, U2FeatureKey(U2FeatureKeyGroup, groupToDelete->getGroupName()), op);
        CHECK_OP(op, );
    }
}

void FeatureSynchronizer::modifyFeatures( const AnnotationModification& md, const U2DataId& rootFeatureId, const U2DbiRef& dbiRef, U2OpStatus& op ){
    U2DataId featureId = syncTable.key(md.annotation, U2DataId());
    if(featureId.isEmpty()){
        return;
    }

    DbiConnection connection;
    connection.open(dbiRef, op);
    CHECK_OP(op, );

    switch(md.type){
        case AnnotationModification_NameChanged:
            {
                connection.dbi->getFeatureDbi()->updateName(featureId, md.annotation->getAnnotationName(), op);
                break;
            }
        case AnnotationModification_QualifierAdded:
            {
                const QualifierModification& qm = (const QualifierModification&)md;
                connection.dbi->getFeatureDbi()->addKey(featureId, U2FeatureKey(qm.qualifier.name, qm.qualifier.value), op);
                break;
            }
        case AnnotationModification_QualifierRemoved:
            {
                const QualifierModification& qm = (const QualifierModification&)md;
                connection.dbi->getFeatureDbi()->removeAllKeys(featureId, U2FeatureKey(qm.qualifier.name, qm.qualifier.value), op);
                break;
            }
        case AnnotationModification_LocationChanged:
            {
                //TODO: if there will be not only region subfeatures, update it another way
                qint64 subfeaturesCount = U2FeaturesUtils::countChildren(featureId, connection.dbi->getFeatureDbi(), op);
                if(subfeaturesCount >= 0){
                     removeFeature(featureId, dbiRef, op);
                }

                U2FeatureKey locationOpKey;
                if(md.annotation->getLocationOperator() == U2LocationOperator_Join){
                    locationOpKey = U2FeatureKey(U2FeatureKeyOperation, U2FeatureKeyOperationJoin);
                }else{
                    locationOpKey = U2FeatureKey(U2FeatureKeyOperation, U2FeatureKeyOperationOrder);
                }

                if(md.annotation->getRegions().size() == 1){
                    U2FeatureLocation location;
                    location.region = md.annotation->getRegions().first();
                    location.strand = md.annotation->getStrand();
                    connection.dbi->getFeatureDbi()->updateLocation(featureId, location, op);
                    connection.dbi->getFeatureDbi()->updateKeyValue(featureId, locationOpKey, op);

                }else{
                    //split
                    U2FeatureLocation location;
                    location.region = U2Region(0,0);
                    location.strand = U2Strand::Direct;
                    connection.dbi->getFeatureDbi()->updateLocation(featureId, location, op);
                    connection.dbi->getFeatureDbi()->updateKeyValue(featureId, locationOpKey, op);
                    
                    addSubFeatures(md.annotation->getRegions(), md.annotation->getStrand(), featureId, connection.dbi->getFeatureDbi(), op);
                }
                break;
            }
        case AnnotationModification_AddedToGroup:
            {
                //TODO: check if it's already in the group
                const AnnotationGroupModification& gmd = (const AnnotationGroupModification&)md;
                connection.dbi->getFeatureDbi()->addKey(featureId, U2FeatureKey(U2FeatureKeyGroup, gmd.group->getGroupName()), op);
                break;
            }
        case AnnotationModification_RemovedFromGroup:
            {
                const AnnotationGroupModification& gmd = (const AnnotationGroupModification&)md;
                connection.dbi->getFeatureDbi()->removeAllKeys(featureId, U2FeatureKey(U2FeatureKeyGroup, gmd.group->getGroupName()), op);
                break;
            }
    }
        
}

void FeatureSynchronizer::addSubFeatures( const QVector<U2Region> & regions, const U2Strand& strand, const U2DataId& parentFeatureId, U2FeatureDbi* fDbi, U2OpStatus& op ){
    SAFE_POINT(fDbi != NULL, "Feature DBI is NULL", );

    foreach(const U2Region& reg, regions){
        U2Feature sub;
        sub.location.region = reg;
        sub.location.strand = strand;
        sub.parentFeatureId = parentFeatureId;
        //to db
        fDbi->createFeature(sub, QList<U2FeatureKey>(), op);
        CHECK_OP(op, );
    }
}


//////////////////////////////////////////////////////////////////////////
/// U2FeaturesUtils

QList<U2Feature> U2FeaturesUtils::getSubFeatures(const U2DataId& parentFeatureId, U2FeatureDbi* fdbi, U2OpStatus& os, bool resursive){
    QList<U2Feature> result;

    SAFE_POINT(fdbi!=NULL, "Feature Dbi is null", result);

    FeatureQuery query;
    query.parentFeatureId = parentFeatureId;

    
    QScopedPointer< U2DbiIterator<U2Feature> > featureIter (fdbi->getFeatures(query, os));
    CHECK_OP(os, result);

    while(featureIter->hasNext()){
        U2Feature feature = featureIter->next();
        result.append(feature);
        if(resursive){
            QList<U2Feature> sublist = getSubFeatures(feature.id, fdbi, os, resursive);
            CHECK_OP(os, result);
            result.append(sublist);
        }
    }
    
    return result;
}

QList<U2Feature> U2FeaturesUtils::getFeatureByName(const U2DataId& parentFeatureId, U2FeatureDbi* fdbi, const QString& name, U2OpStatus& os){
    QList<U2Feature> result;

    SAFE_POINT(fdbi!=NULL, "Feature Dbi is null", result);

    FeatureQuery query;
    query.parentFeatureId = parentFeatureId;
    query.featureName = name;

    QScopedPointer< U2DbiIterator<U2Feature> > featureIter (fdbi->getFeatures(query, os));
    CHECK_OP(os, result);

    while(featureIter->hasNext()){
        U2Feature feature = featureIter->next();
        result.append(feature);
    }

    return result;
}

QList<U2Feature> U2FeaturesUtils::getChildFeatureSublist( const U2DataId& parentFeatureId, const QList<U2Feature>& features ){
    QList<U2Feature> result;
    foreach(const U2Feature& f, features){
        if(f.parentFeatureId == parentFeatureId){
            result.append(f);
        }
    }
    return result;
}

qint64 U2FeaturesUtils::countChildren( const U2DataId& parentFeatureId, U2FeatureDbi* fdbi, U2OpStatus& os ){
    SAFE_POINT(fdbi!=NULL, "Feature Dbi is null", -1);

    FeatureQuery query;
    query.parentFeatureId = parentFeatureId;
    query.topLevelOnly = false;

    qint64 result = fdbi->countFeatures(query, os);
    CHECK_OP(os, result);

    return result;
}

void U2FeaturesUtils::clearKeys( QList<U2FeatureKey> & fKeys ){
    foreach(const U2FeatureKey& key, fKeys){
        if(key.name == U2FeatureKeyGroup || key.name == U2FeatureKeyOperation){
            fKeys.removeOne(key);
        }
    }
}

void U2FeaturesUtils::clearKeys( const U2DataId& parentFeatureId, U2FeatureDbi* fdbi, U2OpStatus& op ){
    //root
    fdbi->removeAllKeys(parentFeatureId, U2FeatureKeyGroup, op);
    CHECK_OP(op, )
        fdbi->removeAllKeys(parentFeatureId, U2FeatureKeyOperation, op);
    CHECK_OP(op, )

    //children
    QList<U2Feature> subfeaturesList = U2FeaturesUtils::getSubFeatures(parentFeatureId, fdbi, op, true);
    CHECK_OP(op, )
        foreach(const U2Feature& f, subfeaturesList){
            fdbi->removeAllKeys(f.id, U2FeatureKeyGroup, op);
             CHECK_OP(op, )
            fdbi->removeAllKeys(f.id, U2FeatureKeyOperation, op);
            CHECK_OP(op, )
    } 
}

} //namespace
