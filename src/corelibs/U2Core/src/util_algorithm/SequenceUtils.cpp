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

#include "SequenceUtils.h"

#include <U2Core/DNATranslation.h>
#include <U2Core/TextUtils.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/GObjectRelationRoles.h>

namespace U2 {

QList<QByteArray> SequenceUtils::extractRegions(const QByteArray& seq, const QVector<U2Region>& regions, DNATranslation* complTT) {
    QVector<U2Region> safeLocation = regions;
    U2Region::bound(0, seq.length(), safeLocation);

    QList<QByteArray> resParts;
    for (int i = 0, n = safeLocation.size(); i < n; i++) {
        const U2Region& oReg = safeLocation.at(i);
        if (complTT == NULL) {
            resParts.append(seq.mid(oReg.startPos, oReg.length));
        } else {
            QByteArray arr = seq.mid(oReg.startPos, oReg.length);
            TextUtils::reverse(arr.data(), arr.length());
            complTT->translate(arr.data(), arr.length());
            resParts.prepend(arr);
        }
    }
    return resParts;
}

QByteArray SequenceUtils::joinRegions(const QList<QByteArray>& parts) {
    if (parts.size() == 1) {
        return parts.first();
    }
    QByteArray res;
    foreach(const QByteArray& p, parts) {
        res.append(p);
    }
    return res;
}

QList<QByteArray> SequenceUtils::translateRegions(const QList<QByteArray>& origParts, DNATranslation* aminoTT, bool join) {
    QList<QByteArray> resParts;
    assert(aminoTT != NULL);
    if (join) {
        resParts.append(SequenceUtils::joinRegions(origParts));
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


QList<QByteArray> SequenceUtils::extractSequence(const QByteArray& seq, const QVector<U2Region>& origLocation, 
                                         DNATranslation* complTT, DNATranslation* aminoTT, bool join, bool circular)
{
    QList<QByteArray> resParts = extractRegions(seq, origLocation, complTT);
    if (circular && resParts.size() > 1) {
        const U2Region& firstL = origLocation.first();
        const U2Region& lastL = origLocation.last();
        if (firstL.startPos == 0 && lastL.endPos() == seq.length()) { 
            QByteArray lastS = resParts.last();
            QByteArray firstS = resParts.first();
            resParts.removeLast();
            resParts[0] = lastS.append(firstS);
        }
    }
    if (aminoTT != NULL) {
        resParts = translateRegions(resParts, aminoTT, join);
    } else if (join) {
        QByteArray joined = joinRegions(resParts);
        resParts.clear();
        resParts.append(joined);
    }
    return resParts;
}


QVector<U2Region> SequenceUtils::toJoinedRegions(const QList<QByteArray>& seqParts) {
    QVector<U2Region>  res;
    int prevEnd = 0;
    foreach(const QByteArray& seq, seqParts) {
        res.append(U2Region(prevEnd, seq.size()));
        prevEnd += seq.size();
    }
    return res;
}


Document* SequenceUtils::mergeSequences(const Document* doc, int mergeGap, U2OpStatus& os) {
    // prepare  annotation object -> sequence object mapping first
    // and precompute resulted sequence size and alphabet
    int mergedSize = 0;
    QHash< QString, QList<AnnotationTableObject*> > annotationsBySequenceObjectName;
    QList<DNASequenceObject*> seqObjects;
    QString docUrl = doc->getURLString();
    DNAAlphabet* al = NULL;
    foreach(GObject* obj, doc->getObjects()) {
        AnnotationTableObject* annObj = qobject_cast<AnnotationTableObject*>(obj);
        if (annObj == NULL) {
            DNASequenceObject* seqObj = qobject_cast<DNASequenceObject*>(obj);
            if (seqObj != NULL) {
                seqObjects << seqObj;
                mergedSize += mergedSize == 0 ? 0 : mergeGap;
                mergedSize += seqObj->getSequenceLen();
                DNAAlphabet* seqAl = seqObj->getAlphabet(); 
                al = (al == NULL) ?  seqAl : DNAAlphabet::deriveCommonAlphabet(al, seqAl);
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
    if (os.hasError()) {
        return NULL;
    }
    if (seqObjects.isEmpty()) {
        return NULL;
    }
    if (mergedSize > 1000*1000*1000) { //2Gb (max qbytearray size) / 2
        os.setError(tr("Not enough memory to complete operation!"));
        return NULL;
    }
    DNASequence seq(doc->getURL().fileName(), QByteArray(), al);
    seq.seq.reserve(mergedSize); //TODO: check if memory op succeed!!!
    AnnotationTableObject* annObj = new AnnotationTableObject(seq.getName() + " annotations");
    QByteArray delim(mergeGap, al->getDefaultSymbol());
    foreach(DNASequenceObject* seqObj, seqObjects) {
        if (!seq.seq.isEmpty()) {
            seq.seq.append(delim);
        }
        U2Region contigReg(seq.seq.length(), seqObj->getSequenceLen());
        seq.seq.append(seqObj->getSequence());

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
    DNASequenceObject* seqObj = new DNASequenceObject(seq.getName(), seq);
    QList<GObject*> objects; objects << seqObj << annObj;
    QVariantMap hints;
    Document* resultDoc = new Document(doc->getDocumentFormat(), doc->getIOAdapterFactory(), doc->getURL(), objects, hints, tr("File content was merged"));
    doc->propagateModLocks(resultDoc);
    return resultDoc;
}

}//namespace
