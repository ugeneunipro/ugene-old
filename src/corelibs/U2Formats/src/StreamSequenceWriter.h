#ifndef _U2_STREAM_SEQUENCE_WRITER_H_
#define _U2_STREAM_SEQUENCE_WRITER_H_

#include <QtCore/QList>
#include <QtCore/QString>

#include <U2Core/GUrl.h>
#include <U2Core/DNASequenceObject.h>

namespace U2 {

class Document;
class IOAdapter;
class SAMFormat;

/** 
*
* Class provides stream writing for large alignment files.
* It supports only SAM format.
* 
*/ 

class U2FORMATS_EXPORT StreamContigWriter {
    IOAdapter* io;
    SAMFormat* format;
    int numSeqWritten;
    int refSeqLength;
    QString refSeqName;

public:
    StreamContigWriter(const GUrl& url, const QString& refName = QString(), int refLength = 0);
    ~StreamContigWriter();
    bool writeNextAlignedRead(int offset, const DNASequence& seq);
    void setRefSeqLength(int l) { refSeqLength = l;}
    void setRefSeqName(const QString& name ) { refSeqName = name; }
    int getNumSeqWritten() { return numSeqWritten; }
    void close();
};


} //namespace

#endif //_U2_STREAM_SEQUENCE_WRITER_H_
