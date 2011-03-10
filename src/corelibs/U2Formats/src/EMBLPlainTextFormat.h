#ifndef _U2_EMBL_PLAIN_TEXT_FORMAT_H_
#define _U2_EMBL_PLAIN_TEXT_FORMAT_H_

#include "EMBLGenbankAbstractDocument.h"

namespace U2 {

class U2FORMATS_EXPORT EMBLPlainTextFormat : public EMBLGenbankAbstractDocument {
    Q_OBJECT
public:
    EMBLPlainTextFormat(QObject* p);

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

protected:

    bool readIdLine(ParserState*);
    bool readEntry(QByteArray&, ParserState*);
    //void readAnnotations(ParserState*, int offset);
    QMap<QString, QString> tagMap;
};


}//namespace

#endif
