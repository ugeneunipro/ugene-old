#ifndef _U2_ABI_FORMAT_H_
#define _U2_ABI_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class IOAdapter;
class SeekableBuf;

class U2FORMATS_EXPORT  ABIFormat : public DocumentFormat {
    Q_OBJECT
public:
    ABIFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::ABIF;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;
    
private:
    Document* parseABI(SeekableBuf*, IOAdapter* io, const QVariantMap& fs);
    QString formatName;
};

}//namespace

#endif
