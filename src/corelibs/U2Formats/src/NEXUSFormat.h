#ifndef _U2_NEXUS_FORMAT_H_
#define _U2_NEXUS_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

#include <U2Core/IOAdapter.h>
#include <U2Core/Task.h>

#include <cassert>

namespace U2 
{

class IOAdapter;

class U2FORMATS_EXPORT NEXUSFormat : public DocumentFormat 
{
    Q_OBJECT

public:
    NEXUSFormat(QObject *p);

    virtual DocumentFormatId getFormatId() const { return BaseDocumentFormats::NEXUS; }
    virtual const QString& getFormatName() const { return formatName; }

    virtual FormatDetectionResult checkRawData(const QByteArray &rawData, const GUrl& = GUrl()) const;

    virtual Document* loadDocument(IOAdapter *io, TaskStateInfo &ti, const QVariantMap &fs, DocumentLoadMode mode = DocumentLoadMode_Whole);
    virtual void storeDocument(Document *d, TaskStateInfo &ts, IOAdapter *io);

private:
    QList<GObject*> loadObjects(IOAdapter *io, TaskStateInfo &ti);
    void storeObjects(QList<GObject*> objects, IOAdapter *io, TaskStateInfo &ti);

private:
    QString formatName;
};

} // namespace U2

#endif    // #ifndef _U2_NEXUS_FORMAT_H_
