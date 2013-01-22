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

#include "GenomeAlignerIO.h"

#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2CoreAttributes.h>
#include <U2Core/Timer.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>

#include <U2Formats/DocumentFormatUtils.h>

namespace U2 {

/************************************************************************/
/* GenomeAlignerUrlReader                                               */
/************************************************************************/

GenomeAlignerUrlReader::GenomeAlignerUrlReader(const QList<GUrl> &dnaList) {
    initOk = reader.init(dnaList);
}

bool GenomeAlignerUrlReader::isEnd() {
    if (!initOk) {
        return true;
    }
    return !reader.hasNext();
}

int GenomeAlignerUrlReader::getProgress() {
    return reader.getProgress();
}

SearchQuery *GenomeAlignerUrlReader::read() {
    return new SearchQuery(reader.getNextSequenceObject());
}

/************************************************************************/
/* GenomeAlignerUrlWriter                                               */
/************************************************************************/

GenomeAlignerUrlWriter::GenomeAlignerUrlWriter(const GUrl &resultFile, const QString &refName, int refLength)
    :seqWriter(resultFile, refName, refLength)
{
    writtenReadsCount = 0;
}

void GenomeAlignerUrlWriter::write(SearchQuery *seq, SAType offset) {
    seqWriter.writeNextAlignedRead(offset, DNASequence(seq->getName(), seq->constSequence()));
    writtenReadsCount++;
}

void GenomeAlignerUrlWriter::close() {
    seqWriter.close();
}

void GenomeAlignerUrlWriter::setReferenceName(const QString &refName) {
    this->refName = refName;
    seqWriter.setRefSeqName(refName);
}

namespace LocalWorkflow {

/************************************************************************/
/* GenomeAlignerCommunicationChanelReader                               */
/************************************************************************/

GenomeAlignerCommunicationChanelReader::GenomeAlignerCommunicationChanelReader(CommunicationChannel* reads) {
    assert(reads != NULL);
    this->reads = reads;
}

bool GenomeAlignerCommunicationChanelReader::isEnd() {
    return !reads->hasMessage() || reads->isEnded();
}

int GenomeAlignerCommunicationChanelReader::getProgress() {
    return 100;
}

SearchQuery *GenomeAlignerCommunicationChanelReader::read() {
    DNASequence seq = reads->get().getData().toMap().value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<DNASequence>();
    
    return new SearchQuery(&seq);
}

GenomeAlignerCommunicationChanelReader::~GenomeAlignerCommunicationChanelReader() {
}

/************************************************************************/
/* GenomeAlignerMAlignmentWriter                                        */
/************************************************************************/
GenomeAlignerMAlignmentWriter::GenomeAlignerMAlignmentWriter() {
    writtenReadsCount = 0;
}

void GenomeAlignerMAlignmentWriter::close() { 
    //TODO: add some heuristic alphabet selection. 
    result.setAlphabet(AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT()));
}

MAlignment& GenomeAlignerMAlignmentWriter::getResult() {
    return result;
}

void GenomeAlignerMAlignmentWriter::write(SearchQuery *seq, SAType offset) {
    U2OpStatus2Log os;
    //if (seq->hasQuality() && seq->getQuality().qualCodes.length() > 0) {
    //    row.setQuality(seq->getQuality());
    //}
    QByteArray offsetGaps;
    offsetGaps.fill(MAlignment_GapChar, offset);
    QByteArray seqWithOffset = seq->constSequence();
    seqWithOffset.prepend(offsetGaps);
    result.addRow(seq->getName(), seqWithOffset, os);
    writtenReadsCount++;
}

void GenomeAlignerMAlignmentWriter::setReferenceName(const QString &refName) {
    this->refName = refName;
    result.setName(refName);
}

} //LocalWorkflow

/************************************************************************/
/* GenomeAlignerDbiReader                                               */
/************************************************************************/
const qint64 GenomeAlignerDbiReader::readBunchSize = 1000;

GenomeAlignerDbiReader::GenomeAlignerDbiReader(U2AssemblyDbi *_rDbi, U2Assembly _assembly)
: rDbi(_rDbi), assembly(_assembly)
{
    wholeAssembly.startPos = 0;
    wholeAssembly.length = rDbi->getMaxEndPos(assembly.id, status) + 1;
    currentRead = reads.end();
    readNumber = 0;
    maxRow = rDbi->getMaxPackedRow(assembly.id, wholeAssembly, status);

    readsInAssembly = rDbi->countReads(assembly.id, wholeAssembly, status);
    if (readsInAssembly <= 0 || status.hasError()) {
        uiLog.error(QString("Genome Aligner -> Database Error: " + status.getError()).toAscii().data());
        end = true;
        return;
    }

    end = false;
}

SearchQuery *GenomeAlignerDbiReader::read() {
    if (end) {
        return NULL;
    }
    reads.clear();
    if (dbiIterator.get() == NULL) {
        dbiIterator.reset(rDbi->getReads(assembly.id, wholeAssembly, status));
    }
    if (dbiIterator->hasNext()) {
        U2AssemblyRead read = dbiIterator->next();
        readNumber++;
        return new SearchQuery(read);
    } else {
        end = true;
        return NULL;
    }
}

bool GenomeAlignerDbiReader::isEnd() {
    return end;
}

int GenomeAlignerDbiReader::getProgress() {
    return (int)(100*(double)readNumber/readsInAssembly);
}

/************************************************************************/
/* GenomeAlignerDbiWriter                                               */
/************************************************************************/
const qint64 GenomeAlignerDbiWriter::readBunchSize = 10000;

inline void checkOperationStatus(const U2OpStatus &status) {
    if (status.hasError()) {
        throw status.getError();
    }
}

GenomeAlignerDbiWriter::GenomeAlignerDbiWriter(QString dbiFilePath, QString refName, int refLength) {
    //TODO: support several assemblies.
    dbiHandle = QSharedPointer<DbiConnection>(new DbiConnection(U2DbiRef(SQLITE_DBI_ID, dbiFilePath), true, status));
    checkOperationStatus(status);
    sqliteDbi = dbiHandle->dbi;
    wDbi = sqliteDbi->getAssemblyDbi();

    sqliteDbi->getObjectDbi()->createFolder("/", status);
    checkOperationStatus(status);
    assembly.visualName = refName;
    U2AssemblyReadsImportInfo importInfo;
    wDbi->createAssemblyObject(assembly, "/", NULL, importInfo, status);
    checkOperationStatus(status);

    U2IntegerAttribute lenAttr;
    lenAttr.objectId = assembly.id;
    lenAttr.name = U2BaseAttributeName::reference_length;
    lenAttr.version = 1;
    lenAttr.value = refLength;
    dbiHandle->dbi->getAttributeDbi()->createIntegerAttribute(lenAttr, status);
}

void GenomeAlignerDbiWriter::write(SearchQuery *seq, SAType offset) {
    U2AssemblyRead read(new U2AssemblyReadData());

    read->name = seq->getName().toAscii();
    read->leftmostPos = offset;
    read->effectiveLen = seq->length();
    read->readSequence = seq->constSequence();
    read->quality = seq->hasQuality() ? seq->getQuality().qualCodes : "";
    read->flags = None;
    read->cigar.append(U2CigarToken(U2CigarOp_M, seq->length()));

    reads.append(read);
    if (reads.size() >= readBunchSize) {
        BufferedDbiIterator<U2AssemblyRead> readsIterator(reads);
        wDbi->addReads(assembly.id, &readsIterator, status);
        checkOperationStatus(status);
        reads.clear();
    }
}

void GenomeAlignerDbiWriter::close() {
    if (reads.size() > 0) {
        BufferedDbiIterator<U2AssemblyRead> readsIterator(reads);
        wDbi->addReads(assembly.id, &readsIterator, status);
        checkOperationStatus(status);
        reads.clear();
    }

    U2AssemblyPackStat packStatus;
    wDbi->pack(assembly.id, packStatus, status);
    checkOperationStatus(status);
    sqliteDbi->flush(status);
}

} //U2
