#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <U2Core/Log.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/AppContext.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNASequenceObject.h>

#include "ImportQualityScoresTask.h"
#include <time.h>
#include <memory>

#define ULOG_IMPORT_QUALITY_SCORES "Dna_export: import quality scores"

namespace U2 {

static Logger log(ULOG_IMPORT_QUALITY_SCORES);

ReadQualityScoresTask::ReadQualityScoresTask( const QString& file, DNAQualityType t)
: Task("ReadPhredQuality", TaskFlag_None), fileName(file), type(t)
{

}

#define READ_BUF_SIZE 4096

void ReadQualityScoresTask::run() {
    
    IOAdapterFactory* f = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    std::auto_ptr<IOAdapter> io( f->createIOAdapter() );
    
    if (!io->open(fileName, IOAdapterMode_Read) ) {
        stateInfo.setError("Can not open quality file");
        return;
    }
    
    int headerCounter = -1;
    QByteArray readBuf(READ_BUF_SIZE+1, 0);
    char* buf = readBuf.data();


    while (!stateInfo.cancelFlag) {
        
        int len = io->readUntil(buf, READ_BUF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include); 
 
        stateInfo.progress = io->getProgress();

        if (len == 0) {
            recordQuality(headerCounter);
            break;
        }

        if (buf[0] == '>') {
            recordQuality(headerCounter);
            QByteArray header = readBuf.mid(1, len - 1).trimmed();
            headers.append(header);
            values.clear();
            ++headerCounter;
            continue;
        } 

        QByteArray valsBuf = readBuf.mid(0, len).trimmed();
        QList<QByteArray> valList = valsBuf.split(' ');
        foreach(const QByteArray& valStr, valList) {
            bool ok = false;
            values.append( valStr.toInt(&ok) );
            if (!ok) {
                setError(QString("Failed parse quality value: file %1, seq name %2").arg(fileName).arg(headers[headerCounter]));
            }
        }
    }

    io->close();

}


void ReadQualityScoresTask::recordQuality( int headerCounter )
{
    if (headerCounter > -1) {
        QByteArray qualCodes;
        foreach (int v, values) {
            char code = DNAQuality::encode(v, type);
            qualCodes.append(code);
        }
        result.insert(headers[headerCounter], DNAQuality(qualCodes,type));
        log.trace( QString("Phred quality parsed: %1 %2").arg(headers[headerCounter]).arg(qualCodes.constData()) );
    }
}


//////////////////////////////////////////////////////////////////////////


ImportPhredQualityScoresTask::ImportPhredQualityScoresTask(const QList<DNASequenceObject*>& sequences, ImportQualityScoresConfig& cfg )
: Task("ImportPhredQualityScores", TaskFlags_NR_FOSCOE), readQualitiesTask(NULL), config(cfg), seqList(sequences)
{
    
}

void ImportPhredQualityScoresTask::prepare()
{
    readQualitiesTask = new ReadQualityScoresTask(config.fileName, config.type);
    addSubTask(readQualitiesTask);
}

QList<Task*> ImportPhredQualityScoresTask::onSubTaskFinished( Task* subTask )
{
    QList<Task*> subTasks;
    if ( subTask->hasErrors() || subTask->isCanceled() ) {
        return subTasks;
    }

    if (subTask == readQualitiesTask) {
        QMap<QString,DNAQuality> qualities = readQualitiesTask->getResult();
        if (config.createNewDocument) {
            assert(0);
            //TODO: consider creating this option
        } else {
            foreach (DNASequenceObject* obj, seqList) {
                if (obj->isStateLocked()) {
                    setError(QString("Unable to modify sequence %1: object is locked.").arg(obj->getGObjectName()));
                    continue;
                }
                QString dnaName = obj->getDNASequence().getName().split(' ').first();
                if (qualities.contains(dnaName)) {
                    obj->setQuality(qualities.value(dnaName));
                } else {
                    setError(QString("Quality scores for %1 are not found.").arg(dnaName));
                    break;
                }
            }
        }
    }

    return subTasks;
}




} // namespace U2
