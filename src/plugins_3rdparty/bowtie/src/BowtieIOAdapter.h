#ifndef _BOWTIE_IO_ADAPTER_H_
#define _BOWTIE_IO_ADAPTER_H_

#include "bowtie/pat.h"
#include "bowtie/hit.h"
#include "BowtieReadsIOUtils.h"

#include <U2Core/GUrl.h>
#include <U2Formats/StreamSequenceReader.h>
#include <U2Formats/StreamSequenceWriter.h>

#include <QtCore/QMutex>

namespace U2 {
	class MAlignment;
	class MAlignmentRow;
	class DNASequence;
}

/************************************************************************/
/* DNAReadsReader                                                       */
/************************************************************************/

class BowtieUrlReadsReader : public U2::BowtieReadsReader {
public:
	BowtieUrlReadsReader(const QList<U2::GUrl>& dnaList);
	U2::DNASequence read();
	bool isEnd();
private:
	bool initOk;
	U2::StreamSequenceReader reader;
};

class BowtieUrlReadsWriter : public U2::BowtieReadsWriter {
public:
	BowtieUrlReadsWriter(const U2::GUrl& resultFile, const QString& refName, int refLength);
	void write(const U2::DNASequence& seq, int offset);
	void close();
private:
	U2::StreamContigWriter seqWriter;
};

/************************************************************************/
/* DNASequencesPatternSource                                            */
/************************************************************************/

class DNASequencesPatternSource: public PatternSource {
public:
	DNASequencesPatternSource(uint32_t seed, U2::BowtieReadsReader* reader);
	void nextReadImpl(ReadBuf& r, uint32_t& patid);
	void nextReadPairImpl(ReadBuf& ra, ReadBuf& rb, uint32_t& patid);
private:
	U2::BowtieReadsReader* reader;
	QMutex mutex;
};

/************************************************************************/
/* MAlignmentHitSink                                                    */
/************************************************************************/


class MAlignmentHitSink: public HitSink {
public:
	MAlignmentHitSink(U2::BowtieReadsWriter* writer, OutFileBuf* out, DECL_HIT_DUMPS, bool onePairFile, bool sampleMax, RecalTable *table, vector<string>* refnames = NULL);
	void append(ostream& o, const Hit& h);
	void commitResultMA();
private:
	U2::BowtieReadsWriter* writer;
	QMutex mutex;
};

#endif // _BOWTIE_IO_ADAPTER_H_
