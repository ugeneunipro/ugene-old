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
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Formats/GenbankFeatures.h>

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

DNASequenceObject* DocumentFormatUtils::addSequenceObject(QList<GObject*>& objects, const QString& name, DNASequence& seq) {
    if (seq.alphabet== NULL) {
        seq.alphabet = findAlphabet(seq.seq);
        assert(seq.alphabet!=NULL);
    }

    if (!seq.alphabet->isCaseSensitive()) {
        TextUtils::translate(TextUtils::UPPER_CASE_MAP, const_cast<char*>(seq.seq.constData()), seq.seq.length());
    }

    trySqueeze(seq.seq);

    DNASequenceObject* so = new DNASequenceObject(name, seq);
    objects.append(so);
    return so;
}


DNASequenceObject* DocumentFormatUtils::addMergedSequenceObject(QList<GObject*>& objects, const GUrl& docUrl, const QStringList& contigs, QByteArray& mergedSequence, const QVector<U2Region>& mergedMapping) {
    if (contigs.size() == 1) {
        DNAAlphabet* al = findAlphabet(mergedSequence);
        const QString& name = contigs.first();
        DNASequence seq( mergedSequence, al );
        return DocumentFormatUtils::addSequenceObject(objects, name, seq);
    }

    assert(contigs.size() >=2);
    assert(contigs.size() == mergedMapping.size());

    DNAAlphabet* al = findAlphabet(mergedSequence, mergedMapping);
    char defSym = al->getDefaultSymbol();
    //fill gaps with defSym
    for (int i=1; i<mergedMapping.size(); i++) {
        const U2Region& prev = mergedMapping[i-1];
        const U2Region& next = mergedMapping[i];
        int gapSize = next.startPos - prev.endPos();
        assert(gapSize >= 0);
        if (gapSize > 0) {
            qMemSet(mergedSequence.data() + prev.endPos(), defSym, (size_t)gapSize);
        }
    }
    DNASequence seq( mergedSequence, al );
    DNASequenceObject* so = addSequenceObject(objects, "Sequence", seq);
    AnnotationTableObject* ao = new AnnotationTableObject("Contigs");

    //save relation if docUrl is not empty
    if (!docUrl.isEmpty()) {
        GObjectReference r(docUrl.getURLString(), so->getGObjectName(), GObjectTypes::SEQUENCE);
        ao->addObjectRelation(GObjectRelation(r, GObjectRelationRole::SEQUENCE));
    }

    //save mapping info as annotations
    for (int i=0; i<contigs.size(); i++) {
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
    //squeeze can cause 2x memusage -> avoid squeezing of large arrays
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
        fs[MERGE_MULTI_DOC_SEQUENCE_SIZE_SETTINGS] = len;
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

} //namespace
