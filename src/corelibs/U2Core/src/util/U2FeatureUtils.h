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

#ifndef _U2_FEATURE_UTILS_H_
#define _U2_FEATURE_UTILS_H_

//#include <U2Core/AnnotationTableObject.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2Type.h>
#include <U2Core/U2OpStatus.h>

namespace U2 {

class Annotation;
class AnnotationGroup;
class AnnotationModification;
class U2FeatureDbi;

/**All queries to feature dbi from annotation dbi must be throw this object 
it synchronizes annotations and corresponding features*/

 //TODO: add dbi connection to the class?
class U2CORE_EXPORT FeatureSynchronizer{
public:
    FeatureSynchronizer();
    
    /** in case of multiple regions, feature contains all the names and qualifiers and subfeatures contain only regions,
        in case of single regions features contains all the information from the annotation*/
    void exportAnnotationToFeatures(Annotation* a, const U2DataId& parentFeatureId, const U2DbiRef& dbiRef, U2OpStatus& op);

    /**Removes corresponding to annotation feature and its children from db. RootId feature is not deleted*/
    void removeFeature (Annotation* a, const U2DataId& rootFeatureId, const U2DbiRef& dbiRef, U2OpStatus& op);

    /**remove features and it subfeatures with given parentID. RootId feature is not deleted*/
    void removeFeature (const U2DataId& rootFeatureId, const U2DbiRef& dbiRef, U2OpStatus& op);

    /**rename group in db*/
    void renameGroup( AnnotationGroup* g, const QString& oldName, const U2DataId& rootFeatureId, const U2DbiRef& dbiRef, U2OpStatus& op);

    /**remove group in db
    TODO: use subgrouping*/
    void removeGroup( AnnotationGroup* parentGroup, AnnotationGroup* groupToDelete, const U2DataId& rootFeatureId, const U2DbiRef& dbiRef, U2OpStatus& op);

    /**use annotation modification to update features*/
    void modifyFeatures( const AnnotationModification& md, const U2DataId& rootFeatureId, const U2DbiRef& dbiRef, U2OpStatus& op);

private:
    void addSubFeatures( const QVector<U2Region> & regions, const U2Strand& strand, const U2DataId& parentFeatureId, U2FeatureDbi* fDbi, U2OpStatus& op);
    
private:
    QHash<U2DataId, Annotation*> syncTable;
};

class U2CORE_EXPORT U2FeaturesUtils{
public:
    /** return list of all child features of parent feature with given id. 
    if recursive is true the list contains all the children of child features and so on*/
    static QList<U2Feature> getSubFeatures(const U2DataId& parentFeatureId, U2FeatureDbi* fdbi, U2OpStatus& os, bool resursive = false);

    /**Get features by its name and given parent feature id*/
    static QList<U2Feature> getFeatureByName(const U2DataId& parentFeatureId, U2FeatureDbi* fdbi, const QString& name, U2OpStatus& os);

    /**Return sublist of features which have given parentId*/
    static QList<U2Feature> getChildFeatureSublist(const U2DataId& parentFeatureId, const QList<U2Feature>& features);

    /**Counts children which one level below the parentFeature*/
    static qint64 countChildren(const U2DataId& parentFeatureId, U2FeatureDbi* fdbi, U2OpStatus& os);

    /**Delete groups and operation keys from list*/
    static void clearKeys (QList<U2FeatureKey> & fKeys);

    /**Delete groups and operation keys from db
    Recursively from the parent feature*/
    static void clearKeys (const U2DataId& parentFeatureId, U2FeatureDbi* fdbi, U2OpStatus& os);
};

} //namespace


#endif //_U2_FEATURE_UTILS_H_
