#ifndef _U2_STREAM_SEQUENCE_READER_H_
#define _U2_STREAM_SEQUENCE_READER_H_

#include <QtCore/QList>
#include <QtCore/QString>

#include <U2Core/GUrl.h>
#include <U2Core/Task.h>
#include <U2Core/DNASequenceObject.h>

#include <memory>

namespace U2 {

class Document;
class DocumentFormat;
class IOAdapter;

/** 
*
* Class provides stream reading for large sequence files.
* For example, dna assembly short reads usually are 
* of size 1GB and more, it is impossible to store whole file in RAM.
* Note, that document format has to support DocumentReadMode_SingleObject
* to be read by StreamSequenceReader.
* In case of multiple files, they will be read subsequently.
*
*/ 

class U2FORMATS_EXPORT StreamSequenceReader {
    struct ReaderContext {
        ReaderContext() : io(NULL), format(NULL) {}
        IOAdapter* io;
        DocumentFormat* format;
    };
    QList<ReaderContext> readers;
    int currentReaderIndex;
    std::auto_ptr<Document> currentDoc;
    bool errorOccured;
    bool lookupPerformed;
    QString errorMessage;
    TaskStateInfo taskInfo;

public:
    StreamSequenceReader();
    ~StreamSequenceReader();
    bool init(const QList<GUrl>& urls);
    bool hasNext();
    bool hasErrors() { return errorOccured; }
    int getProgress();
    QString getErrorMessage();
    const DNASequenceObject* getNextSequenceObject();
};


} //namespace

#endif //_U2_STREAM_SEQUENCE_READER_H_
