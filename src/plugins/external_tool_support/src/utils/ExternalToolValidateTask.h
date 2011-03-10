#ifndef _U2_EXTERNAL_TOOL_VALIDATE_TASK_H
#define _U2_EXTERNAL_TOOL_VALIDATE_TASK_H

#include <U2Core/Task.h>
#include <QtCore/QProcess>

namespace U2 {

class ExternalToolLogParser;
class ExternalToolValidateTask: public Task {
    Q_OBJECT
public:
    ExternalToolValidateTask(const QString& toolName);
    ExternalToolValidateTask(const QString& toolName, const QString& path);
    ~ExternalToolValidateTask();

    void prepare();
    void run();
    Task::ReportResult report();

    void cancelProcess();
    bool isValidTool()  { return isValid; }
    QString getToolName()  { return toolName; }
    QString getToolPath()  { return program; }
    QString getToolVersion()  { return version; }
public slots:
    void sl_onReadyToReadLog();
    void sl_onReadyToReadErrLog();
private:
    void checkVersion(const QString& partOfLog, bool isOut);

    QStringList arguments;
    QString     program;
    QProcess*   externalToolProcess;
    char*       logData;
    QString     toolName;
    QString     expectedMessage;
    QRegExp     checkVersionRegExp;
    QString     version;
    bool        isValid;
    QString     lastErrLine;
    QString     lastOutLine;
};

}//namespace

#endif // _U2_EXTERNAL_TOOL_VALIDATE_TASK_H
