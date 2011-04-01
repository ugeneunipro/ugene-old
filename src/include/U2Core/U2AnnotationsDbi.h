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

#ifndef _U2_ANNOTATION_DBI_H_
#define _U2_ANNOTATION_DBI_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2Annotation.h>

namespace U2 {


/**
    An interface to obtain 'read' access to sequence annotations
*/
class U2CORE_EXPORT U2AnnotationDbi : public U2ChildDbi {
protected:
    U2AnnotationDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi){}

public:
    /** 
        Returns number of annotations for the given sequence object  that belongs to the group specified
        If group name is empty - all annotations are counted
    */
    virtual qint64 countAnnotations(const U2DataId& sequenceId, const QString& group,  U2OpStatus& os) = 0;
    
    /** 
        Returns number of annotations for the given sequence object in the given region.
        Counts all annotations whose location intersects the region.
    */
    virtual qint64 countAnnotations(const U2DataId& sequenceId, const U2Region& region, U2OpStatus& os) = 0;

    /** 
        Returns annotations for the given sequence object that belongs to the group specified
        If group is empty searches globally in all groups
        Orders result by qualifier if not empty
        The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. 
    */
    virtual QList<U2DataId> getAnnotations(const U2DataId& sequenceId, const QString& group, const QString& orderByQualifier,
                                            qint64 offset, qint64 count, U2OpStatus& os) = 0;
    
    /** 
        Returns annotations for the given sequence object in the given region.
        Counts all annotations whose location intersects the region.
        Orders result by qualifier if not empty.
        The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. 
    */
    virtual QList<U2DataId> getAnnotations(const U2DataId& sequenceId, const U2Region& region, const QString& orderByQualifier, 
                                            qint64 offset, qint64 count, U2OpStatus& os) = 0;

    /** Reads annotation entity by id */
    virtual U2Annotation getAnnotation(const U2DataId& annotationId, U2OpStatus& os) = 0;


    /** 
        Adds new annotation. Assigns Id to annotation 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void createAnnotation(U2Annotation& a, U2OpStatus& os) = 0;

    /** 
        Adds list of new annotations. Assigns Ids to annotations added 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void createAnnotations(QList<U2Annotation>& annotations, U2OpStatus& os) = 0;

    
    /** 
        Removes annotation from database 
        Requires: U2DbiFeature_WriteAnnotation feature support
     */
    virtual void removeAnnotation(const U2DataId& annotationId, U2OpStatus& os) = 0;

    /**  
        Removes annotations from database  
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void removeAnnotations(const QList<U2DataId>& annotationIds, U2OpStatus& os) = 0;
    
    /** 
        Changes annotations location 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void updateLocation(const U2DataId& annotationId, const U2Location& location, U2OpStatus& os) = 0;  
    
    /** 
        Changes annotations name 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void updateName(const U2DataId& annotationId, const QString& newName, U2OpStatus& os) = 0;  
    
    /** 
        Adds new qualifier to annotation  
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void createQualifier(const U2DataId& annotationId, const U2Qualifier& q, U2OpStatus& os) = 0;
    
    /** 
        Removes existing qualifier from annotation  
        Requires: U2DbiFeature_WriteAnnotation feature support
     */    
    virtual void removeQualifier(const U2DataId& annotationId, const U2Qualifier& q, U2OpStatus& os) = 0; 

    /** 
        Adds annotation to the specified group 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */    
    virtual void addToGroup(const U2DataId& annotationId, const QString& group, U2OpStatus& os) = 0; 
    
    /** 
        Removes annotation from the specified group 
        If annotation belongs to no group, it is removed
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void removeFromGroup(const U2DataId& annotationId, const QString& group, U2OpStatus& os) = 0; 
};

} //namespace

#endif
