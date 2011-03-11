#include <U2Core/AppContext.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Formats/DocumentFormatUtils.h>
#include "GenomeAlignerIO.h"

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

}

void GenomeAlignerUrlWriter::write(const DNASequence &seq, int offset) {
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
/* GenomeAlignerMAlignmentWriter                                               */
/************************************************************************/
GenomeAlignerMAlignmentWriter::GenomeAlignerMAlignmentWriter()
{

}

void GenomeAlignerMAlignmentWriter::close() { 
    //TODO: add some heuristic alphabet selection. 
    result.setAlphabet(AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT()));
}

MAlignment& GenomeAlignerMAlignmentWriter::getResult() {
    return result;
}

void GenomeAlignerMAlignmentWriter::write(const DNASequence& seq, int offset) {
    MAlignmentRow row;
    row.setName(seq.getName());
    row.setSequence(seq.seq, offset);
    if (seq.quality.qualCodes.length() > 0) {
        row.setQuality(seq.quality);
    }
    result.addRow(row);
}

void GenomeAlignerMAlignmentWriter::setReferenceName(const QString &refName) {
    this->refName = refName;
    result.setName(refName);
}

} //LocalWorkflow
} //U2
