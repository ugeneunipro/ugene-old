#ifndef _U2_SCF_FORMAT_H_
#define _U2_SCF_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class IOAdapter;
class SeekableBuf;
class DNAChromatogram;
class DNASequence;

class U2FORMATS_EXPORT  SCFFormat : public DocumentFormat {
    Q_OBJECT
public:
    SCFFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::SCF;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

    static void exportDocumentToSCF(const QString& fileName, const DNAChromatogram& cd, const DNASequence& dna, TaskStateInfo& ts);

private:
    Document* parseSCF(SeekableBuf*, IOAdapterFactory* io, const GUrl& url, const QVariantMap& fs);

    QString formatName;
};

}//namespace

#endif
