#ifndef _U2_CLUSTAL_W_ALN_FORMAT_H_
#define _U2_CLUSTAL_W_ALN_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class IOAdapter;

class U2FORMATS_EXPORT ClustalWAlnFormat : public DocumentFormat {
    Q_OBJECT
public:
    static const QByteArray CLUSTAL_HEADER;
    
    ClustalWAlnFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::CLUSTAL_ALN;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual void storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io );

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

private:
    void save(IOAdapter* io, Document* doc, TaskStateInfo& ti);
    void load(IOAdapter* io, QList<GObject*>& objects, TaskStateInfo& ti);

    QString formatName;
};

}//namespace

#endif
