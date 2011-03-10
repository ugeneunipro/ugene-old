#ifndef _U2_GFF_FORMAT_H_
#define _U2_GFF_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class IOAdapter;

class U2FORMATS_EXPORT  GFFFormat : public DocumentFormat {
    Q_OBJECT
public:
    GFFFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::GFF;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    void load(IOAdapter* io, QList<GObject*>& objects, TaskStateInfo& si);

    virtual void storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io );

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

private:
    QStringList parseLine(QString line) const;

    QString formatName;
};

}//namespace

#endif
