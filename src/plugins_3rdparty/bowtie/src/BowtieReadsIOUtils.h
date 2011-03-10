#ifndef _BOWTIE_IO_READS_READER_H_
#define _BOWTIE_IO_READS_READER_H_

#include <U2Core/DNASequence.h>
#include <QtCore/QMetaType>

namespace U2 {

/************************************************************************/
/* Abstract short reads reader and writer                               */
/************************************************************************/

class BowtieReadsReader {
public:
	virtual DNASequence read()=0;
	virtual bool isEnd()=0;
};

class BowtieReadsWriter {
public:
	virtual void write(const DNASequence& seq, int offset)=0;
	virtual void close()=0;
};

/************************************************************************/
/* Abstract container for store in QVartiant                            */
/************************************************************************/
class BowtieReadsReaderContainer {
public:
	BowtieReadsReaderContainer() : reader(NULL) { }
	BowtieReadsReaderContainer(BowtieReadsReader* reader) {
		this->reader = reader;
	}
	BowtieReadsReader* reader;
};

class BowtieReadsWriterContainer {
public:
	BowtieReadsWriterContainer() : writer(NULL) {}
	BowtieReadsWriterContainer(BowtieReadsWriter* writer) {
		this->writer = writer;
	}
	BowtieReadsWriter* writer;
};

} //namespace

Q_DECLARE_METATYPE( U2::BowtieReadsReaderContainer )
Q_DECLARE_METATYPE( U2::BowtieReadsWriterContainer )

#endif //_BOWTIE_IO_READS_READER_H_