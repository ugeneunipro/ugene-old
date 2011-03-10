#ifndef _U2_BLASTPLUS_SUPPORT_TASK_H
#define _U2_BLASTPLUS_SUPPORT_TASK_H

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DocumentModel.h>

#include <U2Core/AnnotationData.h>

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include "utils/ExportTasks.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>

#include "ExternalToolRunTask.h"
#include "utils/BlastTaskSettings.h"

#include <QtXml/QDomNode>
#include <QtXml/QDomDocument>

#define ULOG_CAT_BLASTPLUS_RUN_TASK "Plugin: External Tool Support: blast+"

namespace U2 {

class BlastPlusSupportCommonTask : public Task {
    Q_OBJECT
public:
    BlastPlusSupportCommonTask(const BlastTaskSettings& settings);
    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);

    Task::ReportResult report();

    QList<SharedAnnotationData> getResultedAnnotations() const;
    BlastTaskSettings           getSettings() const;

    virtual ExternalToolRunTask* createBlastPlusTask() = 0;
protected:
    BlastTaskSettings               settings;
    ExternalToolLogParser*          logParser;
    QString                         url;
private:
    SaveDocumentTask*               saveTemporaryDocumentTask;
    ExternalToolRunTask*            blastPlusTask;
    DNASequenceObject*              sequenceObject;
    Document*                       tmpDoc;
    QList<SharedAnnotationData>     result;

    void parseResult();
    void parseHit(const QDomNode &xml);
    void parseHsp(const QDomNode &xml,const QString &id, const QString &def, const QString &accession);
};

class BlastPlusSupportMultiTask : public Task {
    Q_OBJECT
public:
    BlastPlusSupportMultiTask(QList<BlastTaskSettings>& settingsList, QString& url);
    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);

    Task::ReportResult report();
    QString generateReport() const;
private:
    QList<BlastTaskSettings>    settingsList;
    Document*                   doc;
    QString                     url;
};

}//namespace
#endif // _U2_BLASTPLUS_SUPPORT_TASK_H
