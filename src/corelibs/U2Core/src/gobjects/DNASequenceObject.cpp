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


#include <U2Core/DocumentModel.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2AttributeUtils.h>

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
cachedAlphabet(NULL), cachedLength(-1), cachedCircular(TriState_Unknown)
{
    entityRef = seqRef;
}

bool U2SequenceObject::checkConstraints(const GObjectConstraints* c) const {
    const U2SequenceObjectConstraints* dnac = qobject_cast<const U2SequenceObjectConstraints*>(c);
    SAFE_POINT(dnac != NULL, "Not a U2SequenceObjectConstraints!", NULL);

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

#define FETCH_SEQUENCE(seqGet, seq, entityRef) \
    if (!seqGet) { \
        U2OpStatus2Log os; \
        DbiConnection con(entityRef.dbiRef, os); \
        CHECK_OP(os, DNASequence()); \
        seq = con.dbi->getSequenceDbi()->getSequenceObject(entityRef.entityId, os); \
        CHECK_OP(os, DNASequence()); \
        seqGet = true; \
    }

DNASequence U2SequenceObject::getWholeSequence() const {
    QByteArray wholeSeq = getWholeSequenceData();

    U2Sequence seq;
    bool seqGet = false;

    DNAAlphabet* alpha = cachedAlphabet;
    if (!cachedAlphabet) {
        FETCH_SEQUENCE(seqGet, seq, entityRef);
        cachedAlphabet = alpha = U2AlphabetUtils::getById(seq.alphabet);
    }
    QString seqName = cachedName;
    if (cachedName.isEmpty()) {
        FETCH_SEQUENCE(seqGet, seq, entityRef);
        seqName = cachedName = seq.visualName;
    }
    DNASequence res(seqName, wholeSeq, alpha);
    if (cachedCircular == TriState_Unknown) {
        FETCH_SEQUENCE(seqGet, seq, entityRef);
        cachedCircular = seq.circular ? TriState_Yes : TriState_No;
    }
    res.circular = (cachedCircular == TriState_Yes);

    res.quality = getQuality();
    res.info = getSequenceInfo();
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
    QVariantMap hints;
    con.dbi->getSequenceDbi()->updateSequenceData(entityRef.entityId, U2_REGION_MAX, seq.seq, hints, os);
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

QByteArray U2SequenceObject::getSequenceData(const U2Region& r, U2OpStatus& os) const {
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, QByteArray());

    QByteArray res = con.dbi->getSequenceDbi()->getSequenceData(entityRef.entityId, r, os);
    return res;
}

bool U2SequenceObject::isValidDbiObject(U2OpStatus &os) {
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, false);
    U2Sequence s = con.dbi->getSequenceDbi()->getSequenceObject(entityRef.entityId, os);
    CHECK_OP(os, false);
    if(U2AlphabetUtils::getById(s.alphabet) == NULL) {
        os.setError(tr("Internal error, sequence alphabet id '%1' is invalid").arg(s.alphabet.id));
        return false;
    }
    return true;
}

void U2SequenceObject::replaceRegion(const U2Region& region, const DNASequence& seq, U2OpStatus& os) {
    // seq.alphabet == NULL - for tests.
    CHECK_EXT(seq.alphabet == getAlphabet() || seq.seq.isEmpty() || seq.alphabet == NULL, 
        os.setError(tr("Modified sequence & region have different alphabet")), );
    
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );
    QVariantMap hints;
    con.dbi->getSequenceDbi()->updateSequenceData(entityRef.entityId, region, seq.seq, hints, os);
    cachedLength = -1;
    setModified(true);
    emit si_sequenceChanged();
}

GObject* U2SequenceObject::clone(const U2DbiRef& dbiRef, U2OpStatus& os) const {
    DbiConnection srcCon(this->entityRef.dbiRef, os);
    CHECK_OP(os, NULL);
    DbiConnection dstCon(dbiRef, true, os);
    CHECK_OP(os, NULL);

    U2Sequence seq = U2SequenceUtils::copySequence(entityRef, dbiRef, os);
    CHECK_OP(os, NULL);

    U2SequenceObject* res = new U2SequenceObject(seq.visualName, U2EntityRef(dbiRef, seq.id), getGHintsMap());

    U2AttributeUtils::copyObjectAttributes(entityRef.entityId, seq.id,
        srcCon.dbi->getAttributeDbi(), dstCon.dbi->getAttributeDbi(), os);
    CHECK_OP(os, NULL);

    return res;
}

void U2SequenceObject::setCircular(bool isCircular) {

    TriState newVal = isCircular ? TriState_Yes : TriState_No;
    if ( newVal == cachedCircular ) {
        return;
    }

    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );
    con.dbi->getSequenceDbi()->getSequenceObject(entityRef.entityId, os).circular = isCircular;
    CHECK_OP(os, );
    cachedCircular = newVal;


}

void U2SequenceObject::setQuality(const DNAQuality& q) {
    U2SequenceUtils::setQuality(entityRef, q);
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

void U2SequenceObject::setSequenceInfo(const QVariantMap& info) {
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );
    QList<U2DataId> chainIdList = con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,DNAInfo::CHAIN_ID,os);
    CHECK_OP(os, );
    if(!chainIdList.isEmpty()){
        con.dbi->getAttributeDbi()->removeObjectAttributes(chainIdList.first(),os);
        CHECK_OP(os, );
    }
    QList<U2DataId> commentList = con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,DNAInfo::COMMENT,os);
    CHECK_OP(os, );
    if(!commentList.isEmpty()){
        con.dbi->getAttributeDbi()->removeObjectAttributes(commentList.first(),os);
        CHECK_OP(os, );
    }
    QList<U2DataId> definitionList = con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,DNAInfo::DEFINITION,os);
    CHECK_OP(os, );
    if(!definitionList.isEmpty()){
        con.dbi->getAttributeDbi()->removeObjectAttributes(definitionList.first(),os);
        CHECK_OP(os, );
    }
    U2StringAttribute chainID(entityRef.entityId, DNAInfo::CHAIN_ID, info.value(DNAInfo::CHAIN_ID).toString());
    U2StringAttribute comment(entityRef.entityId, DNAInfo::COMMENT, info.value(DNAInfo::COMMENT).toString());
    U2StringAttribute definition(entityRef.entityId, DNAInfo::DEFINITION, info.value(DNAInfo::DEFINITION).toString());
    con.dbi->getAttributeDbi()->createStringAttribute(chainID, os);
    CHECK_OP(os, );
    con.dbi->getAttributeDbi()->createStringAttribute(comment, os);
    CHECK_OP(os, );
    con.dbi->getAttributeDbi()->createStringAttribute(definition, os);
    CHECK_OP(os, );
}

QVariantMap U2SequenceObject::getSequenceInfo() const {
    U2OpStatus2Log os;
    QVariantMap resultingInfo;
    DbiConnection con(entityRef.dbiRef, os);
    QList<U2DataId> chainIdList = con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,DNAInfo::CHAIN_ID,os);
    CHECK_OP(os, resultingInfo);
    QList<U2DataId> commentList = con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,DNAInfo::COMMENT,os);
    CHECK_OP(os, resultingInfo);
    QList<U2DataId> definitionList = con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,DNAInfo::DEFINITION,os);
    CHECK_OP(os, resultingInfo);
    if(!chainIdList.isEmpty() && !commentList.isEmpty() && !definitionList.isEmpty()) {
        resultingInfo.insert(DNAInfo::CHAIN_ID, con.dbi->getAttributeDbi()->getStringAttribute(chainIdList.first(),os).value);
        CHECK_OP(os, QVariantMap());
        resultingInfo.insert(DNAInfo::COMMENT, con.dbi->getAttributeDbi()->getStringAttribute(commentList.first(),os).value);
        CHECK_OP(os, QVariantMap());
        resultingInfo.insert(DNAInfo::DEFINITION, con.dbi->getAttributeDbi()->getStringAttribute(definitionList.first(),os).value);
        CHECK_OP(os, QVariantMap());
    }
    U2StringAttribute attr = U2AttributeUtils::findStringAttribute(con.dbi->getAttributeDbi(), entityRef.entityId, DNAInfo::GENBANK_HEADER, os);
    if(attr.hasValidId()) {
        resultingInfo.insert(DNAInfo::GENBANK_HEADER, attr.value);
        CHECK_OP(os, QVariantMap());
    }

    attr = U2AttributeUtils::findStringAttribute(con.dbi->getAttributeDbi(), entityRef.entityId, DNAInfo::SOURCE, os);
    if(attr.hasValidId()) {
        resultingInfo.insert(DNAInfo::SOURCE, attr.value);
        CHECK_OP(os, QVariantMap());
    }

    attr = U2AttributeUtils::findStringAttribute(con.dbi->getAttributeDbi(), entityRef.entityId, DNAInfo::ACCESSION, os);
    if(attr.hasValidId()) {
        resultingInfo.insert(DNAInfo::ACCESSION, attr.value);
        CHECK_OP(os, QVariantMap());
    }
    attr = U2AttributeUtils::findStringAttribute(con.dbi->getAttributeDbi(), entityRef.entityId, Translation_Table_Id_Attribute, os);
    if(attr.hasValidId()) {
        resultingInfo.insert(Translation_Table_Id_Attribute, attr.value);
        CHECK_OP(os, QVariantMap());
    }
    
    QString name = this->getSequenceName();
    if (!name.isEmpty()) {
        resultingInfo.insert(DNAInfo::ID,name);  
    }
    return resultingInfo;
}

QString U2SequenceObject::getStringAttribute(const QString& seqAttr) const {
    return getSequenceInfo().value(seqAttr).toString();
}

void U2SequenceObject::setStringAttribute(const QString& newStringAttributeValue, const QString& type) {
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );
    QList<U2DataId> oldStringAttributeList = con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,type,os);
    CHECK_OP(os, );
    if(!oldStringAttributeList.isEmpty()){
        con.dbi->getAttributeDbi()->removeObjectAttributes(oldStringAttributeList.first(),os);
        CHECK_OP(os, );
    }
    U2StringAttribute newStringAttribute(entityRef.entityId, type, newStringAttributeValue);
    con.dbi->getAttributeDbi()->createStringAttribute(newStringAttribute, os);
    CHECK_OP(os, );
}

qint64 U2SequenceObject::getIntegerAttribute(const QString& seqAttr) const {
    return getSequenceInfo().value(seqAttr).toInt();
}

void U2SequenceObject::setIntegerAttribute(int newIntegerAttributeValue, const QString& type) {
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );
    QList<U2DataId> oldIntegerAttributeList = con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,type,os);
    CHECK_OP(os, );
    if(!oldIntegerAttributeList.isEmpty()){
        con.dbi->getAttributeDbi()->removeObjectAttributes(oldIntegerAttributeList.first(),os);
        CHECK_OP(os, );
    }
    U2IntegerAttribute newIntegerAttribute(entityRef.entityId, type, newIntegerAttributeValue);
    con.dbi->getAttributeDbi()->createIntegerAttribute(newIntegerAttribute, os);
    CHECK_OP(os, );
}

double U2SequenceObject::getRealAttribute(const QString& seqAttr) const {
    return getSequenceInfo().value(seqAttr).toReal();
}

void U2SequenceObject::setRealAttribute(double newRealAttributeValue, const QString& type) {
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );
    QList<U2DataId> oldRealAttributeList = con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,type,os);
    CHECK_OP(os, );
    if(!oldRealAttributeList.isEmpty()){
        con.dbi->getAttributeDbi()->removeObjectAttributes(oldRealAttributeList.first(),os);
        CHECK_OP(os, );
    }
    U2RealAttribute newRealAttribute(entityRef.entityId, type, newRealAttributeValue);
    con.dbi->getAttributeDbi()->createRealAttribute(newRealAttribute, os);
    CHECK_OP(os, );
}

QByteArray U2SequenceObject::getByteArrayAttribute(const QString& seqAttr) const {
    return getSequenceInfo().value(seqAttr).toByteArray();
}

void U2SequenceObject::setByteArrayAttribute(const QByteArray& newByteArrayAttributeValue, const QString& type) {
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );
    QList<U2DataId> oldByteArrayAttributeList = con.dbi->getAttributeDbi()->getObjectAttributes(entityRef.entityId,type,os);
    CHECK_OP(os, );
    if(!oldByteArrayAttributeList.isEmpty()){
        con.dbi->getAttributeDbi()->removeObjectAttributes(oldByteArrayAttributeList.first(),os);
        CHECK_OP(os, );
    }
    U2ByteArrayAttribute newByteArrayAttribute(entityRef.entityId, type, newByteArrayAttributeValue);
    con.dbi->getAttributeDbi()->createByteArrayAttribute(newByteArrayAttribute, os);
}



}//namespace


