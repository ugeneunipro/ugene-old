#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>

#include <U2Formats/SAMFormat.h>

#include "StreamSequenceWriter.h"

namespace U2 {

bool StreamContigWriter::writeNextAlignedRead( int offset, const DNASequence& seq )
{
    bool writeOk = format->storeAlignedRead(offset, seq, io, refSeqName, refSeqLength, numSeqWritten == 0);
    if (writeOk) {
        ++numSeqWritten;
        return true;
    }

    return false;

}

StreamContigWriter::StreamContigWriter(const GUrl& url, const QString& refName , int refLength ) 
: numSeqWritten(0), refSeqLength(refLength), refSeqName(refName)
{
    DocumentFormat* f = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::SAM);
    format = qobject_cast<SAMFormat*> (f);
    assert(format != NULL);

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    io = iof->createIOAdapter();
    bool res = io->open(url, IOAdapterMode_Write);
    assert(res == true);
    Q_UNUSED(res);

}

void StreamContigWriter::close()
{
    io->close();
}

StreamContigWriter::~StreamContigWriter()
{
    delete io;
}




} //namespace 

