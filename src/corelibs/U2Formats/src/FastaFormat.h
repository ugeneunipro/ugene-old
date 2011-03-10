#ifndef _U2_FASTA_FORMAT_H_
#define _U2_FASTA_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class IOAdapter;

class U2FORMATS_EXPORT  FastaFormat : public DocumentFormat {
    Q_OBJECT
public:
    FastaFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::PLAIN_FASTA;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);
    
    virtual void storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io );

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

private:
    
    QString formatName;
};

}//namespace

#endif
