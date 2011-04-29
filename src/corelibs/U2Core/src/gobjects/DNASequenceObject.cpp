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

#include "DNASequenceObject.h"

#include "GObjectTypes.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/Timer.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

DNASequenceObjectSequenceDbiWrapper::DNASequenceObjectSequenceDbiWrapper(const QList<DNASequenceObject*> & _seqObjs, U2Dbi * root)
: QObject(_seqObjs.first()), U2SimpleSequenceDbi(root), seqObjs(_seqObjs){
}

static DNASequenceObject* findObjById(const U2DataId & seqId, const QList<DNASequenceObject*> & objs) {
    foreach(DNASequenceObject* obj, objs) {
        if(obj->getGObjectName().toUtf8() == seqId) {
            return obj;
        }
    }
    return NULL;
}

U2Sequence DNASequenceObjectSequenceDbiWrapper::getSequenceObject(const U2DataId& sequenceId, U2OpStatus& os)  {
    DNASequenceObject * seqObj = findObjById(sequenceId, seqObjs);
    if(seqObj == NULL) {
        return U2Sequence();
    }
    Document* doc = seqObj->getDocument();
    QString docUrl = (doc == NULL) ? QString("") : doc->getURLString();
    U2Sequence res(seqObj->getGObjectName().toUtf8(), docUrl, 0);
    res.alphabet = seqObj->getAlphabet()->getId();
    res.length = seqObj->getSequenceLen();
    res.circular = seqObj->isCircular();
    return res;
}

QByteArray DNASequenceObjectSequenceDbiWrapper::getSequenceData(const U2DataId& sequenceId, const U2Region& region, U2OpStatus& os)  {
    GTIMER(c1, t1, "DNASequenceObjectSequenceDbiWrapper::getSequenceData");
    DNASequenceObject * seqObj = findObjById(sequenceId, seqObjs);
    if(seqObj == NULL) {
        return QByteArray();
    }
    return seqObj->getSequence().mid(region.startPos, region.length);
}

DNASequenceObject::DNASequenceObject(const QString& name, const DNASequence& seq, const QVariantMap& hintsMap) 
: GObject(GObjectTypes::SEQUENCE, name, hintsMap), dnaSeq(seq), dbi(NULL)
{
    assert(dnaSeq.alphabet!=NULL);
    seqRange = U2Region(0, dnaSeq.seq.length());
    dbi = new DNASequenceObjectSequenceDbiWrapper(QList<DNASequenceObject*>() << this, NULL);
}

DNASequenceObject::~DNASequenceObject() {
    delete dbi;
}

GObject* DNASequenceObject::clone() const {
    DNASequenceObject* cln = new DNASequenceObject(getGObjectName(), dnaSeq, getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    return cln;
}

void DNASequenceObject::setBase(int pos, char base) {
    SAFE_POINT(pos >=0 && pos < dnaSeq.length(), "Position is out of range!",);
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



/*
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

*/

}//namespace


