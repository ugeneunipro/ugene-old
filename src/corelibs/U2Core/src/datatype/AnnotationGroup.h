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

namespace U2 {

class FeaturesTableObject;

// TODO: this class should replace U2::AnnotationGroup. It should implement the interface
// defined below using U2::FeatureUtils
class U2CORE_EXPORT __AnnotationGroup {
public:
                            __AnnotationGroup( const U2DataId &featureId,
                                FeaturesTableObject *parentObject );
                            ~__AnnotationGroup( );

    static bool             isValidGroupName( const QString &name, bool pathMode );

    void                    findAllAnnotationsInGroupSubTree( QList<__Annotation> &set ) const;

    QList<__Annotation>     getAnnotations( ) const;

    void                    removeAnnotation( const __Annotation &a );

    void                    removeAnnotations( const QList<__Annotation> &annotations );

    QList<__AnnotationGroup> getSubgroups( ) const;

    void                    removeSubgroup( __AnnotationGroup &g );

<<<<<<< HEAD
    QString                 getName( ) const;

    void                    setName( const QString &newName );
=======
    QString                 getGroupName( ) const;

    void                    setGroupName( const QString &newName );
>>>>>>> some functions of U2FeatureUtils separated for annotations and annotation groups. also consistency checks were added

    QString                 getGroupPath( ) const;

    FeaturesTableObject *   getGObject( ) const;

    U2DataId                getId( ) const;

    __AnnotationGroup       getParentGroup( ) const;

    __AnnotationGroup       getSubgroup( const QString &path, bool create );

    void                    getSubgroupPaths( QStringList &res ) const;
    /**
     * Removes all references to subgroups and annotations.
     */
    void                    clear( );

    int                     getGroupDepth() const;

    bool                    isParentOf( const __AnnotationGroup &g ) const;
    /**
     * Returns "true" if the parent of the group is root group
     */
    bool                    isTopLevelGroup( ) const;

    static const QString    ROOT_GROUP_NAME;
    static const QChar      GROUP_PATH_SEPARATOR;

 private:
    U2DataId                featureId;
    FeaturesTableObject *   parentObject;
};

} // namespace U2

#endif // 
