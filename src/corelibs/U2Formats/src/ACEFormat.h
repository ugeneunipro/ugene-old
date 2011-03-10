#ifndef _U2_ACE_FORMAT_H_
#define _U2_ACE_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>

namespace U2 {
class IOAdapter;
class U2FORMATS_EXPORT ACEFormat : public DocumentFormat {
Q_OBJECT
public:
    ACEFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const { return BaseDocumentFormats::ACE; }
    virtual const QString& getFormatName() const { return formatName; }
    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);
    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;
    
private:
    QString formatName;
    void load(IOAdapter* io, QList<GObject*>& objects, TaskStateInfo& ti);

    static const QString CO;
    static const QString RD;
    static const QString QA;
    static const QString AS;
    static const QString AF;
    static const QString BQ;
};

} //namespace

#endif
