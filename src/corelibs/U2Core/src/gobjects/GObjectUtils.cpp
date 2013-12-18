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

#include "GObjectUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GHints.h>
#include <U2Core/GUrl.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/UnloadedObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

QList<GObject*> GObjectUtils::select(const QList<GObject*>& objs, GObjectType t, UnloadedObjectFilter f) {
    QList<GObject*> res;
    foreach(GObject* o, objs) {
        bool isUnloaded = o->getGObjectType() == GObjectTypes::UNLOADED;
        if ((t.isEmpty() && (f == UOF_LoadedAndUnloaded || !isUnloaded)) || o->getGObjectType() == t) {
            res.append(o);
        } else if (f == UOF_LoadedAndUnloaded && isUnloaded) {
            UnloadedObject* uo = qobject_cast<UnloadedObject*>(o);
            if (uo->getLoadedObjectType() == t) {
                res.append(o);
            }
        }
    }
    return res;
}

GObject* GObjectUtils::selectOne(const QList<GObject*>& objects, GObjectType type, UnloadedObjectFilter f) {
    QList<GObject*> res = select(objects, type, f);
    return res.isEmpty() ? NULL : res.first();
}


QList<GObject*> GObjectUtils::findAllObjects(UnloadedObjectFilter f, GObjectType t) {
    QList<GObject*> res;
    SAFE_POINT(AppContext::getProject()!=NULL, "No active project found", res);

    foreach(Document* doc, AppContext::getProject()->getDocuments()) {
        if (t.isEmpty()) {
            if (doc->isLoaded() || f == UOF_LoadedAndUnloaded) {
                res+=doc->getObjects();
            }
        } else {
            res+=doc->findGObjectByType(t, f);
        }
    }
    return res;
}


QList<GObject*> GObjectUtils::selectRelations(GObject* obj, GObjectType type, const QString& relationRole, 
                                             const QList<GObject*>& fromObjects, UnloadedObjectFilter f)
{
    QList<GObject*> res;
    QList<GObjectRelation> relations = obj->getObjectRelations();
    foreach(const GObjectRelation& r, relations) {
        if (r.role != relationRole || (!type.isEmpty() && r.ref.objType!=type)) {
            continue;
        }
        GObject* obj = selectObjectByReference(r.ref, fromObjects, f);
        if (obj!=NULL) {
            res.append(obj);
        }
    }
    return res;
}

QList<GObject*> GObjectUtils::findObjectsRelatedToObjectByRole(const GObject* obj, GObjectType resultObjType, 
                                                               const QString& role, const QList<GObject*>& fromObjects, 
                                                               UnloadedObjectFilter f) 
{
    QList<GObject*> res;
    QList<GObject*> objects  = select(fromObjects, resultObjType, f);
    foreach(GObject* o, objects) {
        if (o->hasObjectRelation(obj, role)) {
            res.append(o);
        }
    }
    return res;
}

QList<GObject*> GObjectUtils::selectObjectsWithRelation(const QList<GObject*>& objs, GObjectType type, 
                                                        const QString& relationRole, UnloadedObjectFilter f, bool availableObjectsOnly) 
{
    QSet<GObject*> availableObjects;
    if (availableObjectsOnly) {
        availableObjects = findAllObjects(f).toSet();
    }
    QList<GObject*> res;
    foreach(GObject* obj, objs) {
        QList<GObjectRelation> relations = obj->getObjectRelations();
        foreach(const GObjectRelation& r, relations) {
            if (r.role != relationRole || (!type.isEmpty() && r.ref.objType!=type)) {
                continue;
            }
            if (availableObjectsOnly) {
                Document* doc = AppContext::getProject()->findDocumentByURL(r.ref.docUrl);
                GObject* refObj = doc == NULL ? NULL : doc->findGObjectByName(r.ref.objName);
                if (refObj == NULL || (f == UOF_LoadedOnly && refObj->getGObjectType() == GObjectTypes::UNLOADED)) {
                    continue;
                }
            }
            res.append(obj);
        }
    }
    return res;
}

GObject* GObjectUtils::selectObjectByReference(const GObjectReference& r, UnloadedObjectFilter f) {
    return selectObjectByReference(r, findAllObjects(f, r.objType), f);
}

GObject* GObjectUtils::selectObjectByReference(const GObjectReference& r, const QList<GObject*>& fromObjects, UnloadedObjectFilter f) {
    foreach(GObject* o, fromObjects) {
        if (o->getGObjectName() != r.objName) {
            continue;
        }
        if ((o->getDocument() == NULL && !r.docUrl.isEmpty())
            || (o->getDocument()->getURL() != r.docUrl)) 
        {
            continue;
        }
        if (r.objType != o->getGObjectType()) {
            if (f != UOF_LoadedAndUnloaded) {
                continue;
            }
            if (o->getGObjectType()!=GObjectTypes::UNLOADED || r.objType != qobject_cast<UnloadedObject*>(o)->getLoadedObjectType()) {
                continue;
            }
        }
        return o;
    }
    return NULL;
}

DNATranslation* GObjectUtils::findComplementTT(const DNAAlphabet* al) {
    if (!al || !al->isNucleic()) {
        return NULL;
    }
    return AppContext::getDNATranslationRegistry()->lookupComplementTranslation(al);
}

DNATranslation* GObjectUtils::findAminoTT(U2SequenceObject* so, bool fromHintsOnly, const QString& table) {
    if (!so || !so->getAlphabet() || !so->getAlphabet()->isNucleic()) {
        return NULL;
    }
    DNATranslationRegistry* tr = AppContext::getDNATranslationRegistry();
    QString tid = so->getGHints()->get(AMINO_TT_GOBJECT_HINT).toString();
    if (table != NULL) {
        DNATranslation* res = tr->lookupTranslation(so->getAlphabet(), DNATranslationType_NUCL_2_AMINO, table);
        return res;
    }
    DNATranslation* res = tr->lookupTranslation(so->getAlphabet(), DNATranslationType_NUCL_2_AMINO, tid);
    if (res != NULL || fromHintsOnly) {
        return res;
    }
    QList<DNATranslation*> aminoTs = tr->lookupTranslation(so->getAlphabet(), DNATranslationType_NUCL_2_AMINO);
    if (!aminoTs.empty()) {
        res = tr->getStandardGeneticCodeTranslation(so->getAlphabet());
    }
    return res;
}

DNATranslation* GObjectUtils::findBackTranslationTT(U2SequenceObject* so, const QString& table) {

    if (!so || !so->getAlphabet() || !so->getAlphabet()->isAmino()) {
        return NULL;
    }
    DNATranslationRegistry* tr = AppContext::getDNATranslationRegistry();
    if (table != NULL) {
        DNATranslation* res = tr->lookupTranslation(so->getAlphabet(), DNATranslationType_AMINO_2_NUCL, table);
        return res;
    }
    QList<DNATranslation*> dnaTs = tr->lookupTranslation(so->getAlphabet(), DNATranslationType_AMINO_2_NUCL);
    DNATranslation* res = NULL;
    if (!dnaTs.empty()) {
        res = dnaTs.first();
    }
    return res;
}

bool GObjectUtils::hasType(GObject* obj, const GObjectType& type) {
    if (obj->getGObjectType() == type) {
        return true;
    }
    if (obj->getGObjectType() != GObjectTypes::UNLOADED) {
        return false;
    }
    UnloadedObject* uo = qobject_cast<UnloadedObject*>(obj);
    return uo->getLoadedObjectType() == type;
}


void GObjectUtils::updateRelationsURL(GObject* o, const GUrl& fromURL, const GUrl& toURL) {
    updateRelationsURL(o, fromURL.getURLString(), toURL.getURLString());
}

void GObjectUtils::updateRelationsURL(GObject* o, const QString& fromURL, const QString& toURL) {
    QList<GObjectRelation> relations = o->getObjectRelations();
    bool changed = false;
    for(int i=0; i<relations.size(); i++) {
        GObjectRelation& r = relations[i];
        if (r.ref.docUrl == fromURL) {
            r.ref.docUrl = toURL;
            changed = true;
        }
    }
    if (changed) {
        o->setObjectRelations(relations);
    }
}

void GObjectUtils::replaceAnnotationQualfier( AnnotationData &a, const QString &name,
    const QString &newVal, bool create )
{
    QVector<U2Qualifier> quals;
    a.findQualifiers( name, quals );
    
    foreach ( const U2Qualifier &q, quals ) {
        for ( int i = 0; i < quals.size( ); ++i ) {
            if ( a.qualifiers[i] == q ) {
                a.qualifiers.remove( i );
                --i;
            }
        }
    }

    if ( create || !quals.isEmpty( ) ) {
        a.qualifiers << U2Qualifier( name, newVal );
    }
}

} // namespace U2
