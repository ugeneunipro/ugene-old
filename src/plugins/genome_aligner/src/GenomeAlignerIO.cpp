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

const DNASequenceObject *GenomeAlignerUrlReader::read() {
    return reader.getNextSequenceObject();
}

/************************************************************************/
/* GenomeAlignerUrlWriter                                               */
/************************************************************************/

GenomeAlignerUrlWriter::GenomeAlignerUrlWriter(const GUrl &resultFile, const QString &refName)
    :seqWriter(resultFile, refName)
{
    writtenReadsCount = 0;
}

void GenomeAlignerUrlWriter::write(const DNASequence &seq, quint32 offset) {
    seqWriter.writeNextAlignedRead(offset, seq);
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

const DNASequenceObject *GenomeAlignerCommunicationChanelReader::read() {
    DNASequence seq = reads->get().getData().toMap().value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<DNASequence>();
    obj->setSequence(seq);
    
    return obj;
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

void GenomeAlignerMAlignmentWriter::write(const DNASequence& seq, quint32 offset) {
    MAlignmentRow row;
    row.setName(seq.getName());
    row.setSequence(seq.seq, offset);
    if (seq.quality.qualCodes.length() > 0) {
        row.setQuality(seq.quality);
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
    currentIteration = -1;
    currentRead = reads.end();
    readNumber = 0;
    maxRow = rDbi->getMaxPackedRow(assembly.id, wholeAssembly, status);

    qint64 readsInAssembly = rDbi->countReadsAt(assembly.id, wholeAssembly, status);
    if (readsInAssembly <= 0 || status.hasError()) {
        uiLog.error(QString("Genome Aligner -> Database Error: " + status.getError()).toAscii().data());
        end = true;
        return;
    }

    end = false;
}

const DNASequenceObject *GenomeAlignerDbiReader::read() {
    if (end) {
        return NULL;
    }

    if (currentRead == reads.end()) {
        currentIteration ++;
        /*if (currentRow >= maxRow) {
            end = true;
            return NULL;
        }*/
        reads = rDbi->getReadsAt(assembly.id, wholeAssembly, currentIteration*readBunchSize, readBunchSize, status);
        static int count = 0;
        count += reads.size();
        currentRead = reads.begin();

        if (reads.size() <= 0) {
            end = true;
            return NULL;
        }
    }

    U2AssemblyRead &read = *currentRead;
    quint64 idd = read.sequenceId;
    QString seqName = QString("r.%1").arg(readNumber);
    obj->setSequence(DNASequence(seqName, read.readSequence));

    currentRead++;
    readNumber++;
    
    return obj;
}

bool GenomeAlignerDbiReader::isEnd() {
    return end;
}

} //U2
