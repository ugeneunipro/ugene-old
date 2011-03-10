#ifndef _U2_GENBANK_PLAIN_TEXT_FORMAT_H_
#define _U2_GENBANK_PLAIN_TEXT_FORMAT_H_

#include "EMBLGenbankAbstractDocument.h"

namespace U2 {

class U2FORMATS_EXPORT GenbankPlainTextFormat : public EMBLGenbankAbstractDocument {
    Q_OBJECT
public:
    GenbankPlainTextFormat(QObject* p);

    virtual void storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io );
    
    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

protected:

    bool readIdLine(ParserState*);
    bool readEntry(QByteArray&, ParserState*);
    //void readAnnotations(ParserState*, int offset);
};


}//namespace

#endif
