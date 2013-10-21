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

#include "SequenceUtils.h"

#include <U2Core/DNATranslation.h>
#include <U2Core/TextUtils.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/ProjectModel.h>

namespace U2 {

QList<QByteArray> U1SequenceUtils::translateRegions(const QList<QByteArray>& origParts, DNATranslation* aminoTT, bool join) {
    QList<QByteArray> resParts;
    assert(aminoTT != NULL);
    if (join) {
        resParts.append(U1SequenceUtils::joinRegions(origParts));
    }  else {
        resParts.append(origParts);
    }
    for (int i = 0, n = resParts.length(); i < n; i++) {
        const QByteArray& d = resParts[i];
        int translatedLen = d.size() / 3;
        QByteArray translated(translatedLen, '?');
        aminoTT->translate(d.constData(), d.length(), translated.data(), translatedLen);
        resParts[i] = translated;
    }
    return resParts;
}

static QList<QByteArray> _extractRegions(const QByteArray& seq, const QVector<U2Region>& regions, DNATranslation* complTT) 
{
    QList<QByteArray> res;

    QVector<U2Region> safeLocation = regions;
    U2Region::bound(0, seq.size(), safeLocation);

    for (int i = 0, n = safeLocation.size(); i < n; i++) {
        const U2Region& oReg = safeLocation.at(i);
        if (complTT == NULL) {
            QByteArray part = seq.mid(oReg.startPos, oReg.length);
            res.append(part);
        } else {
            QByteArray arr = seq.mid(oReg.startPos, oReg.length);
            TextUtils::reverse(arr.data(), arr.length());
            complTT->translate(arr.data(), arr.length());
            res.prepend(arr);
        }
    }
    return res;
}

QList<QByteArray> U1SequenceUtils::extractRegions(const QByteArray& seq, const QVector<U2Region>& origLocation, 
                                                DNATranslation* complTT, DNATranslation* aminoTT, bool circular, bool join)
{
    QList<QByteArray> res = _extractRegions(seq, origLocation, complTT);
    if (circular && res.size() > 1) {
        const U2Region& firstL = origLocation.first();
        const U2Region& lastL = origLocation.last();
        if (firstL.startPos == 0 && lastL.endPos() == seq.size()) { 
            QByteArray lastS = res.last();
            QByteArray firstS = res.first();
            res.removeLast();
            res[0] = lastS.append(firstS);
        }
    }
    if (aminoTT != NULL) {
        res = translateRegions(res, aminoTT, join);
    } 

    if (join && res.size() > 1) {
        QByteArray joined = joinRegions(res);
        res.clear();
        res.append(joined);
    }

    return res;
}


QVector<U2Region> U1SequenceUtils::getJoinedMapping(const QList<QByteArray>& seqParts) {
    QVector<U2Region>  res;
    int prevEnd = 0;
    foreach(const QByteArray& seq, seqParts) {
        res.append(U2Region(prevEnd, seq.size()));
        prevEnd += seq.size();
    }
    return res;
}


static void reorderingObjects(QList<GObject* >& objs){
    if(objs.size() >= 2){ // ordering the object. Ahead of objects is sequence and annotations are behind.
        for(int i = 0; i < objs.size(); ++i){
            if(objs.at(i)->getGObjectType() == GObjectTypes::SEQUENCE){
                objs.push_front(objs.at(i));
                objs.removeAt(i + 1);
            }
        }
    }
}

static QString getSuffixByAlphabet(const DNAAlphabetType& alpType){
    QString suffix;
    switch(alpType){
                case DNAAlphabet_AMINO: suffix = "amino";
                    break;
                case DNAAlphabet_NUCL : suffix = "dna";
                    break;
                case DNAAlphabet_RAW : suffix = "raw";
                    break;
                default:
                    assert(false && "Unknown alphabet");
    }
    return "_" + suffix;
}

static bool isGenbankHeaderUsed(const QVariantMap& hints, const QString& urlGenbank){
    return hints.value(RawDataCheckResult_HeaderSequenceLength + urlGenbank, -1) != -1;
}

static U2SequenceObject* storeSequenceUseGenbankHeader(const QVariantMap& hints, const QString& urlGenbank, const QString& seqName, U2OpStatus& os){
    qint64 sequenceLength = hints[RawDataCheckResult_HeaderSequenceLength + urlGenbank].toLongLong();
    TmpDbiHandle dbiHandle(SESSION_TMP_DBI_ALIAS, os);
    CHECK_OP(os, NULL);

    U2SequenceImporter seqImport;
    seqImport.startSequence(dbiHandle.getDbiRef(), seqName, false, os);
    CHECK_OP(os, NULL);

    QByteArray symbolsOfNotExistingSequence(sequenceLength, 'N');

    seqImport.addBlock(symbolsOfNotExistingSequence.data(), sequenceLength, os);
    CHECK_OP(os, NULL);

    U2Sequence u2seq = seqImport.finalizeSequence(os);
    CHECK_OP(os, NULL);

    return new U2SequenceObject(u2seq.visualName, U2EntityRef(dbiHandle.getDbiRef(), u2seq.id));
}

//TODO move to AnnotationUtils ?
static void shiftAnnotations(AnnotationTableObject* newAnnObj, QList<AnnotationTableObject*> annObjects, const U2Region& contigReg){
    SharedAnnotationData ad(new AnnotationData());
    ad->name = "contig";
    ad->location->regions << contigReg;
    newAnnObj->addAnnotation(new Annotation(ad));

    foreach(AnnotationTableObject* annObj, annObjects) {
        foreach(Annotation* a, annObj->getAnnotations()) {
            Annotation* newAnnotation = new Annotation(a->data());
            U2Location newLocation = newAnnotation->getLocation();
            U2Region::shift(contigReg.startPos, newLocation->regions);
            newAnnotation->setLocation(newLocation);
            QStringList groupNames;
            foreach(AnnotationGroup* g, a->getGroups()) {
                groupNames << g->getGroupName();
            }
            newAnnObj->addAnnotation(newAnnotation, groupNames);
        }
    }    
}

static void importGroupSequences2newObject(const QList<U2SequenceObject*>& seqObjects, AnnotationTableObject* newAnnObj, int mergeGap, U2SequenceImporter& seqImport, 
                                           const QHash< U2SequenceObject*, QList<AnnotationTableObject*> >& annotationsBySequenceObjectName, U2OpStatus& os)
{
    qint64 currentSeqLen = 0;

    foreach(U2SequenceObject* seqObj, seqObjects) {
        if (currentSeqLen > 0) {
            seqImport.addDefaultSymbolsBlock(mergeGap, os);
            CHECK_OP(os, );
            currentSeqLen+=mergeGap;
        }
        U2Region contigReg(currentSeqLen, seqObj->getSequenceLength());
        currentSeqLen+=seqObj->getSequenceLength();
        seqImport.addSequenceBlock(seqObj->getSequenceRef(), U2_REGION_MAX, os);
        CHECK_OP(os, );
       
        // now convert all annotations;
        shiftAnnotations(newAnnObj, annotationsBySequenceObjectName.value(seqObj), contigReg);        
    }
}

void processOldObjects(const QList<GObject* > & objs, 
                       QHash< U2SequenceObject*, QList<AnnotationTableObject*> >& annotationsBySequenceObjectName, 
                       QMap<DNAAlphabetType, QList<U2SequenceObject*> >& mapObjects2Alphabets, const QString& url, const QString& fileName, const QVariantMap& hints, U2OpStatus& os)
{
    U2SequenceObject* seqObj = NULL;        
    int currentObject = -1;

    foreach(GObject* obj, objs) {
        currentObject++;
        AnnotationTableObject* annObj = qobject_cast<AnnotationTableObject*>(obj);
        if (annObj == NULL) {
            seqObj = qobject_cast<U2SequenceObject*>(obj);
            CHECK_EXT(seqObj != NULL, os.setError("No sequence and annotations are found"), );
            const DNAAlphabet* seqAl = seqObj->getAlphabet(); 
            mapObjects2Alphabets[seqAl->getType()].append(seqObj);

            continue;
        }
        // GENBANK without sequence but header have sequence length - made sequence with 'N' characters
        if(currentObject == 0 && isGenbankHeaderUsed(hints, url)){ 
            U2SequenceObject*  seqObj = storeSequenceUseGenbankHeader(hints, url, fileName, os);
            GObjectReference sequenceRef(GObjectReference(url, "", GObjectTypes::SEQUENCE));
            sequenceRef.objName = seqObj->getGObjectName();
            annObj->addObjectRelation(GObjectRelation(sequenceRef, GObjectRelationRole::SEQUENCE));

            const DNAAlphabet* seqAl = seqObj->getAlphabet();
            mapObjects2Alphabets[seqAl->getType()].append(seqObj);
        }

        QList<GObjectRelation> seqRelations = annObj->findRelatedObjectsByRole(GObjectRelationRole::SEQUENCE);
        foreach(const GObjectRelation& rel, seqRelations) {
            const QString& relDocUrl = rel.getDocURL();
            if (relDocUrl == url) {
                QList<AnnotationTableObject*>& annObjs = annotationsBySequenceObjectName[seqObj];
                if (!annObjs.contains(annObj)) {
                    annObjs << annObj;
                }
            }
        }
    }
}

static QList<GObject*> createNewObjects(const QHash< U2SequenceObject*, QList<AnnotationTableObject*> >& annotationsBySequenceObjectName, 
                                        const QMap<DNAAlphabetType, QList<U2SequenceObject*> >& mapObjects2Alpabets,
                                        const U2DbiRef& ref, const GUrl& newUrl, QVariantMap& hints, int mergeGap, U2OpStatus& os)
{
    QList<GObject*> objects;
    // Creating sequence object for group sequence with the same alphabets. Amount of different alphabets = amount of sequence objects
    bool init = false;
    for(QMap<DNAAlphabetType, QList<U2SequenceObject*> >::const_iterator it = mapObjects2Alpabets.begin(); it != mapObjects2Alpabets.end(); ++it){
        U2SequenceImporter seqImport;
        QString seqName = newUrl.fileName() ;
        if(mapObjects2Alpabets.size() > 1){
            seqName += getSuffixByAlphabet(it.key());
            if(!init){
                hints[ProjectLoaderHint_MergeMode_DifferentAlphabets] = QString(QObject::tr("Loaded sequences have different alphabets. "
                    "That's why several sequence objects are created, each for specified alphabet. All sequences at one object have the same alphabet "
                    "and sequences from different objects have different alphabets."));
                init = true;
            }
        }
        seqImport.startSequence(ref, seqName, false, os);
        CHECK_OP(os, QList<GObject*>());

        AnnotationTableObject* newAnnObj = new AnnotationTableObject(seqName + " annotations");
        QList<U2SequenceObject*> seqObjects = it.value();
        importGroupSequences2newObject(seqObjects, newAnnObj, mergeGap, seqImport, annotationsBySequenceObjectName, os);

        U2Sequence u2seq = seqImport.finalizeSequence(os);
        CHECK_OP(os, QList<GObject*>());

        if (!newUrl.getURLString().isEmpty()) {
            GObjectReference r(newUrl.getURLString(), u2seq.visualName, GObjectTypes::SEQUENCE);
            newAnnObj->addObjectRelation(GObjectRelation(r, GObjectRelationRole::SEQUENCE));
        }

        U2SequenceObject* seqObj = new U2SequenceObject(u2seq.visualName, U2EntityRef(ref, u2seq.id));    
        objects << seqObj << newAnnObj ;
    }
    return objects;
}

QList<GObject*> U1SequenceUtils::mergeSequences(const QList<Document*> docs, const U2DbiRef& ref, const QString& newStringUrl, QVariantMap& hints, U2OpStatus& os) {
    // prepare  annotation object -> sequence object mapping first
    // and precompute resulted sequence size and alphabet
    int mergeGap = hints[DocumentReadingMode_SequenceMergeGapSize].toInt();
    assert(mergeGap >= 0);
    QHash< U2SequenceObject*, QList<AnnotationTableObject*> > annotationsBySequenceObjectName;
    GUrl newUrl(newStringUrl, GUrl_File);

    QMap<DNAAlphabetType, QList<U2SequenceObject*> > mapObjects2Alpabets;

    foreach(const Document* doc, docs){        
        QList<GObject* > objs = doc->getObjects();
        reorderingObjects(objs);        
        processOldObjects(objs, annotationsBySequenceObjectName, mapObjects2Alpabets, doc->getURLString(), doc->getURL().fileName(), hints, os);        
        CHECK_OP(os, QList<GObject*>());
    }
    CHECK_OP(os, QList<GObject*>());
    return createNewObjects(annotationsBySequenceObjectName, mapObjects2Alpabets, ref, newUrl, hints, mergeGap, os);            
}

QList<GObject*> U1SequenceUtils::mergeSequences(Document* doc, const U2DbiRef& ref, QVariantMap& hints, U2OpStatus& os){
    QList<Document* > docs; docs << doc;
    return mergeSequences(docs, ref, doc->getURLString(), hints, os);
} 

QByteArray U1SequenceUtils::joinRegions(const QList<QByteArray>& parts) {
    if (parts.size() == 1) {
        return parts.first();
    }
    int size =0;
    foreach(const QByteArray& p, parts) {
        size += p.size();
    }
    QByteArray res;
    res.reserve(size);
    foreach(const QByteArray& p, parts) {
        res.append(p);
    }
    return res;
}




}//namespace
