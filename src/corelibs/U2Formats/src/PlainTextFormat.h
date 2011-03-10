#ifndef _U2_PLAIN_TEXT_FORMAT_H_
#define _U2_PLAIN_TEXT_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class U2FORMATS_EXPORT PlainTextFormat : public DocumentFormat {
    Q_OBJECT
public:
    PlainTextFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::PLAIN_TEXT;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* createNewDocument(IOAdapterFactory* io, const QString& url, const QVariantMap& fs = QVariantMap());

    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual void storeDocument(Document* d, TaskStateInfo& ts, IOAdapter* io);

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

    static void storeRawData(const QByteArray& data, TaskStateInfo& ts, IOAdapter* io);

private:
    QString formatName;
};

}//namespace

#endif
