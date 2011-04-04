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
        Returns group by path. Path construction algorithm: parent-name2 + "/" + parent-name1 +  "/" + ... + "/" + groupName 
    */
    virtual U2AnnotationGroup getGroupByPath(const U2DataId& sequenceId, const QString& path, U2OpStatus& os) =0;
    
    /** Returns all subgroups of the given group */
    virtual QList<U2AnnotationGroup> getSubgroups(const U2DataId& groupId, U2OpStatus& os) = 0;

    /** Reads annotation group by id */
    virtual U2AnnotationGroup getGroup(const U2DataId& groupId, U2OpStatus& os) = 0;
    
    /**
        Creates new group. If group already exists - returns existing instance
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual U2AnnotationGroup createGroup(const U2DataId& sequenceId, const QString& path, U2OpStatus& os) = 0;    

    /**
        Changes group path.
        If destination group is already exists operation fails.
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual U2AnnotationGroup moveGroup(const U2DataId& groupId, const QString& newPath, U2OpStatus& os) = 0;    

    /** 
        Returns number of annotations for the given sequence object in the given region.
        Counts all annotations whose location intersects the region.
        If region is [0, U2_DBI_NO_LIMIT] - returns all annotation for the given sequence
    */
    virtual qint64 countSequenceAnnotations(const U2DataId& sequenceId, const U2Region& region, U2OpStatus& os) = 0;

    /** 
        Returns number of annotations in the given sequence group object in the given region.
        Counts all annotations whose location intersects the region.
        If region is [0, U2_DBI_NO_LIMIT] - returns all annotation for the given sequence
    */
    virtual qint64 countGroupAnnotations(const U2DataId& sequenceId, const U2Region& region, U2OpStatus& os) = 0;


    /** 
        Returns annotations for the given sequence object
        Orders result by qualifier if not empty
    */
    virtual U2DbiIterator<U2DataId> getAnnotationsBySequence(const U2DataId& sequenceId, const U2Region& region, const QString& orderByQualifier, U2OpStatus& os) = 0;
    

    /** 
        Returns annotations for the given group
        Orders result by qualifier if not empty
    */
    virtual U2DbiIterator<U2DataId> getAnnotationsByGroup(const U2DataId& sequenceId, const U2Region& region, const QString& orderByQualifier, U2OpStatus& os) = 0;

    /** Reads annotation entity by id */
    virtual U2Annotation getAnnotation(const U2DataId& annotationId, U2OpStatus& os) = 0;

    /** Returns groups this annotation belongs to */
    virtual QList<U2DataId> getAnnotationGroups(const U2DataId& annotationId, U2OpStatus& os) = 0;


    /** 
        Adds list of new annotations. Assigns Ids to annotations added 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void createAnnotations(QList<U2Annotation>& annotations, const U2DataId& group, U2OpStatus& os) = 0;

    
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
    virtual void addToGroup(const U2DataId& groupId, const U2DataId& annotationId, U2OpStatus& os) = 0; 
    
    /** 
        Removes annotation from the specified group 
        If annotation belongs to no group, it is removed
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void removeFromGroup(const U2DataId& groupId, const U2DataId& annotationId, U2OpStatus& os) = 0; 
    
};

} //namespace

#endif
