#ifndef _U2_SQLITE_DOCUMENT_FORMAT_H_
#define _U2_SQLITE_DOCUMENT_FORMAT_H_

#include <U2Core/U2Dbi.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

/** UGENE v1 wrapper over DBI (UGENE v2 data access) interface */
class U2CORE_EXPORT DbiDocumentFormat: public DocumentFormat {
    Q_OBJECT
public:
    DbiDocumentFormat(const U2DbiFactoryId& id, const DocumentFormatId& formatId, 
        const QString& formatName, const QStringList& exits, QObject* p = NULL);

    virtual DocumentFormatId getFormatId() const {return formatId;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* createNewDocument(IOAdapterFactory* io, const QString& url, const QVariantMap& fs = QVariantMap());

    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual void storeDocument(Document* d, TaskStateInfo& ts, IOAdapter* io);

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& url = GUrl()) const;

private:
    QString             formatName;
    U2DbiFactoryId      id;
    DocumentFormatId    formatId;
};

}//namespace

#endif
