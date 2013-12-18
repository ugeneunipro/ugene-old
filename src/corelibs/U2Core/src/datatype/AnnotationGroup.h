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

#ifndef _U2_ANNOTATION_GROUP_H_
#define _U2_ANNOTATION_GROUP_H_

#include <U2Core/Annotation.h>
#include <U2Core/DbiIdBasedData.h>

namespace U2 {

class FeaturesTableObject;

// TODO: this class should replace U2::AnnotationGroup. It should implement the interface
// defined below using U2::FeatureUtils
class U2CORE_EXPORT __AnnotationGroup : public DbiIdBasedData {
public:
                            __AnnotationGroup( const U2DataId &featureId,
                                FeaturesTableObject *parentObject );
                            ~__AnnotationGroup( );
    /*
     * Returning value specifies whether the @name may be used as an annotation name or not.
     * @pathMode allows to take into account group path separation symbol (currently "/")
     * I.e. if @pathMode is true then that symbol is allowed, otherwise it is not.
     */
    static bool             isValidGroupName( const QString &name, bool pathMode );
    /*
     * After calling this function @set additionally contains all the distinct annotations
     * belonging to the subtree of this annotation group
     */
    void                    findAllAnnotationsInGroupSubTree( QList<__Annotation> &set ) const;

    QList<__Annotation>     getAnnotations( ) const;

    __Annotation            addAnnotation( const AnnotationData &a );

    void                    addAnnotation( const __Annotation &a );

    void                    removeAnnotation( const __Annotation &a );

    void                    removeAnnotations( const QList<__Annotation> &annotations );

    QList<__AnnotationGroup> getSubgroups( ) const;

    void                    removeSubgroup( __AnnotationGroup &g );

    QString                 getName( ) const;

    void                    setName( const QString &newName );

    QString                 getGroupPath( ) const;

    FeaturesTableObject *   getGObject( ) const;

    /*
     * Returns parent annotation group. Returns *this (i.e. the same group) for a top-level group
     */
    __AnnotationGroup       getParentGroup( ) const;
    /*
     * Returns subgroup located in @path. @create specifies whether it's required to create
     * a new group(s) accordingly to @path if they are not exist. If @create == false and
     * there are no subgroups satisfying @path, then *this is returned.
     */
    __AnnotationGroup       getSubgroup( const QString &path, bool create );

    void                    getSubgroupPaths( QStringList &res ) const;
    /**
     * Removes all references to subgroups and annotations
     */
    void                    clear( );
    /**
     * Returns the depth of a root subtree containing the feature. For top-level features depth is "1"
     */
    int                     getGroupDepth( ) const;

    bool                    isParentOf( const __AnnotationGroup &g ) const;
    /**
     * Returns "true" if the parent of the group is root group
     */
    bool                    isTopLevelGroup( ) const;

    static const QString    ROOT_GROUP_NAME;
    static const QChar      GROUP_PATH_SEPARATOR;

private:
    FeaturesTableObject *   parentObject;
};

} // namespace U2

#endif // 
