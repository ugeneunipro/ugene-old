#ifndef _U2_STOCKHOLM_FORMAT_H_
#define _U2_STOCKHOLM_FORMAT_H_

#include <QtCore/QByteArray>

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class IOAdapter;

class U2FORMATS_EXPORT StockholmFormat : public DocumentFormat {
    Q_OBJECT
public:
    static const QByteArray FILE_ANNOTATION_ID;
    static const QByteArray FILE_ANNOTATION_AC;
    static const QByteArray FILE_ANNOTATION_DE;
    static const QByteArray FILE_ANNOTATION_GA;
    static const QByteArray FILE_ANNOTATION_NC;
    static const QByteArray FILE_ANNOTATION_TC;
    static const QByteArray COLUMN_ANNOTATION_SS_CONS;
    static const QByteArray COLUMN_ANNOTATION_RF;
    static const QByteArray UNI_ANNOTATION_MARK;
    
private:
    QString format_name;

public:
    StockholmFormat( QObject* obj );

    virtual DocumentFormatId getFormatId() const {
        return BaseDocumentFormats::STOCKHOLM;
    }

    virtual const QString& getFormatName() const {
        return format_name;
    }

    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual void storeDocument( Document* doc, TaskStateInfo& ti, IOAdapter* io );

    virtual FormatDetectionResult checkRawData(const QByteArray& data, const GUrl& = GUrl()) const;

    virtual bool isObjectOpSupported( const Document* d , DocumentFormat::DocObjectOp op, GObjectType t ) const;

    //exceptions
    struct StockholmBaseException {
        QString msg;
        StockholmBaseException( const QString& str ): msg( str ){}
    };
    struct ReadError : public StockholmBaseException {
        ReadError(const GUrl& url);
    };
    struct WriteError: public StockholmBaseException {
        WriteError(const GUrl& url);
    };
    struct BadFileData : public StockholmBaseException {
        BadFileData( const QString& msg ): StockholmBaseException( msg ){}
    };

}; // StockholmFormat

} // namespace


#endif //_U2_STOCKHOLM_FORMAT_H_
