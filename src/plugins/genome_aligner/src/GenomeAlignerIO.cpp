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

#include <U2Core/AppContext.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Formats/DocumentFormatUtils.h>
#include "GenomeAlignerIO.h"

#include <U2Core/Counter.h>

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

SearchQuery *GenomeAlignerUrlReader::read() {
    return new SearchQuery(reader.getNextSequenceObject()->getDNASequence());
}

/************************************************************************/
/* GenomeAlignerUrlWriter                                               */
/************************************************************************/

GenomeAlignerUrlWriter::GenomeAlignerUrlWriter(const GUrl &resultFile, const QString &refName)
    :seqWriter(resultFile, refName)
{
    writtenReadsCount = 0;
}

void GenomeAlignerUrlWriter::write(SearchQuery *seq, quint32 offset) {
    seqWriter.writeNextAlignedRead(offset, seq->getSequence());
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
    obj = new DNASequenceObject("obj", DNASequence(QByteArray("aaa"), DocumentFormatUtils::findAlphabet("aaa")));
}

bool GenomeAlignerCommunicationChanelReader::isEnd() {
    return !reads->hasMessage() || reads->isEnded();
}

SearchQuery *GenomeAlignerCommunicationChanelReader::read() {
    DNASequence seq = reads->get().getData().toMap().value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<DNASequence>();
    
    return new SearchQuery(seq);
}

GenomeAlignerCommunicationChanelReader::~GenomeAlignerCommunicationChanelReader() {
    delete obj;
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

void GenomeAlignerMAlignmentWriter::write(SearchQuery *seq, quint32 offset) {
    MAlignmentRow row;
    row.setName(seq->getName());
    row.setSequence(seq->constSequence(), offset);
    if (seq->getQuality().qualCodes.length() > 0) {
        row.setQuality(seq->getQuality());
    }
    result.addRow(row);
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
    obj = new DNASequenceObject("obj", DNASequence(QByteArray("aaa"), DocumentFormatUtils::findAlphabet("aaa")));
    wholeAssembly.startPos = 0;
    wholeAssembly.length = rDbi->getMaxEndPos(assembly.id, status);
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
    if (currentRead == reads.end()) {
        for (int i=0; i < readBunchSize && dbiIterator->hasNext(); i++) {
            reads.append(dbiIterator->next());
        }
        if (reads.isEmpty()) {
            end = true;
            return NULL;
        }
        currentRead = reads.begin();
    }
    
    U2AssemblyRead &read = *currentRead;

    currentRead++;
    readNumber++;
    
    return new SearchQuery(read);
}

bool GenomeAlignerDbiReader::isEnd() {
    return end;
}

/************************************************************************/
/* GenomeAlignerDbiWriter                                               */
/************************************************************************/
const qint64 GenomeAlignerDbiWriter::readBunchSize = 1000;

GenomeAlignerDbiWriter::GenomeAlignerDbiWriter(U2AssemblyDbi *_wDbi, U2Assembly _assembly)
: wDbi(_wDbi), assembly(_assembly)
{
    wholeAssembly.startPos = 0;
    wholeAssembly.length = wDbi->getMaxEndPos(assembly.id, status);
    maxRow = wDbi->getMaxPackedRow(assembly.id, wholeAssembly, status);
    readsInAssembly = wDbi->countReads(assembly.id, wholeAssembly, status);
    currentRow = maxRow;
}

void GenomeAlignerDbiWriter::write(SearchQuery *seq, quint32 offset) {
    U2AssemblyRead read(new U2AssemblyReadData());
    read->readSequence = seq->constSequence();
    read->leftmostPos = offset;
    read->cigar.append(U2CigarToken(U2CigarOp_M, seq->length()));
    read->packedViewRow = currentRow;
    currentRow++;

    reads.append(read);
    if (reads.size() >= readBunchSize) {
        wDbi->addReads(assembly.id, reads, status);
        reads.clear();
    }
}

void GenomeAlignerDbiWriter::close() {
    if (reads.size() > 0) {
        wDbi->addReads(assembly.id, reads, status);
        reads.clear();
    }


    /* TODO: what this all about?
    QList<U2DataId> ids;
    qint64 toRead = 0;
    for (qint64 count = 0; count < readsInAssembly;) {
        toRead = qMin((readsInAssembly - count), readBunchSize);
        ids = wDbi->getReadIds(assembly.id, wholeAssembly, 0, toRead, status);
        count += toRead;
        wDbi->removeReads(assembly.id, ids.mid(0, toRead), status);
        ids.clear();
    }
    //wDbi->pack(assembly.id, status);
    */
}

bool checkAndLogError(const U2OpStatusImpl & status) {
    if(status.hasError()) {
        uiLog.error(QString(QString("Genome Aligner -> Database Error: " + status.getError()).toAscii().data()));
    }
    return status.hasError();
}

} //U2
