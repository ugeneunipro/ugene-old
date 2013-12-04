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

#ifndef _U2_FEATURES_TABLE_OBJECT_H_
#define _U2_FEATURES_TABLE_OBJECT_H_

#include <U2Core/Annotation.h>
#include <U2Core/AnnotationGroup.h>
#include <U2Core/GObject.h>
#include <U2Core/U2Feature.h>

namespace U2 {

class U2CORE_EXPORT FeaturesTableObject : public GObject {
    Q_OBJECT
public:
                            FeaturesTableObject( const QString &objectName, const U2DbiRef &dbiRef,
                                const QVariantMap &hintsMap = QVariantMap( ) );
     virtual                ~FeaturesTableObject( );

    /**
     * Converts all the features stored in DB to annotations and returns the result
     */
    QList<__Annotation>     getAnnotations( );
    /**
     * Returns root group
     */
    __AnnotationGroup       getRootGroup( );
    /**
     * Adds to DB a set of features based on @a
     */
    void                    addAnnotation( AnnotationData &a, const QString &groupName = QString( ) );
    /**
     * Adds to DB a set of features based on @annotations
     */
    void                    addAnnotations( QList<AnnotationData> &annotations,
                                const QString &groupName = QString( ) );
    /**
     * Removes features corresponding to @a from DB
     */
    void                    removeAnnotation( const __Annotation &a );
    /**
     * Removes features corresponding to @annotations from DB
     */
    void                    removeAnnotations( const QList<__Annotation> &annotations );
    /**
     * Reimplemented from GObject
     */
    virtual GObject *       clone( const U2DbiRef &ref, U2OpStatus &os ) const;
    /**
     * Returns list of annotations having @name
     */
    QList<__Annotation>     getAnnotationsByName( const QString &name );
    /**
     * Returns list containing all annotation regions
     */
    QList<U2Region>         getAnnotatedRegions( ) const;
    /**
     * Returns list of annotations having belonging to the @region. @contains specifies
     * whether the result set should include only annotations that has no region or its part
     * beyond the @region or each annotation that intersects it.
     */
    QList<__Annotation>     getAnnotationsByRegion( const U2Region &region,
                                bool contains = false );
    /**
     * Reimplemented from GObject
     */
    bool                    checkConstraints( const GObjectConstraints *c ) const;

    //////////////////////////////////////////////////////////////////////////
    // Direct features interface (without sync with annotations) /////////////
    /**
     * Returns the root feature
     */
    U2DataId               getRootFeatureId( ) const;
    /**
     * Adds a new feature. Sets its id to the correct value. Also adds the list of keys
     * to the feature. If not specified otherwise, parentFeatureId is set to the root feature.
     */
    void                    addFeature( U2Feature &f, QList<U2FeatureKey> keys, U2OpStatus &os );

private:
    void                    removeAnnotationFromDb( const __Annotation &a );
    void                    copyFeaturesToObject( const U2Feature &feature,
                                const U2DataId &newParentId, FeaturesTableObject *obj,
                                U2OpStatus &os ) const;
    QList<__Annotation>     convertFeaturesToAnnotations( const QList<U2Feature> &features );

    U2DataId                rootFeatureId;
};

} // namespace

#endif
