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

#include "DocumentFormatUtils.h"

#include <U2Core/AnnotationSettings.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/BioStruct3D.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Formats/GenbankFeatures.h>


namespace U2 {

static int getIntSettings(const QVariantMap& fs, const char* sName, int defVal) {
    QVariant v = fs.value(sName);
    if (v.type()!= QVariant::Int) {
        return defVal;
    }
    return v.toInt();
}


U2SequenceObject* DocumentFormatUtils::addSequenceObject(const U2DbiRef& dbiRef, const QString& name, const QByteArray& seq,  bool circular, const QVariantMap& hints, U2OpStatus& os) 
{
    U2SequenceImporter importer;
    
    importer.startSequence(dbiRef, name, circular, os);
    CHECK_OP(os, NULL);
    
    importer.addBlock(seq.constData(), seq.length(), os);
    CHECK_OP(os, NULL);
    
    U2Sequence sequence = importer.finalizeSequence(os);
    TmpDbiObjects dbiObjects(dbiRef, os);
    dbiObjects.objects << sequence.id;
    CHECK_OP(os, NULL);

    U2SequenceObject* so = new U2SequenceObject(name, U2EntityRef(dbiRef, sequence.id), hints);
    return so;
}


AnnotationTableObject* DocumentFormatUtils::addAnnotationsForMergedU2Sequence(const GUrl& docUrl, const QStringList& contigNames, 
                                                                        const U2Sequence& mergedSequence, 
                                                                        const QVector<U2Region>& mergedMapping, 
                                                                        U2OpStatus& os) 
{
    Q_UNUSED(os);
    AnnotationTableObject* ao = new AnnotationTableObject("Contigs");

    //save relation if docUrl is not empty
    if (!docUrl.isEmpty()) {
        GObjectReference r(docUrl.getURLString(), mergedSequence.visualName, GObjectTypes::SEQUENCE);
        ao->addObjectRelation(GObjectRelation(r, GObjectRelationRole::SEQUENCE));
    }

    //save mapping info as annotations
    QStringList::const_iterator it = contigNames.begin();
    for (int i = 0; it != contigNames.end(); i++, it++) {
        SharedAnnotationData d(new AnnotationData());
        d->name = QString("contig");
        d->location->regions << mergedMapping[i];
        d->qualifiers << U2Qualifier("name", *it);
        d->qualifiers << U2Qualifier("number", QString("%1").arg(i));
        ao->addAnnotation(new Annotation(d), NULL);
    }
    return ao;
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

QList<AnnotationSettings*> DocumentFormatUtils::predefinedSettings() {
    QList<AnnotationSettings*> predefined;
    foreach(GBFeatureKeyInfo fi, GBFeatureUtils::allKeys()) {
        AnnotationSettings* as = new AnnotationSettings();
        as->name = fi.text;
        as->amino = fi.showOnaminoFrame;
        as->color = fi.color;
        as->visible = as->name!="source";
        as->nameQuals = fi.namingQuals;
        if (!fi.namingQuals.isEmpty()) {
            as->showNameQuals = true;
        }
        predefined.append(as);
    }
    AnnotationSettings* secStructAnnotationSettings = new  AnnotationSettings(BioStruct3D::SecStructAnnotationTag, true, QColor(102,255, 0), true);
    secStructAnnotationSettings->nameQuals.append(BioStruct3D::SecStructTypeQualifierName);
    secStructAnnotationSettings->showNameQuals = true;
    predefined.append(secStructAnnotationSettings);
    predefined.append(new AnnotationSettings(BioStruct3D::AlphaHelixAnnotationTag, true, QColor(102,255, 0), true));
    predefined.append(new AnnotationSettings(BioStruct3D::BetaStrandAnnotationTag, true, QColor(255,255,153), true));
    predefined.append(new AnnotationSettings(BioStruct3D::TurnAnnotationTag, true, QColor(255,85,127), true));
    predefined.append(new AnnotationSettings(BioStruct3D::MoleculeAnnotationTag, false, QColor(0,255,0), false));
    return predefined;
}

QList<DNASequence> DocumentFormatUtils::toSequences(const GObject* obj) {
    QList<DNASequence> res;
    SAFE_POINT(obj != NULL, "Gobject is NULL", res);
    const U2SequenceObject* seqObj = qobject_cast<const U2SequenceObject*>(obj);
    if (seqObj != NULL) {
        res << seqObj->getWholeSequence();
        return res;
    }
    const MAlignmentObject* maObj = qobject_cast<const MAlignmentObject*>(obj);
    SAFE_POINT(maObj != NULL, "MAlignmentObject is NULL", res);
    DNAAlphabet* al = maObj->getMAlignment().getAlphabet();
    U2OpStatus2Log os;
    foreach(const MAlignmentRow& row, maObj->getMAlignment().getRows()) {
        DNASequence seq;
        seq.seq = row.toByteArray(row.getCoreEnd(), os);
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


int DocumentFormatUtils::getMergedSize(const QVariantMap& hints, int defaultVal) {
    int res = getIntSettings(hints, DocumentReadingMode_SequenceMergingFinalSizeHint, defaultVal);
    return res;
}

void DocumentFormatUtils::updateFormatHints(QList<GObject*>& objects, QVariantMap& fs) {
    //1. remove all cached sequence sizes
    //2. add new sizes
    QList<GObject*> sequences;
    foreach(GObject* obj, objects) {
        if (obj->getGObjectType() == GObjectTypes::SEQUENCE) {
            sequences.append(obj);
        }
    }
    if (sequences.size() == 1) {
        U2SequenceObject* so = qobject_cast<U2SequenceObject*>(sequences.first());
        int len = so->getSequenceLength();
        fs[DocumentReadingMode_SequenceMergingFinalSizeHint] = len;
    }
}


U2SequenceObject* DocumentFormatUtils::addSequenceObjectDeprecated(const U2DbiRef& dbiRef, const QString& seqObjName,
                                                                   QList<GObject*>& objects, DNASequence& sequence, U2OpStatus& os)
{
#ifdef _DEBUG
    foreach(GObject* obj, objects) {
        const QString& name = obj->getGObjectName();
        assert(name != seqObjName);
    }
#endif

    if (sequence.alphabet== NULL) {
        sequence.alphabet = U2AlphabetUtils::findBestAlphabet(sequence.seq);
        CHECK_EXT(sequence.alphabet != NULL, os.setError(tr("Undefined sequence alphabet")), NULL);
    }

    if (!sequence.alphabet->isCaseSensitive()) {
        TextUtils::translate(TextUtils::UPPER_CASE_MAP, const_cast<char*>(sequence.seq.constData()), sequence.seq.length());
    }

    U2SequenceImporter importer;
    importer.startSequence(dbiRef, sequence.getName(), sequence.circular, os);
    CHECK_OP(os, NULL);
    importer.addBlock(sequence.seq.constData(), sequence.seq.length(), os);
    CHECK_OP(os, NULL);
    U2Sequence u2seq = importer.finalizeSequence(os);
    TmpDbiObjects dbiObjects(dbiRef, os);
    dbiObjects.objects << u2seq.id;
    CHECK_OP(os, NULL);
    
    U2SequenceObject* so = new U2SequenceObject(seqObjName, U2EntityRef(dbiRef, u2seq.id));
    so->setSequenceInfo(sequence.info);
    objects << so;
    return so;
}


U2SequenceObject* DocumentFormatUtils::addMergedSequenceObjectDeprecated(const U2DbiRef& dbiRef, 
                                                                QList<GObject*>& objects, const GUrl& docUrl, 
                                                                const QStringList& contigNames, QByteArray& mergedSequence, 
                                                                const QVector<U2Region>& mergedMapping,
                                                                U2OpStatus& os)
{
    if (contigNames.size() == 1) {
        DNAAlphabet* al = U2AlphabetUtils::findBestAlphabet(mergedSequence);
        const QString& name = contigNames.first();
        DNASequence seq(name, mergedSequence, al );
        return DocumentFormatUtils::addSequenceObjectDeprecated(dbiRef, name, objects, seq, os);
    }

    assert(contigNames.size() >= 2);
    assert(contigNames.size() == mergedMapping.size());

    DNAAlphabet* al = U2AlphabetUtils::findBestAlphabet(mergedSequence, mergedMapping);
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
    ;
    DNASequence seq("Sequence", mergedSequence, al);
    U2SequenceObject* so = addSequenceObjectDeprecated(dbiRef, "Sequence", objects, seq, os);
    CHECK_OP(os, NULL);
    SAFE_POINT(so != NULL, "DocumentFormatUtils::addSequenceObject returned NULL but didn't set error", NULL);
    

    AnnotationTableObject* ao = new AnnotationTableObject("Annotations");

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


} //namespace
