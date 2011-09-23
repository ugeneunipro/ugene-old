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

#include "DocumentFormatUtils.h"

#include <U2Core/AnnotationSettings.h>
#include <U2Core/AppContext.h>
#include <U2Core/BioStruct3D.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/TextUtils.h>
#include <U2Core/MAlignment.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Formats/GenbankFeatures.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

DNAAlphabet* DocumentFormatUtils::findAlphabet(const QByteArray& arr) {
    DNAAlphabetRegistry* r = AppContext::getDNAAlphabetRegistry();
    return r->findAlphabet(arr);
}

DNAAlphabet* DocumentFormatUtils::findAlphabet(const QByteArray& arr, const QVector<U2Region>& regionsToProcess) {
    DNAAlphabetRegistry* r = AppContext::getDNAAlphabetRegistry();
    QList<DNAAlphabet*> als = r->findAlphabets(arr, regionsToProcess, true);
    assert(!als.empty());
    return als.first();
}


QList<DNAAlphabet*> DocumentFormatUtils::findAlphabets(const QByteArray& arr) {
    DNAAlphabetRegistry* r = AppContext::getDNAAlphabetRegistry();
    QList<DNAAlphabet*> als = r->findAlphabets(arr);
    return als;
}

DNASequenceObject* DocumentFormatUtils::addSequenceObject(QList<GObject*>& objects, const QString& name, DNASequence& seq, 
                                                          const QVariantMap& hints, U2OpStatus& os) 
{
    if (hints.contains(DocumentReadingMode_MaxObjectsInDoc) && !hints.value(DocumentReadingMode_SequenceAsAlignmentHint).toBool()) {
        int n = hints.value(DocumentReadingMode_MaxObjectsInDoc).toInt();
        if (n > 0 && n <= objects.size()) {
            os.setError(tr("Maximum number of objects per document limit reached. Try different options for opening the document!"));
            return NULL;
        }
    }
    if (seq.alphabet== NULL) {
        seq.alphabet = findAlphabet(seq.seq);
        if (seq.alphabet == NULL) {
            os.setError(tr("Undefined sequence alphabet"));
            return NULL;
        }
    }

    if (!seq.alphabet->isCaseSensitive()) {
        TextUtils::translate(TextUtils::UPPER_CASE_MAP, const_cast<char*>(seq.seq.constData()), seq.seq.length());
    }

    trySqueeze(seq.seq);

    DNASequenceObject* so = new DNASequenceObject(name, seq);
    objects.append(so);
    return so;
}


DNASequenceObject* DocumentFormatUtils::addMergedSequenceObject(QList<GObject*>& objects, const GUrl& docUrl, 
                                                                const QStringList& contigNames, QByteArray& mergedSequence, 
                                                                const QVector<U2Region>& mergedMapping,
                                                                const QVariantMap& hints, U2OpStatus& os) 
{
    if (contigNames.size() == 1) {
        DNAAlphabet* al = findAlphabet(mergedSequence);
        const QString& name = contigNames.first();
        DNASequence seq( mergedSequence, al );
        return DocumentFormatUtils::addSequenceObject(objects, name, seq, hints, os);
    }

    assert(contigNames.size() >=2);
    assert(contigNames.size() == mergedMapping.size());

    DNAAlphabet* al = findAlphabet(mergedSequence, mergedMapping);
    char defSym = al->getDefaultSymbol();
    //fill gaps with defSym
    for (int i = 1; i < mergedMapping.size(); i++) {
        const U2Region& prev = mergedMapping[i-1];
        const U2Region& next = mergedMapping[i];
        int gapSize = next.startPos - prev.endPos();
        assert(gapSize >= 0);
        if (gapSize > 0) {
            qMemSet(mergedSequence.data() + prev.endPos(), defSym, (size_t)gapSize);
        }
    }
    DNASequence seq( mergedSequence, al );
    DNASequenceObject* so = addSequenceObject(objects, "Sequence", seq, hints, os);
    if(os.hasError()) {
        return NULL;
    }
    SAFE_POINT(so != NULL, "DocumentFormatUtils::addSequenceObject returned NULL but didn't set error", NULL);

    AnnotationTableObject* ao = new AnnotationTableObject("Contigs");

    //save relation if docUrl is not empty
    if (!docUrl.isEmpty()) {
        GObjectReference r(docUrl.getURLString(), so->getGObjectName(), GObjectTypes::SEQUENCE);
        ao->addObjectRelation(GObjectRelation(r, GObjectRelationRole::SEQUENCE));
    }

    //save mapping info as annotations
    for (int i = 0; i < contigNames.size(); i++) {
        SharedAnnotationData d(new AnnotationData());
        d->name = "contig";
        d->location->regions << mergedMapping[i];
        ao->addAnnotation(new Annotation(d), NULL);
    }
    objects.append(ao);
    return so;
}


#define MAX_REALLOC_SIZE (300*1000*1000)
#define MIN_K_TO_REALLOC 1.07
void DocumentFormatUtils::trySqueeze(QByteArray& a) {
    //squeeze can cause 2x memory usage -> avoid squeezing of large arrays
    float k =  float(a.capacity()) / a.size();
    if (a.size() <= MAX_REALLOC_SIZE && k > MIN_K_TO_REALLOC) {
        a.squeeze();
    }
}


int DocumentFormatUtils::getIntSettings(const QVariantMap& fs, const char* sName, int defVal) {
    QVariant v = fs.value(sName);
    if (v.type()!= QVariant::Int) {
        return defVal;
    }
    return v.toInt();
}

void DocumentFormatUtils::updateFormatSettings(QList<GObject*>& objects, QVariantMap& fs) {
    //1. remove all cached sequence sizes
    //2. add new sizes
    QList<GObject*> sequences;
    foreach(GObject* obj, objects) {
        if (obj->getGObjectType() == GObjectTypes::SEQUENCE) {
            sequences.append(obj);
        }
    }
    if (sequences.size() == 1) {
        DNASequenceObject* so = qobject_cast<DNASequenceObject*>(sequences.first());
        int len = so->getSequence().length();
        fs[DocumentReadingMode_SequenceMergingFinalSizeHint] = len;
    }
}


class ExtComparator {
public:
    ExtComparator(const QString& _ext) : ext(_ext) {}

    bool operator()(const DocumentFormat* f1, const DocumentFormat* f2) const { 
        int v1 = f1->getSupportedDocumentFileExtensions().contains(ext) ? 1 : 0;
        int v2 = f2->getSupportedDocumentFileExtensions().contains(ext) ? 1 : 0;
        return  v2 < v1; // reverse sort -> make higher vals on the top
    }
    QString ext;
};

QList<DocumentFormatId> DocumentFormatUtils::toIds(const QList<DocumentFormat*>& formats) {
    QList<DocumentFormatId> result;
    foreach(DocumentFormat* f, formats) {
        result.append(f->getFormatId());
    }
    return result;
}

void DocumentFormatUtils::assignAlphabet(MAlignment& ma) {
    QList<DNAAlphabet*> matchedAlphabets;
    for (int i=0, n = ma.getNumRows();i<n; i++) {
        const MAlignmentRow& item = ma.getRow(i);
        QList<DNAAlphabet*> als = DocumentFormatUtils::findAlphabets(item.getCore());
        if (i == 0) {
            matchedAlphabets = als;
        } else {
            QMutableListIterator<DNAAlphabet*> it(matchedAlphabets);
            while (it.hasNext()) {
                DNAAlphabet* al = it.next();
                if (!als.contains(al)) {
                    it.remove();
                }
            }
            if (matchedAlphabets.isEmpty()) {
                break;
            }
        }
    }
    if (matchedAlphabets.isEmpty()) {
        return; //nothing matched
    }

    ma.setAlphabet(matchedAlphabets.first());

    if (!ma.getAlphabet()->isCaseSensitive()) {
        ma.toUpperCase();
    }
}

void DocumentFormatUtils::assignAlphabet(MAlignment& ma, char ignore) {
    QList<DNAAlphabet*> matchedAlphabets;
    for (int i=0, n = ma.getNumRows();i<n; i++) {
        const MAlignmentRow& item = ma.getRow(i);
        QByteArray core = item.getCore();
        core.replace(ignore, MAlignment_GapChar);
        QList<DNAAlphabet*> als = DocumentFormatUtils::findAlphabets(core);
        if (i == 0) {
            matchedAlphabets = als;
        } else {
            QMutableListIterator<DNAAlphabet*> it(matchedAlphabets);
            while (it.hasNext()) {
                DNAAlphabet* al = it.next();
                if (!als.contains(al)) {
                    it.remove();
                }
            }
            if (matchedAlphabets.isEmpty()) {
                break;
            }
        }
    }
    if (matchedAlphabets.isEmpty()) {
        return; //nothing matched
    }

    ma.setAlphabet(matchedAlphabets.first());

    if (!ma.getAlphabet()->isCaseSensitive()) {
        ma.toUpperCase();
    }
}

QList<AnnotationSettings*> DocumentFormatUtils::predefinedSettings() {
    QList<AnnotationSettings*> predefined;
    foreach(GBFeatureKeyInfo fi, GBFeatureUtils::allKeys()) {
        AnnotationSettings* as = new AnnotationSettings();
        as->name = fi.text;
        as->amino = fi.showOnaminoFrame;
        as->color = fi.color;
        as->visible = as->name!="source";
        as->nameQuals = fi.namingQuals;
        predefined.append(as);
    }
    AnnotationSettings* secStructAnnotationSettings = new  AnnotationSettings(BioStruct3D::SecStructAnnotationTag, true, QColor(102,255, 0), true);
    secStructAnnotationSettings->nameQuals.append(BioStruct3D::SecStructTypeQualifierName);
    predefined.append(secStructAnnotationSettings);
    predefined.append(new AnnotationSettings(BioStruct3D::AlphaHelixAnnotationTag, true, QColor(102,255, 0), true));
    predefined.append(new AnnotationSettings(BioStruct3D::BetaStrandAnnotationTag, true, QColor(255,255,153), true));
    predefined.append(new AnnotationSettings(BioStruct3D::TurnAnnotationTag, true, QColor(255,85,127), true));
    predefined.append(new AnnotationSettings(BioStruct3D::MoleculeAnnotationTag, false, QColor(0,255,0), false));
    return predefined;
}

QList<DNASequence> DocumentFormatUtils::toSequences(const GObject* obj) {
    QList<DNASequence> res;
    const DNASequenceObject* seqObj = qobject_cast<const DNASequenceObject*>(obj);
    if (seqObj != NULL) {
        res << seqObj->getDNASequence();
        return res;
    }
    const MAlignmentObject* maObj = qobject_cast<const MAlignmentObject*>(obj);
    DNAAlphabet* al = maObj->getMAlignment().getAlphabet();
    foreach(const MAlignmentRow& row, maObj->getMAlignment().getRows()) {
        DNASequence seq;
        seq.seq = row.toByteArray(row.getCoreEnd());
        seq.setName(row.getName());
        seq.alphabet = al;
        res << seq;
    }
    return res;
}

// This property is replaced with (DocumentReadingMode_SequenceMergeGapSize "merge-gap") today
// we keep this property for compatibility with previous version of UGENE only
#define MERGE_MULTI_DOC_GAP_SIZE_SETTINGS_DEPRECATED   "merge_gap"

int DocumentFormatUtils::getMergeGap(const QVariantMap& hints) {
    int res = getIntSettings(hints, DocumentReadingMode_SequenceMergeGapSize, -1);
    if (res == -1)  {
        res = getIntSettings(hints, MERGE_MULTI_DOC_GAP_SIZE_SETTINGS_DEPRECATED, -1);
    }
    return res;
}


} //namespace
