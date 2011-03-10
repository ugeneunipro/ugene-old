#ifndef _U2_GOBJECT_UTILS_H_
#define _U2_GOBJECT_UTILS_H_

#include "GObjectTypes.h"
#include <U2Core/GObject.h>

namespace U2 {

class AnnotationTableObject;
class Annotation;
class DNASequenceObject;
class DNATranslation;
class DNASequence;
class GUrl;

class U2CORE_EXPORT GObjectUtils {
public:

    static QList<GObject*>  select(const QList<GObject*>& objects, GObjectType type, UnloadedObjectFilter f);

    static GObject*         selectOne(const QList<GObject*>& objects, GObjectType type, UnloadedObjectFilter f);

    static QList<GObject*>  findAllObjects(UnloadedObjectFilter f, GObjectType t = GObjectType());

    // select objects from 'fromObjects' 
    // that are referenced by relations stored in 'obj' with 'relationRole' and 'type'
    static QSet<GObject*>   selectRelations(GObject* obj, GObjectType type, const QString& relationRole, 
                                        const QList<GObject*>& fromObjects, UnloadedObjectFilter f);

    static QList<GObject*>  findObjectsRelatedToObjectByRole(const GObject* obj, GObjectType resultObjType, 
                                        const QString& relationRole, const QList<GObject*>& fromObjects, UnloadedObjectFilter f);

    // selects objects from 'fromObjects' with filter 'f' that have relation(relationRole, type)
    // 'availableObjectsOnly'  -> check if the related object is in project
    static QList<GObject*>  selectObjectsWithRelation(const QList<GObject*>& fromObjects, GObjectType type, 
                                        const QString& relationRole, UnloadedObjectFilter f, bool availableObjectsOnly);

    static GObject*         selectObjectByReference(const GObjectReference& r, const QList<GObject*>& fromObjects, UnloadedObjectFilter f);

    static GObject*         selectObjectByReference(const GObjectReference& r, UnloadedObjectFilter f);

    static DNATranslation*  findComplementTT(DNASequenceObject* so);

    static DNATranslation*  findAminoTT(DNASequenceObject* so, bool fromHintsOnly, const QString& table = NULL);

    static DNATranslation*  findBackTranslationTT(DNASequenceObject* so, const QString& table = NULL);

    //checks object type for both loaded and unloaded states
    static bool             hasType(GObject* obj, const GObjectType& type);

    static void             updateRelationsURL(GObject* o, const QString& fromURL, const QString& toURL);

    static void             updateRelationsURL(GObject* o, const GUrl& fromURL, const GUrl& toURL);
    
    // checks value of "SPLIT" qualifier for circular DNA
    static bool             annotationHasNegativeSplit(Annotation* a);

    static void             replaceAnnotationQualfier(Annotation* a, const QString& name, const QString& newVal, bool create = false);

};


} // namespace

#endif
