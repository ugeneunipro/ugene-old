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

#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/DNASequence.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/SequenceUtils.h>
#include <U2Core/U1AnnotationUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2AttributeDbi.h>

#include "U2SequenceUtils.h"

namespace U2 {

const QString U2SequenceDbiHints::UPDATE_SEQUENCE_LENGTH = "update-length";
const QString U2SequenceDbiHints::EMPTY_SEQUENCE = "empty-sequence";

DNAAlphabetType U2SequenceUtils::alphabetType(const U2EntityRef& ref, U2OpStatus& os) {
    DNAAlphabetType res = DNAAlphabet_RAW;
    DbiConnection con(ref.dbiRef, os);
    
    U2Sequence seq = con.dbi->getSequenceDbi()->getSequenceObject(ref.entityId, os);
    CHECK_OP(os, res);
    
    DNAAlphabet* al = AppContext::getDNAAlphabetRegistry()->findById(seq.alphabet.id);
    CHECK_EXT(al != NULL, os.setError(tr("Alphabet is not found!")), res);
    
    return al->getType();
}


qint64 U2SequenceUtils::length(const U2EntityRef& ref, U2OpStatus& os) {
    DbiConnection con(ref.dbiRef, os);

    U2Sequence seq = con.dbi->getSequenceDbi()->getSequenceObject(ref.entityId, os);
    CHECK_OP(os, -1);

    return seq.length;
}

U2Sequence U2SequenceUtils::copySequence(const DNASequence& srcSeq, const U2DbiRef& dstDbi, U2OpStatus& os) {
    U2Sequence res;
    res.alphabet = srcSeq.alphabet->getId();
    res.circular = srcSeq.circular;
    res.length = srcSeq.length();
    res.visualName = srcSeq.getName();

    DbiConnection dstCon(dstDbi, os);
    CHECK_OP(os, res);
    dstCon.dbi->getSequenceDbi()->createSequenceObject(res, QString(), os);
    CHECK_OP(os, res);

    dstCon.dbi->getSequenceDbi()->updateSequenceData(res.id, U2Region(0, 0), srcSeq.seq, QVariantMap(), os);
    CHECK_OP(os, res);

    return res;
}

U2Sequence U2SequenceUtils::copySequence(const U2EntityRef& srcSeq, const U2DbiRef& dstDbi, U2OpStatus& os) {
    //TODO: use small blocks
    U2Sequence res;
    DbiConnection srcCon(srcSeq.dbiRef, os);
    CHECK_OP(os, res);
    
    U2Sequence seq = srcCon.dbi->getSequenceDbi()->getSequenceObject(srcSeq.entityId, os);
    CHECK_OP(os, res);

    res = seq;
    res.id.clear();
    res.length = 0;

    DbiConnection dstCon(dstDbi, os);
    CHECK_OP(os, res);
    dstCon.dbi->getSequenceDbi()->createSequenceObject(res, QString(), os);
    CHECK_OP(os, res);

    //TODO: optimize and insert by small chunks! ~2-4 mb
    QByteArray wholeSeq = srcCon.dbi->getSequenceDbi()->getSequenceData(srcSeq.entityId, U2_REGION_MAX, os);
    CHECK_OP(os, res);
    QVariantMap hints;
    dstCon.dbi->getSequenceDbi()->updateSequenceData(res.id, U2Region(0, 0), wholeSeq, hints, os);
    CHECK_OP(os, res);
    res.length += wholeSeq.length();
   
    return res;
}


static QList<QByteArray> _extractRegions(const U2EntityRef& seqRef, const QVector<U2Region>& regions, DNATranslation* complTT, U2OpStatus& os) 
{
    QList<QByteArray> res;

    DbiConnection con(seqRef.dbiRef, os);
    CHECK_OP(os, res);
    U2SequenceDbi* seqDbi = con.dbi->getSequenceDbi();
    U2Sequence seq = seqDbi->getSequenceObject(seqRef.entityId, os);
    CHECK_OP(os, res);

    QVector<U2Region> safeLocation = regions;
    U2Region::bound(0, seq.length, safeLocation);

    for (int i = 0, n = safeLocation.size(); i < n; i++) {
        const U2Region& oReg = safeLocation.at(i);
        if (complTT == NULL) {
            QByteArray part = seqDbi->getSequenceData(seq.id, U2Region(oReg.startPos, oReg.length) , os);
            CHECK_OP(os, res);
            res.append(part);
        } else {
            QByteArray arr = seqDbi->getSequenceData(seq.id, U2Region(oReg.startPos, oReg.length), os);
            CHECK_OP(os, res);
            TextUtils::reverse(arr.data(), arr.length());
            complTT->translate(arr.data(), arr.length());
            res.prepend(arr);
        }
    }
    return res;
}

QList<QByteArray> U2SequenceUtils::extractRegions(const U2EntityRef& seqRef, const QVector<U2Region>& origLocation, 
                                                DNATranslation* complTT, DNATranslation* aminoTT, bool join, U2OpStatus& os)
{
    QList<QByteArray> res = _extractRegions(seqRef, origLocation, complTT, os);
    CHECK_OP(os, res)

        DbiConnection con(seqRef.dbiRef, os);
    CHECK_OP(os, res);

    U2SequenceDbi* seqDbi = con.dbi->getSequenceDbi();
    U2Sequence seq = seqDbi->getSequenceObject(seqRef.entityId, os);
    CHECK_OP(os, res);

    if (seq.circular && res.size() > 1) {
        const U2Region& firstL = origLocation.first();
        const U2Region& lastL = origLocation.last();
        if (firstL.startPos == 0 && lastL.endPos() == seq.length) { 
            QByteArray lastS = res.last();
            QByteArray firstS = res.first();
            res.removeLast();
            res[0] = lastS.append(firstS);
        }
    }
    if (aminoTT != NULL) {
        res = U1SequenceUtils::translateRegions(res, aminoTT, join);
    } 

    if (join && res.size() > 1) {
        QByteArray joined = U1SequenceUtils::joinRegions(res);
        res.clear();
        res.append(joined);
    }

    return res;
}


U2EntityRef U2SequenceUtils::import(const U2DbiRef& dbiRef, const DNASequence& seq, U2OpStatus& os) {

    U2EntityRef res;
    U2SequenceImporter i;
    
    i.startSequence(dbiRef, seq.getName(), seq.circular, os);
    CHECK_OP(os, res);
    
    i.addBlock(seq.constData(), seq.length(), os);
    CHECK_OP(os, res);
    
    U2Sequence u2seq = i.finalizeSequence(os);
    CHECK_OP(os, res);
    
    res.dbiRef = dbiRef;
    res.entityId = u2seq.id;

    setQuality(res, seq.quality);

    return res;
}

void U2SequenceUtils::setQuality(const U2EntityRef& entityRef, const DNAQuality& q) {
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

//////////////////////////////////////////////////////////////////////////
// U2SequenceImporter
#define DEFAULT_SEQUENCE_INSERT_BLOCK_SIZE (4*1024*1024)


/**
 * If GObjectHint_CaseAnns is present in the QVariantMap (it should be stored as int),
 * then the method verifies the value and returns it (if it is correct).
 * Otherwise, returns NO_CASE_ANNS.
 */
static CaseAnnotationsMode getCaseAnnotationsModeHint(const QVariantMap& fs)
{
    if (fs.keys().contains(GObjectHint_CaseAnns))
    {
        QVariant caseAnnsVariant = fs.value(GObjectHint_CaseAnns);
        SAFE_POINT(caseAnnsVariant.canConvert<int>(), "Can't convert a case annotations hint!", NO_CASE_ANNS);

        bool conversionIsOK = 0;
        int caseAnnsInt = caseAnnsVariant.toInt(&conversionIsOK);
        SAFE_POINT(conversionIsOK, "Can't convert a case annotations hint to int!", NO_CASE_ANNS);

        SAFE_POINT((caseAnnsInt == LOWER_CASE) || (caseAnnsInt == UPPER_CASE) || (caseAnnsInt == NO_CASE_ANNS),
            "Incorrect value of a case annotation hint!",
            NO_CASE_ANNS);

        return (CaseAnnotationsMode)caseAnnsInt;
    }

    return NO_CASE_ANNS;
}


U2SequenceImporter::U2SequenceImporter(const QVariantMap& fs, bool lazyMode, bool singleThread)
: lazyMode(lazyMode), singleThread(singleThread), sequenceCreated(false)
{
    insertBlockSize = DEFAULT_SEQUENCE_INSERT_BLOCK_SIZE;
    currentLength = 0;
    isUnfinishedRegion = false;
    caseAnnsMode = getCaseAnnotationsModeHint(fs);
    sequenceCreated = false;
    committedLength = 0;
}

U2SequenceImporter::U2SequenceImporter(qint64 _insertBlockSize, const QVariantMap& fs, bool lazyMode, bool singleThread)
: insertBlockSize(_insertBlockSize), lazyMode(lazyMode), singleThread(singleThread)
{
    insertBlockSize = qMin((qint64)10, insertBlockSize);
    currentLength = 0;
    isUnfinishedRegion = false;
    caseAnnsMode = getCaseAnnotationsModeHint(fs);
    sequenceCreated = false;
    committedLength = 0;
}

U2SequenceImporter::~U2SequenceImporter() {
    if (con.isOpen()) {
        coreLog.trace(QString("Removing sequence from unfinished import: %1").arg(sequence.visualName));
        U2OpStatus2Log os;
        con.dbi->getObjectDbi()->removeObject(sequence.id, os);
    }
}


void U2SequenceImporter::startSequence(const U2DbiRef& dbiRef, const QString& visualName, bool circular, U2OpStatus& os) {
    SAFE_POINT(!con.isOpen(), "Connection is already opened!", );
    con.open(dbiRef, true, os);
    CHECK_OP(os, );

    sequence = U2Sequence();
    sequence.visualName = visualName;
    sequence.circular = circular;

    currentLength = 0;
    isUnfinishedRegion = false;
    annList.clear();
    
    if (!lazyMode) {
        con.dbi->getSequenceDbi()->createSequenceObject(sequence, "", os);
        CHECK_OP(os, );
        sequenceCreated = true;
    }
}

void U2SequenceImporter::addBlock(const char* data, qint64 len, U2OpStatus& os) {
    // derive common alphabet
    DNAAlphabet* blockAl = U2AlphabetUtils::findBestAlphabet(data, len);
    CHECK_EXT(blockAl != NULL, os.setError("Failed to match sequence alphabet!"), );

    DNAAlphabet* oldAl = U2AlphabetUtils::getById(sequence.alphabet);
    DNAAlphabet* resAl = blockAl;
    if (oldAl != NULL) {
        if(oldAl->getType() == DNAAlphabet_AMINO && resAl->getType() == DNAAlphabet_NUCL){
            resAl = oldAl;
        }
        else if(resAl->getType() == DNAAlphabet_AMINO && oldAl->getType() == DNAAlphabet_NUCL){
            oldAl = resAl;
        }
        else{
            resAl = U2AlphabetUtils::deriveCommonAlphabet(blockAl, oldAl);
        }
        CHECK_EXT(resAl!=NULL, os.setError(U2SequenceUtils::tr("Failed to derive sequence alphabet!")), );
    } 
    
    if (resAl != U2AlphabetUtils::getById(sequence.alphabet)) {
        sequence.alphabet.id = resAl->getId();
        if (sequenceCreated) {
            con.dbi->getSequenceDbi()->updateSequenceObject(sequence, os);
            CHECK_OP(os, );
        }
    }

    _addBlock2Buffer(data, len, os);

    if (caseAnnsMode != NO_CASE_ANNS) {
        annList << U1AnnotationUtils::getCaseAnnotations(data, len, currentLength, isUnfinishedRegion, unfinishedRegion, LOWER_CASE == caseAnnsMode);
    }
    currentLength += len;
}

void U2SequenceImporter::addSequenceBlock(const U2EntityRef& sequenceRef, const U2Region& r, U2OpStatus& os) {
    _addBuffer2Db(os);
    CHECK_OP(os, );
    DbiConnection con(sequenceRef.dbiRef, os);
    CHECK_OP(os, );
    
    //TODO: optimize -> create utility that uses small to copy sequence!
    QByteArray arr = con.dbi->getSequenceDbi()->getSequenceData(sequenceRef.entityId, r, os);
    CHECK_OP(os, );
    addBlock(arr.constData(), arr.size(), os);
}

void U2SequenceImporter::addDefaultSymbolsBlock(int n, U2OpStatus& os) {
    SAFE_POINT(n >= 0, QString("Invalid number of symbols: %1").arg(n), );
    char defaultChar = U2AlphabetUtils::getDefaultSymbol(sequence.alphabet);
    QByteArray a(n, defaultChar);
    _addBlock2Buffer(a.data(), a.size(), os);
    currentLength += n;
}


void U2SequenceImporter::_addBlock2Buffer(const char* data, qint64 len, U2OpStatus& os) {
    if (len + sequenceBuffer.length() < insertBlockSize) {
        sequenceBuffer.append(data, len);
        return;
    }
    _addBlock2Db(sequenceBuffer.data(), sequenceBuffer.length(), os);
    CHECK_OP(os, );
    sequenceBuffer.clear();
    _addBlock2Db(data, len, os);
}

void U2SequenceImporter::_addBlock2Db(const char* data, qint64 len, U2OpStatus& os) {
    SAFE_POINT(len >= 0, "Illegal block length!", );
    if (len == 0) {
        return;
    }
    QByteArray arr(data, len); 
    TextUtils::translate(TextUtils::UPPER_CASE_MAP, arr.data(), arr.length());

    bool updateLength = true;
    bool emptySequence = false;
    bool justCreated = false;
    if (!sequenceCreated) {
        emptySequence = true;
        if (singleThread) {
            SAFE_POINT(0 == committedLength, "Sequence object is not created, but sequence data already exists", );
            sequence.length = len;
            updateLength = false;
        }
        con.dbi->getSequenceDbi()->createSequenceObject(sequence, "", os);
        CHECK_OP(os, );
        sequenceCreated = true;
        justCreated = true;
    }

    QVariantMap hints;
    hints[U2SequenceDbiHints::UPDATE_SEQUENCE_LENGTH] = updateLength;
    hints[U2SequenceDbiHints::EMPTY_SEQUENCE] = emptySequence;
    U2Region reg(sequence.length, 0);
    if (justCreated) {
        reg.startPos = 0;
        reg.length = 0;
    }
    con.dbi->getSequenceDbi()->updateSequenceData(sequence.id, reg, arr, hints, os);
    CHECK_OP(os, );
    if (committedLength == sequence.length) {
        sequence.length += len;
    } else { // because of lazyMode and delayed sequence creation
        sequence.length = committedLength + len;
    }
    committedLength += len;
}

void U2SequenceImporter::_addBuffer2Db(U2OpStatus& os) {
    CHECK(!sequenceBuffer.isEmpty(), );
    _addBlock2Db(sequenceBuffer.data(), sequenceBuffer.length(), os);
    sequenceBuffer.clear();
}

U2Sequence U2SequenceImporter::finalizeSequence(U2OpStatus& os) {
    _addBuffer2Db(os);
    LOG_OP(os);
    // If sequence is empty, addBlock is never called and alphabet is not set. So set it here to some default value
    if(! sequence.alphabet.isValid() && sequence.version != 0) {
        sequence.alphabet.id = BaseDNAAlphabetIds::RAW();
        con.dbi->getSequenceDbi()->updateSequenceObject(sequence, os);
        LOG_OP(os);
    }
    con.close(os);
    if (caseAnnsMode != NO_CASE_ANNS) {
        annList << U1AnnotationUtils::finalizeUnfinishedRegion(isUnfinishedRegion, unfinishedRegion, LOWER_CASE == caseAnnsMode);

        if (1 == annList.size()) {
            const QVector<U2Region> &regs = annList.first()->getRegions();
            if (1 == regs.size()) {
                U2Region reg = regs.first();
                if (0 == reg.startPos && sequence.length == reg.length) {
                    annList.clear();
                }
            }
        }
    }
    sequenceCreated = false;
    committedLength = 0;
    return sequence;
}

void U2SequenceImporter::setCaseAnnotationsMode(CaseAnnotationsMode mode) {
    caseAnnsMode = mode;
}

bool U2SequenceImporter::isCaseAnnotationsModeOn() const {
    return caseAnnsMode != NO_CASE_ANNS;
}

QList<Annotation*> &U2SequenceImporter::getCaseAnnotations() {
    return annList;
}

qint64 U2SequenceImporter::getCurrentLength() const {
    return currentLength;
}

void U2MemorySequenceImporter::addBlock(const char* data, qint64 len, U2OpStatus& os) {
    if (qstrlen(data) > len) {
        os.setError("Wrong data length in addBlock");
        return;
    }

    // derive common alphabet
    DNAAlphabet* blockAl = U2AlphabetUtils::findBestAlphabet(data, len);
    CHECK_EXT(blockAl != NULL, os.setError("Failed to match sequence alphabet!"), );

    DNAAlphabet* oldAl = U2AlphabetUtils::getById(sequence.alphabet);
    DNAAlphabet* resAl = blockAl;
    if (oldAl != NULL) {
        if(oldAl->getType() == DNAAlphabet_AMINO && resAl->getType() == DNAAlphabet_NUCL){
            resAl = oldAl;
        }
        else if(resAl->getType() == DNAAlphabet_AMINO && oldAl->getType() == DNAAlphabet_NUCL){
            oldAl = resAl;
        }
        else{
            resAl = U2AlphabetUtils::deriveCommonAlphabet(blockAl, oldAl);
        }
        CHECK_EXT(resAl!=NULL, os.setError(U2SequenceUtils::tr("Failed to derive sequence alphabet!")), );
    } 

    if (resAl != U2AlphabetUtils::getById(sequence.alphabet)) {
        sequence.alphabet.id = resAl->getId();
    }

    sequenceData.append(data, len);
}

void U2MemorySequenceImporter::addDefaultSymbolsBlock(int n, U2OpStatus& os) {
    SAFE_POINT(n >= 0, QString("Invalid number of symbols: %1").arg(n), );
    char defaultChar = U2AlphabetUtils::getDefaultSymbol(sequence.alphabet);
    QByteArray a(n, defaultChar);

    sequenceData.append(a);
}

qint64 U2MemorySequenceImporter::getCurrentLength() const {
    return sequenceData.length();
}


} //namespace
