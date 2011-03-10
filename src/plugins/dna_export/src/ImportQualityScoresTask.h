#ifndef _U2_IMPORT_QUALITY_SCORES_TASK_H_
#define _U2_IMPORT_QUALITY_SOCRES_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/DNAQuality.h>

#include <QtCore/QPointer>

namespace U2 {

class DNASequenceObject;

class ImportQualityScoresConfig {
public:
    ImportQualityScoresConfig() : createNewDocument(false) {}
    QString             fileName;
    DNAQualityType      type; 
    bool                createNewDocument;
    QString             dstFileName;
};

 
class ReadQualityScoresTask : public Task {
    Q_OBJECT
public:
    ReadQualityScoresTask(const QString& fileName, DNAQualityType t);

    void run();
    
    QMap<QString,DNAQuality> getResult() const { return result; }
    
private:
    void recordQuality( int headerCounter );
    QString                     fileName;
    DNAQualityType              type;
    QStringList                 headers;
    QList<int>                  values;
    QMap<QString, DNAQuality>   result;
};


class ImportPhredQualityScoresTask : public Task {
    Q_OBJECT
public:
    ImportPhredQualityScoresTask(const QList<DNASequenceObject*>& sequences, ImportQualityScoresConfig& config);

    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);

private:
    ReadQualityScoresTask* readQualitiesTask;
    ImportQualityScoresConfig config;
    QList<DNASequenceObject*> seqList;
};


} // namespace U2

#endif 
