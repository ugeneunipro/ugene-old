#ifndef _U2_FASTQ_FORMAT_H_
#define _U2_FASTQ_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <QtCore/QStringList>

namespace U2 {

class IOAdapter;

class U2FORMATS_EXPORT FastqFormat : public DocumentFormat {
    Q_OBJECT
public:
    FastqFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::FASTQ;}

    virtual const QString& getFormatName() const {return fn;}
    
    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);
    
    virtual void storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io );

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

private:
    QString fn;
};

}//namespace

#endif
