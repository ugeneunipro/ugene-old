#ifndef _GB2_HMMER3_HMM_FORMAT_H_
#define _GB2_HMMER3_HMM_FORMAT_H_

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <U2Core/global.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>

namespace U2 {

class UHMMFormat : public DocumentFormat {
    Q_OBJECT
public:
    static const DocumentFormatId   UHHMER_FORMAT_ID;
    static const QString            WRITE_LOCK_REASON;
    static const QString            WRITE_FAILED;
    
public:
    UHMMFormat( QObject* obj );
    
    virtual DocumentFormatId getFormatId() const;
    
    virtual const QString& getFormatName() const;
    
    
    virtual Document* loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& formatSettings, DocumentLoadMode mode = DocumentLoadMode_Whole );
    
    virtual void storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io );
    
    virtual FormatDetectionResult checkRawData( const QByteArray& data, const GUrl& = GUrl()) const;
    
    struct UHMMWriteException {
        QString what;
        UHMMWriteException( const QString& msg ) : what( msg ) {}
    }; // UHMMWriteException
    
private:
    QString formatName;
    
}; // UHMMFormat

} // U2

#endif // _GB2_HMMER3_HMM_FORMAT_H_
