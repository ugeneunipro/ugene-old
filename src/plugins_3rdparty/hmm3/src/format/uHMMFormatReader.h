#ifndef _GB2_HMMER3_HMM_FORMAT_READER_H_
#define _GB2_HMMER3_HMM_FORMAT_READER_H_

#include <QtCore/QObject>
#include <QtCore/QString>

#include <U2Core/IOAdapter.h>
#include <U2Core/Task.h>

#include <hmmer3/hmmer.h>

namespace U2 {

class UHMMFormatReader : public QObject {
    Q_OBJECT
public:
    static const QString HMM_FORMAT_READER_ERROR_PREFIX;
    static const QString READ_FAILED;
    
    enum HMMER_VERSIONS {
        UNKNOWN_VERSION = -1,
        HMMER2_VERSION,
        HMMER3_VERSION
    }; // HMMER_VERSIONS
    static const QString HMMER2_VERSION_HEADER;
    static const QString HMMER3_VERSION_HEADER;
    
public:
    UHMMFormatReader( IOAdapter * i, TaskStateInfo & tsi );
    P7_HMM * getNextHmm();
    
    struct UHMMFormatReaderException {
        QString what;
        UHMMFormatReaderException( const QString& msg ) : what( msg ) {}
    }; // UHMMFormatReaderException
    
private:
    P7_HMM * readHMMER3ASCII();
    P7_HMM * readHMMER2ASCII(); /* for backward compatibility */
    HMMER_VERSIONS getVersion( const QByteArray & header ) const;
    
private:
    IOAdapter *     io; // opened io adapter
    TaskStateInfo & ti;
    
}; // UHMMFormatReader

} // U2

#endif // _GB2_HMMER3_HMM_FORMAT_READER_H_
