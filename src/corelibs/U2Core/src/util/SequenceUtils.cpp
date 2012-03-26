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

Document* U1SequenceUtils::mergeSequences(const Document* doc, int mergeGap, U2OpStatus& os) {
    // prepare  annotation object -> sequence object mapping first
    // and precompute resulted sequence size and alphabet
    int mergedSize = 0;
    QHash< QString, QList<AnnotationTableObject*> > annotationsBySequenceObjectName;
    QList<U2SequenceObject*> seqObjects;
    QString docUrl = doc->getURLString();
    DNAAlphabet* al = NULL;
    foreach(GObject* obj, doc->getObjects()) {
        AnnotationTableObject* annObj = qobject_cast<AnnotationTableObject*>(obj);
        if (annObj == NULL) {
            U2SequenceObject* seqObj = qobject_cast<U2SequenceObject*>(obj);
            if (seqObj != NULL) {
                seqObjects << seqObj;
                mergedSize += mergedSize == 0 ? 0 : mergeGap;
                mergedSize += seqObj->getSequenceLength();
                DNAAlphabet* seqAl = seqObj->getAlphabet(); 
                al = (al == NULL) ?  seqAl : U2AlphabetUtils::deriveCommonAlphabet(al, seqAl);
                if (al == NULL) {
                    os.setError(tr("Failed to derive common alphabet!"));
                    break;
                }
            }
            continue;
        }
        QList<GObjectRelation> seqRelations = annObj->findRelatedObjectsByRole(GObjectRelationRole::SEQUENCE);
        foreach(const GObjectRelation& rel, seqRelations) {
            const QString& relDocUrl = rel.getDocURL();
            if (relDocUrl == docUrl) {
                QList<AnnotationTableObject*>& annObjs = annotationsBySequenceObjectName[rel.ref.objName];
                if (!annObjs.contains(annObj)) {
                    annObjs << annObj;
                }
            }
        }
    }
    CHECK_OP(os, NULL);
    CHECK(!seqObjects.isEmpty(), NULL);

    TmpDbiHandle dbiHandle(SESSION_TMP_DBI_ALIAS, os);
    CHECK_OP(os, NULL);
    U2SequenceImporter seqImport;
    QString seqName = doc->getURL().fileName();
    seqImport.startSequence(dbiHandle.getDbiRef(), seqName, false, os);
    CHECK_OP(os, NULL);

    AnnotationTableObject* annObj = new AnnotationTableObject(seqName + " annotations");
    QByteArray delim(mergeGap, al->getDefaultSymbol());
    qint64 currentSeqLen = 0;
    foreach(U2SequenceObject* seqObj, seqObjects) {
        if (currentSeqLen > 0) {
            seqImport.addBlock(delim.constData(), delim.size(), os);
            CHECK_OP(os, NULL);
            currentSeqLen+=delim.size();
        }
        U2Region contigReg(currentSeqLen, seqObj->getSequenceLength());
        seqImport.addSequenceBlock(seqObj->getSequenceRef(), U2_REGION_MAX, os);
        CHECK_OP(os, NULL);

        SharedAnnotationData ad(new AnnotationData());
        ad->name = "contig";
        ad->location->regions << contigReg;
        annObj->addAnnotation(new Annotation(ad));

        // now convert all annotations;
        QList<AnnotationTableObject*> annObjects = annotationsBySequenceObjectName.value(seqObj->getGObjectName());
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
                annObj->addAnnotation(newAnnotation, groupNames);
            }
        }
    }
    U2Sequence u2seq = seqImport.finalizeSequence(os);
    TmpDbiObjects dbiObjects(dbiHandle.getDbiRef(), os);
    dbiObjects.objects << u2seq.id;
    CHECK_OP(os, NULL);
    U2SequenceObject* seqObj = new U2SequenceObject(u2seq.visualName, U2EntityRef(dbiHandle.getDbiRef(), u2seq.id));
    QList<GObject*> objects; objects << seqObj << annObj;
    Document* resultDoc = new Document(doc->getDocumentFormat(), doc->getIOAdapterFactory(), doc->getURL(), 
        dbiHandle.getDbiRef(), objects, QVariantMap(), tr("File content was merged"));
    doc->propagateModLocks(resultDoc);
    return resultDoc;
}

Document* U1SequenceUtils::mergeSequences(const QList<Document*> docs,const QVariantMap& hints, U2OpStatus& os) {
    // prepare  annotation object -> sequence object mapping first
    // and precompute resulted sequence size and alphabet
    int mergeGap = hints[DocumentReadingMode_SequenceFilesMergeGapSize].toInt(); 
    const QString& newStringUrl = hints[ProjectLoaderHint_MergeMode_URLDocument].toString();
    DocumentFormat* df= AppContext::getDocumentFormatRegistry()->getFormatById(hints[ProjectLoaderHint_MergeMode_RealDocumentFormat].toString());

    int mergedSize = 0;
    QHash< U2SequenceObject*, QList<AnnotationTableObject*> > annotationsBySequenceObjectName;
    QList<U2SequenceObject*> seqObjects;
    DNAAlphabet* al = NULL;
    
    GUrl newUrl(newStringUrl, GUrl_File);

    QList <GObjectRelation> tmpRelations;

    foreach(const Document* doc, docs){
        U2SequenceObject* _seqObj = NULL;
        foreach(GObject* obj, doc->getObjects()) {
            AnnotationTableObject* annObj = qobject_cast<AnnotationTableObject*>(obj);
            if (annObj == NULL) {
                U2SequenceObject* seqObj = qobject_cast<U2SequenceObject*>(obj);
                _seqObj = seqObj;
                if (seqObj != NULL) {
                    seqObjects << seqObj;
                    mergedSize += mergedSize == 0 ? 0 : mergeGap;
                    mergedSize += seqObj->getSequenceLength();
                    DNAAlphabet* seqAl = seqObj->getAlphabet(); 
                    al = (al == NULL) ?  seqAl : U2AlphabetUtils::deriveCommonAlphabet(al, seqAl);
                    if (al == NULL) {
                        os.setError(tr("Failed to derive common alphabet!"));
                        break;
                    }
                }
                continue;
            }
            QList<GObjectRelation> seqRelations = annObj->findRelatedObjectsByRole(GObjectRelationRole::SEQUENCE);
            foreach(const GObjectRelation& rel, seqRelations) {
                const QString& relDocUrl = rel.getDocURL();
                if (relDocUrl == doc->getURLString()) {
                    QList<AnnotationTableObject*>& annObjs = annotationsBySequenceObjectName[_seqObj];
                    if (!annObjs.contains(annObj)) {
                        annObjs << annObj;
                    }
                }
            }
	    }
	}
    CHECK_OP(os, NULL);
    CHECK(!seqObjects.isEmpty(), NULL);

    TmpDbiHandle dbiHandle(SESSION_TMP_DBI_ALIAS, os);
    CHECK_OP(os, NULL);
    U2SequenceImporter seqImport;
    QString seqName = newUrl.fileName();
    seqImport.startSequence(dbiHandle.getDbiRef(), seqName, false, os);
    CHECK_OP(os, NULL);

    AnnotationTableObject* newAnnObj = new AnnotationTableObject(seqName + " annotations");
    QByteArray delim(mergeGap, al->getDefaultSymbol());
    qint64 currentSeqLen = 0;
    foreach(U2SequenceObject* seqObj, seqObjects) {
        if (currentSeqLen > 0) {
            seqImport.addBlock(delim.constData(), delim.size(), os);
            CHECK_OP(os, NULL);
            currentSeqLen+=delim.size();
        }
        U2Region contigReg(currentSeqLen, seqObj->getSequenceLength());
        currentSeqLen+=seqObj->getSequenceLength();
        seqImport.addSequenceBlock(seqObj->getSequenceRef(), U2_REGION_MAX, os);
        CHECK_OP(os, NULL);


		// now convert all annotations;

        QList<AnnotationTableObject*> annObjects = annotationsBySequenceObjectName.value(seqObj);
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

    U2Sequence u2seq = seqImport.finalizeSequence(os);
    CHECK_OP(os, NULL);

    if (!newUrl.getURLString().isEmpty()) {
        GObjectReference r(newUrl.getURLString(), u2seq.visualName, GObjectTypes::SEQUENCE);
        newAnnObj->addObjectRelation(GObjectRelation(r, GObjectRelationRole::SEQUENCE));
    }

    U2SequenceObject* seqObj = new U2SequenceObject(u2seq.visualName, U2EntityRef(dbiHandle.getDbiRef(), u2seq.id));
    QList<GObject*> objects; objects << seqObj << newAnnObj ;
    Document* resultDoc = new Document(df , docs[0]->getIOAdapterFactory(), newUrl, 
		dbiHandle.getDbiRef(), objects, hints, tr("Files content was merged"));
    return resultDoc;
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
