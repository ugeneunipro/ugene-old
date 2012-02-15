/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_SQLITE_ANNOTATION_DBI_H_
#define _U2_SQLITE_ANNOTATION_DBI_H_

#include "SQLiteDbi.h"

#include <U2Core/U2AnnotationDbi.h>

namespace U2 {

class SQLiteAnnotationDbi : public U2AnnotationDbi, public SQLiteChildDBICommon {
public:

    SQLiteAnnotationDbi(SQLiteDbi* dbi);

    virtual void initSqlSchema(U2OpStatus& os);

    /** 
        Returns group by path. Path construction algorithm: parent-name2 + "/" + parent-name1 +  "/" + ... + "/" + groupName 
    */
    virtual U2AnnotationGroup getGroupByPath(const U2DataId& sequenceId, const QString& path, U2OpStatus& os);
    
    /** Returns all subgroups of the given group */
    virtual QList<U2AnnotationGroup> getSubgroups(const U2DataId& groupId, U2OpStatus& os);

        /** Reads annotation group by id */
    virtual U2AnnotationGroup getGroup(const U2DataId& groupId, U2OpStatus& os);
    
    /**
        Creates new group. If group already exists - returns existing instance
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual U2AnnotationGroup createGroup(const U2DataId& sequenceId, const QString& path, U2OpStatus& os);

    /**
        Removes group and all child data
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void removeGroup(const U2DataId& groupId, U2OpStatus& os);

    /** 
        Returns number of annotations for the given sequence object in the given region.
        Counts all annotations whose location intersects the region.
        If region is [0, U2_DBI_NO_LIMIT] - returns all annotation for the given sequence
    */
    virtual qint64 countSequenceAnnotations(const U2DataId& sequenceId, const U2Region& region, U2OpStatus& os);

    /** 
        Returns number of annotations in the given sequence group object in the given region.
        Counts all annotations whose location intersects the region.
        If region is [0, U2_DBI_NO_LIMIT] - returns all annotation for the given sequence
    */
    virtual qint64 countGroupAnnotations(const U2DataId& groupId, const U2Region& region, U2OpStatus& os);

    /** 
        Returns annotations for the given sequence object
        'Offset' and 'count' can be used to change the default position and number of results produced by iterator
        Note: 'count' is a hint here while 'offset' is obligatory to support
    */
    virtual U2DbiIterator<U2DataId>* getAnnotationsBySequence(const U2DataId& sequenceId, const U2Region& region, 
        qint64 offset, qint64 count, U2OpStatus& os);
    

    /** 
        Returns annotations for the given group
        'Offset' and 'count' can be used to change the default position and number of results produced by iterator
        Note: 'count' is a hint here while 'offset' is obligatory to support
    */
    virtual U2DbiIterator<U2DataId>* getAnnotationsByGroup(const U2DataId& groupId, const U2Region& region, 
        qint64 offset, qint64 count, U2OpStatus& os);


    /** Reads annotation entity by id */
    virtual U2Annotation getAnnotation(const U2DataId& annotationId, U2OpStatus& os);

    /** Returns groups this annotation belongs to */
    virtual QList<U2DataId> getAnnotationGroups(const U2DataId& annotationId, U2OpStatus& os) ;

    /** 
        Adds list of new annotations. Assigns Ids to annotations added 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void createAnnotations(QList<U2Annotation>& annotations, const U2DataId& group, U2OpStatus& os);

    
    /** 
        Removes annotation from database 
        Requires: U2DbiFeature_WriteAnnotation feature support
     */
    virtual void removeAnnotation(const U2DataId& annotationId, U2OpStatus& os);

    /**  
        Removes annotations from database  
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void removeAnnotations(const QList<U2DataId>& annotationIds, U2OpStatus& os);
    
    /** 
        Changes annotations location 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void updateLocation(const U2DataId& annotationId, const U2Location& location, U2OpStatus& os);
    
    /** 
        Changes annotations name 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void updateName(const U2DataId& annotationId, const QString& newName, U2OpStatus& os);  
    
    /** 
        Adds new qualifier to annotation  
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void createQualifier(const U2DataId& annotationId, const U2Qualifier& q, U2OpStatus& os);
    
    /** 
        Removes existing qualifier from annotation  
        Requires: U2DbiFeature_WriteAnnotation feature support
     */    
    virtual void removeQualifier(const U2DataId& annotationId, const U2Qualifier& q, U2OpStatus& os);

    /** 
        Adds annotation to the specified group 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */    
    virtual void addToGroup(const U2DataId& groupId, const U2DataId& annotationId, U2OpStatus& os);
    
    /** 
        Removes annotation from the specified group 
        Note: If annotation belongs to no group, it is automatically removed from database
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void removeFromGroup(const U2DataId& groupId, const U2DataId& annotationId, U2OpStatus& os); 


    bool isInGroup(const U2DataId& groupId, const U2DataId& annotationId, U2OpStatus& os);
};

} //namespace

#endif
