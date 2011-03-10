#ifndef _U2_EXTERNAL_TOOL_RUN_TASK_H
#define _U2_EXTERNAL_TOOL_RUN_TASK_H

#include <U2Core/Task.h>
#include <QtCore/QProcess>

namespace U2 {

class ExternalToolLogParser;
class ExternalToolRunTask: public Task {
    Q_OBJECT
public:
    ExternalToolRunTask(const QString& toolName, const QStringList& arguments, ExternalToolLogParser*  logParser);
    ~ExternalToolRunTask();

    void prepare();
    void run();
    Task::ReportResult report();

    void cancelProcess();
public slots:
    void sl_onReadyToReadLog();
    void sl_onReadyToReadErrLog();
private:
    QStringList             arguments;
    QString                 program;
    ExternalToolLogParser*  logParser;
    QString                 toolName;
    char*                   logData;
    QProcess*               externalToolProcess;
};

class ExternalToolLogParser {
public:
    ExternalToolLogParser();
    virtual int getProgress(){ return progress; }
    virtual void parseOutput(const QString& partOfLog);
    virtual void parseErrOutput(const QString& partOfLog);
    QString getLastError() const {return lastErrLine;}
    
private:
    /* Percent values in range 0..100, negative if unknown. */
    QString     lastErrLine;
    int         progress;
    QString     lastLine;
    
protected:
    QStringList lastPartOfLog;
};

}//namespace

#endif // _U2_EXTERNAL_TOOL_RUN_TASK_H
