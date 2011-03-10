
#ifndef _U2_MSF_FORMAT_H_
#define _U2_MSF_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class IOAdapter;

class U2FORMATS_EXPORT MSFFormat : public DocumentFormat {
    Q_OBJECT
public:

    MSFFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const { return BaseDocumentFormats::MSF; }

    virtual const QString& getFormatName() const { return formatName; }

    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual void storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io );

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;
    
private:
    void save(IOAdapter* io, Document* doc, TaskStateInfo& ti);
    void load(IOAdapter* io, QList<GObject*>& objects, TaskStateInfo& ti);

    QString formatName;
    
    static int  getCheckSum(const QByteArray& seq);

    static const int CHECK_SUM_MOD;
    static const QByteArray MSF_FIELD;
    static const QByteArray CHECK_FIELD;
    static const QByteArray LEN_FIELD;
    static const QByteArray NAME_FIELD;
    static const QByteArray TYPE_FIELD;
    static const QByteArray WEIGHT_FIELD;
    static const QByteArray TYPE_VALUE_PROTEIN;
    static const QByteArray TYPE_VALUE_NUCLEIC;
    static const double WEIGHT_VALUE;
    static const QByteArray END_OF_HEADER_LINE;
    static const QByteArray SECTION_SEPARATOR;
    static const int CHARS_IN_ROW;
    static const int CHARS_IN_WORD;
};

}//namespace

#endif
