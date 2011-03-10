
#ifndef _U2_CREATE_FILE_INDEX_TASK_H_
#define _U2_CREATE_FILE_INDEX_TASK_H_

#include <QList>

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/UIndex.h>

namespace U2 {

class U2CORE_EXPORT CreateFileIndexTask : public Task {
    Q_OBJECT
public:
    static const QString KEY_NAME;
    static const QString KEY_SEQ_LEN;
    static const QString KEY_SEQ_COUNT;
    static const QString KEY_ACCESSION;
    static const QString KEY_DESCRIPTION;
    
public:
    CreateFileIndexTask( const QList< QString >& inputUrls, const QString& outputUrl, 
                         const QList< IOAdapterFactory* >& inputFactories, IOAdapterFactory* outputFactory );
    
    virtual void run();
    
    const QList<QString> & getInputUrls() const;
    const QString & getOutputUrl() const;
    
private:
    void readInputUrls();
    void readOneUrl( const QString& url, IOAdapterFactory* fa, int ind );
    void writeOutputUrl();
    void fillIOSec(UIndex::IOSection& ioSec, const QString& url, IOAdapterFactory* factory, int num );
    
private:
    QList< QString >  inputUrls;
    QString           outputUrl;
    
    QList< IOAdapterFactory* > inputFactories;
    IOAdapterFactory* outputFactory;
    
    UIndex            ind;
    
}; // CreateFileIndexTask

} // U2

#endif // _U2_CREATE_FILE_INDEX_TASK_H_
