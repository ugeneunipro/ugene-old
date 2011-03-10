#ifndef _U2_ANNOBJ_DBI_H_
#define _U2_ANNOBJ_DBI_H_

#include <U2Core/U2Dbi.h>

namespace U2 {

class DNASequenceObject;
class AnnotationTableObject;
class Annotation;

class FileDbi;

/**
An interface to obtain 'write' access to sequence annotations
*/
class AnnotationObjectDbi : public U2AnnotationRWDbi {
protected:
    AnnotationObjectDbi(FileDbi* rootDbi);

    /** 
    Returns number of annotations for the given sequence object  that belongs to the group specified
    If group name is empty - all annotations are counted
    */
    virtual qint64 countAnnotations(U2DataId sequenceObjectId, const QString& group,  U2OpStatus& os);

    /** Returns number of annotations for the given sequence object in the given region */
    virtual qint64 countAnnotations(U2DataId sequenceObjectId, const U2Region& region, U2OpStatus& os);

    /** 
    Returns annotations for the given sequence object that belongs to the group specified
    If group is empty searches globally in all groups
    Orders result by qualifier if not empty
    */
    virtual QList<U2DataId> getAnnotations(U2DataId sequenceObjectId, const QString& group, const QString& orderByQualifier,
        qint64 offset, qint64 count, U2OpStatus& os);

    /** 
    Returns annotations for the given sequence object in the given region 
    Orders result by qualifier if not empty
    */
    virtual QList<U2DataId> getAnnotations(U2DataId sequenceObjectId, const U2Region& region, const QString& orderByQualifier, 
        qint64 offset, qint64 count, U2OpStatus& os);

    /** Reads annotation entity by id */
    virtual U2Annotation getAnnotation(U2DataId annotationId, U2OpStatus& os);

    /** Adds new annotation. Assigns Id to annotation */
    virtual void createAnnotation(U2Annotation& a, U2OpStatus& os);

    /** Adds list of new annotations. Assigns Ids to annotations added */
    virtual void createAnnotations(QList<U2Annotation>& annotations, U2OpStatus& os);


    /* Removes annotation from database */
    virtual void removeAnnotation(U2DataId annotationId, U2OpStatus& os);

    /**  Removes annotations from database  */
    virtual void removeAnnotations(const QList<U2DataId>& annotationIds, U2OpStatus& os);

    /** Changes annotations location */
    virtual U2Annotation updateLocation(U2DataId annotationId, const U2Location& location, U2OpStatus& os);  

    /** Changes annotations name */
    virtual U2Annotation updateName(U2DataId annotationId, const QString& newName, U2OpStatus& os);  

    /** Adds new qualifier to annotation  */
    virtual U2Annotation createQualifier(U2DataId annotationId, const U2Qualifier& q, U2OpStatus& os);

    /** Removes existing qualifier from annotation  */    
    virtual U2Annotation removeQualifier(U2DataId annotationId, const U2Qualifier& q, U2OpStatus& os); 

    /** Adds annotation to the specified group */    
    virtual U2Annotation addToGroup(U2DataId annotationId, const QString& group, U2OpStatus& os); 

    /** 
    Removes annotation from the specified group 
    If annotation belongs to no group, it is removed
    */
    virtual U2Annotation removeFromGroup(U2DataId annotationId, const QString& group, U2OpStatus& os); 
private:
    FileDbi* root;
    friend class FileDbi;

    QList<AnnotationTableObject*> findRelatedAnnotations(U2DataId sequenceObjectId, const QString& group,  U2OpStatus& os) const;
};
}//ns
#endif
