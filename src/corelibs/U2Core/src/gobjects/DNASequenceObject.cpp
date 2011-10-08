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


#include <U2Core/DocumentModel.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2AttributeDbi.h>

#include "GObjectTypes.h"

#include "DNASequenceObject.h"


namespace U2 {

#define NO_LENGTH_CONSTRAINT -1
U2SequenceObjectConstraints::U2SequenceObjectConstraints(QObject* p) 
: GObjectConstraints(GObjectTypes::SEQUENCE, p), sequenceSize(NO_LENGTH_CONSTRAINT)
{
}

//////////////////////////////////////////////////////////////////////////
// U2SequenceObject
U2SequenceObject::U2SequenceObject(const QString& name, const U2EntityRef& seqRef, const QVariantMap& hintsMap) 
: GObject(GObjectTypes::SEQUENCE, name, hintsMap),  
cachedLength(-1), cachedAlphabet(NULL), cachedCircular(TriState_Unknown)
{
    entityRef = seqRef;
}

bool U2SequenceObject::checkConstraints(const GObjectConstraints* c) const {
    const U2SequenceObjectConstraints* dnac = qobject_cast<const U2SequenceObjectConstraints*>(c);
    SAFE_POINT(dnac != NULL, "Not a U2SequenceObjectConstraints!", NULL);

    bool resultDNAConstraints = true;
    bool resultAlphabetType = true;

    if (dnac->sequenceSize != NO_LENGTH_CONSTRAINT) {
        qint64 seqLen = getSequenceLength();
        if (seqLen != dnac->sequenceSize) {
            return false;
        }
    }
    if (dnac->alphabetType != DNAAlphabet_RAW) {
        DNAAlphabetType aType = getAlphabet()->getType();
        if (dnac->alphabetType != aType) {
            return false;
        }
    }
    return true;
}


qint64 U2SequenceObject::getSequenceLength() const {
    if (cachedLength == -1) {
        U2OpStatus2Log os;
        DbiConnection con(entityRef.dbiRef, os);
        CHECK_OP(os, -1);
        cachedLength = con.dbi->getSequenceDbi()->getSequenceObject(entityRef.entityId, os).length;
    }
    return cachedLength;
}

QString U2SequenceObject::getSequenceName() const  {
    if (cachedName.isEmpty()) {
        U2OpStatus2Log os;
        DbiConnection con(entityRef.dbiRef, os);
        CHECK_OP(os, "");
        cachedName = con.dbi->getSequenceDbi()->getSequenceObject(entityRef.entityId, os).visualName;
    }
    return cachedName;
}

DNASequence U2SequenceObject::getWholeSequence() const {
    QByteArray wholeSeq = getWholeSequenceData();
    DNAAlphabet* alpha = getAlphabet();
    QString seqName = getSequenceName();
    DNASequence res(seqName, wholeSeq, alpha);
    res.circular = isCircular();
    res.quality = getQuality();
    return res;
}

QByteArray U2SequenceObject::getWholeSequenceData() const {
    return getSequenceData(U2_REGION_MAX);
}

bool U2SequenceObject::isCircular() const {
    if (cachedCircular == TriState_Unknown) {
        U2OpStatus2Log os;
        DbiConnection con(entityRef.dbiRef, os);
        CHECK_OP(os, false);
        cachedCircular = con.dbi->getSequenceDbi()->getSequenceObject(entityRef.entityId, os).circular ? TriState_Yes : TriState_No;
    }
    return cachedCircular == TriState_Yes;
}

DNAAlphabet* U2SequenceObject::getAlphabet() const {
    if (cachedAlphabet == NULL) {
        U2OpStatus2Log os;
        DbiConnection con(entityRef.dbiRef, os);
        CHECK_OP(os, NULL);
        U2AlphabetId alphaId = con.dbi->getSequenceDbi()->getSequenceObject(entityRef.entityId, os).alphabet;
        CHECK_OP(os, NULL);
        cachedAlphabet = U2AlphabetUtils::getById(alphaId);
    }
    return cachedAlphabet;
}

void U2SequenceObject::setWholeSequence(const DNASequence& seq) {
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );
    con.dbi->getSequenceDbi()->updateSequenceData(entityRef.entityId, U2_REGION_MAX, seq.seq, os);
    CHECK_OP(os, );
    if (!seq.quality.isEmpty()) {
        setQuality(seq.quality);
    }
    cachedLength = -1;
    setModified(true);
    emit si_sequenceChanged();
}

QByteArray U2SequenceObject::getSequenceData(const U2Region& r) const {
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, QByteArray());
    QByteArray res = con.dbi->getSequenceDbi()->getSequenceData(entityRef.entityId, r, os);
    return res;
}

void U2SequenceObject::replaceRegion(const U2Region& region, const DNASequence& seq, U2OpStatus& os) {
    CHECK_EXT(seq.alphabet == getAlphabet() || seq.seq.isEmpty(), os.setError(tr("Modified sequence & region have different alphabet")), );
    
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );
    con.dbi->getSequenceDbi()->updateSequenceData(entityRef.entityId, region, seq.seq, os);
    cachedLength = -1;
    setModified(true);
    emit si_sequenceChanged();
}

GObject* U2SequenceObject::clone(const U2DbiRef& dbiRef, U2OpStatus& os) const {
    U2Sequence seq = U2SequenceUtils::copySequence(entityRef, dbiRef, os);
    CHECK_OP(os, NULL);
    return new U2SequenceObject(seq.visualName, U2EntityRef(dbiRef, seq.id), getGHintsMap());
}

void U2SequenceObject::setCircular(bool v) {
    //TODO:
}

void U2SequenceObject::setQuality(const DNAQuality& q) {
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );
    QList<U2DataId> idQualList=con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,DNAInfo::FASTQ_QUAL_CODES,os);
    CHECK_OP(os, );
    if(!idQualList.isEmpty()){
        con.dbi->getAttributeDbi()->removeObjectAttributes(idQualList.first(),os);
        CHECK_OP(os, );
    }
    QList<U2DataId> idQualTypeList=con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,DNAInfo::FASTQ_QUAL_TYPE,os);
    CHECK_OP(os, );
    if(!idQualTypeList.isEmpty()){
        con.dbi->getAttributeDbi()->removeObjectAttributes(idQualTypeList.first(),os);
        CHECK_OP(os, );
    }

    U2ByteArrayAttribute qualityCodes(entityRef.entityId, DNAInfo::FASTQ_QUAL_CODES,q.qualCodes);
    U2IntegerAttribute   qualityType(entityRef.entityId, DNAInfo::FASTQ_QUAL_TYPE,q.type);
    con.dbi->getAttributeDbi()->createByteArrayAttribute(qualityCodes,os);
    CHECK_OP(os, );
    con.dbi->getAttributeDbi()->createIntegerAttribute(qualityType,os);
    CHECK_OP(os, );
}

DNAQuality U2SequenceObject::getQuality() const {
    //TODO: may be remove redundant checks
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    DNAQuality res;
    QList<U2DataId> idQualList=con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,DNAInfo::FASTQ_QUAL_CODES,os);
    CHECK_OP(os, res);
    QList<U2DataId> idQualTypeList=con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,DNAInfo::FASTQ_QUAL_TYPE,os);
    CHECK_OP(os, res);
    if(!idQualList.isEmpty() && !idQualTypeList.isEmpty()){
        res.qualCodes=con.dbi->getAttributeDbi()->getByteArrayAttribute(idQualList.first(),os).value;
        CHECK_OP(os, DNAQuality());
        res.type=(DNAQualityType)con.dbi->getAttributeDbi()->getIntegerAttribute(idQualTypeList.first(),os).value;
        CHECK_OP(os, DNAQuality());
    }
    return res;
}

QString U2SequenceObject::getSequenceAttribute(const QString& seqAttr) const {
    //TODO
    return "";
}

}//namespace


