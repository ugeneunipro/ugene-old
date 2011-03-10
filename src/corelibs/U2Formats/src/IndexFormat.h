#ifndef _U2_INDEX_FORMAT_H_
#define _U2_INDEX_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class U2FORMATS_EXPORT IndexFormat : public DocumentFormat {
    Q_OBJECT
public:
    static const int OBJECTS_IN_DOC = 1;
    
    static const QByteArray FILE_HEADER;
    static const QString WRITE_LOCK_REASON;

private:
    QString format_name;
    
public:
    IndexFormat( QObject* obj );
    
    virtual DocumentFormatId getFormatId() const {
        return BaseDocumentFormats::INDEX;
    }

    virtual const QString& getFormatName() const {
        return format_name;
    }

    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual void storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io);

    virtual FormatDetectionResult checkRawData(const QByteArray& data, const GUrl& = GUrl()) const;

    virtual bool isObjectOpSupported( const Document* d , DocumentFormat::DocObjectOp op, GObjectType t ) const;

    struct IndexFormatException {
    QString msg;
    IndexFormatException( const QString& what ) : msg( what ){}
    };
    
    struct ReadError : public IndexFormatException {
        ReadError() : IndexFormatException( tr( "Read error occurred" ) ){}
    };
    
    struct BadDataError : public IndexFormatException {
        BadDataError( const QString& what ) : IndexFormatException( what ){}
    };
    
    struct WriteError : public IndexFormatException {
        WriteError() : IndexFormatException( tr( "Write error occurred" ) ){}
    };
    
}; // IndexFormat

} // U2

#endif // _U2_INDEX_FORMAT_H_
