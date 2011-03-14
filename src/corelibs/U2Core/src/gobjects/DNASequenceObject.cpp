#include "DNASequenceObject.h"

#include "GObjectTypes.h"

#include <U2Core/DNAAlphabet.h>

namespace U2 {

DNASequenceObject::DNASequenceObject(const QString& name, const DNASequence& seq, const QVariantMap& hintsMap) 
: GObject(GObjectTypes::SEQUENCE, name, hintsMap), U2SequenceRDbi(NULL), dnaSeq(seq)
{
    assert(dnaSeq.alphabet!=NULL);
    seqRange = U2Region(0, dnaSeq.seq.length());
}

GObject* DNASequenceObject::clone() const {
    DNASequenceObject* cln = new DNASequenceObject(getGObjectName(), dnaSeq, getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    return cln;
}

void DNASequenceObject::setBase(int pos, char base) {
    assert(pos >=0 && pos < dnaSeq.length());
    dnaSeq.seq[pos] = base;
    setModified(true);
}

bool DNASequenceObject::checkConstraints(const GObjectConstraints* c) const {
    const DNASequenceObjectConstraints* dnac = qobject_cast<const DNASequenceObjectConstraints*>(c);
    bool resultDNAConstraints = true;
    bool resultAlphabetType = true;

    if (dnac->exactSequenceSize != dnaSeq.length()) {
        resultDNAConstraints = false;
    }
    if (dnac->exactSequenceSize <= 0)   {
        resultDNAConstraints = true;
    }
    if (dnac->alphabetType != getAlphabet()->getType()) {
        resultAlphabetType = false;
    }
    if (dnac->alphabetType == DNAAlphabet_RAW)  {
        resultAlphabetType = true;
    }

    return (resultDNAConstraints && resultAlphabetType);
}

void DNASequenceObject::setCircular(bool val) {
    dnaSeq.circular = true;
}

void DNASequenceObject::setSequence( DNASequence seq ){
    dnaSeq = seq;
    U2Region range(0, seq.length());
    seqRange = range;
    setModified(true);
    emit si_sequenceChanged();
}

void DNASequenceObject::setQuality( const DNAQuality& quality )
{
    setModified(true);
    dnaSeq.quality = quality;
    emit si_sequenceChanged();
}

DNASequenceObjectConstraints::DNASequenceObjectConstraints(QObject* p) 
: GObjectConstraints(GObjectTypes::SEQUENCE, p)
{
}



U2Sequence DNASequenceObject::getSequenceObject(U2DataId sequenceId, U2OpStatus& os) {
    GObjectReference ref(this);
    U2Sequence res;
    res.id = sequenceId;
    res.dbiId = ref.docUrl + "|" + ref.objName;
    res.alphabet = dnaSeq.alphabet->getId();
    res.length = dnaSeq.length();
    res.circular = isCircular();
    res.version = -1; // not supported
    return res;
}
    
QByteArray DNASequenceObject::getSequenceData(U2DataId sequenceId, const U2Region& region, U2OpStatus& os) {
    U2Region safeRegion = seqRange.intersect(region);
    if (safeRegion.isEmpty()) {
        return QByteArray();
    }
    return dnaSeq.seq.mid(safeRegion.startPos, safeRegion.length);
}


}//namespace


