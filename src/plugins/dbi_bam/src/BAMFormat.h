#ifndef _U2_BAM_FORMAT_H_
#define _U2_BAM_FORMAT_H_

#include <U2Core/DocumentModel.h>

namespace U2 {
namespace BAM {

class BAMFormat : public DocumentFormat
{
    Q_OBJECT
public:
    BAMFormat(QObject *parent = NULL);

    virtual DocumentFormatId getFormatId()const;
    virtual const QString &getFormatName()const;
    virtual Document *loadDocument(IOAdapter *io, TaskStateInfo &ti, const QVariantMap &fs, DocumentLoadMode mode = DocumentLoadMode_Whole);
    virtual void storeDocument(Document *d, TaskStateInfo &ts, IOAdapter *io);
    virtual FormatDetectionResult checkRawData(const QByteArray &rawData, const GUrl& url = GUrl())const;
private:
    static const QString FORMAT_ID;
    const QString formatName;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_FORMAT_H_
