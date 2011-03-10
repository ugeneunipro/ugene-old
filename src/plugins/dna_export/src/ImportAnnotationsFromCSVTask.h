#ifndef _U2_IMPORT_ANNOTATIONS_FROM_CSV_TASK_H_
#define _U2_IMPORT_ANNOTATIONS_FROM_CSV_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/AnnotationData.h>
#include "CSVColumnConfiguration.h"

#include <QtCore/QPointer>

namespace U2 {

class DocumentFormat;

// FIXME: implement splitToken as a default value for parsing script (i.e line.split(<separator>))
class CSVParsingConfig {
public:
    CSVParsingConfig() : defaultAnnotationName("misc_feature"), linesToSkip(0), keepEmptyParts(true), removeQuotes(true){}
    QString             defaultAnnotationName;
    QString             splitToken;
    int                 linesToSkip;
    QString             prefixToSkip;
    bool                keepEmptyParts;
    QList<ColumnConfig> columns;
    QString             parsingScript;
    bool                removeQuotes;
    
    static QBitArray    QUOTES;
};

class ImportAnnotationsFromCSVTaskConfig {
public:
    ImportAnnotationsFromCSVTaskConfig() : addToProject(true), df(NULL) {}

    QString             csvFile;
    QString             dstFile;
    bool                addToProject;
    DocumentFormat*     df;

    CSVParsingConfig parsingOptions;
};

class ReadCSVAsAnnotationsTask;
class SaveDocumentTask;
class AddDocumentTask;
class Annotation;
class Document;

class ImportAnnotationsFromCSVTask: public Task {
    Q_OBJECT
public:
    ImportAnnotationsFromCSVTask(ImportAnnotationsFromCSVTaskConfig& config);

    QList<Task*> onSubTaskFinished(Task* subTask);

private:
    QList<Annotation*> prepareAnnotations() const;

    Document* prepareNewDocument(const QList<Annotation*>& annotations) const;

    ImportAnnotationsFromCSVTaskConfig  config;
    ReadCSVAsAnnotationsTask*           readTask;
    SaveDocumentTask*                   writeTask;
    AddDocumentTask*                    addTask;
    QPointer<Document>                  doc;
};

class ReadCSVAsAnnotationsTask : public Task {
Q_OBJECT
public:
    ReadCSVAsAnnotationsTask(const QString& file, const CSVParsingConfig& config);

    void run();

    QList<SharedAnnotationData> getResult() const {return result;}

    static QList<QStringList> parseLinesIntoTokens(const QString& text, const CSVParsingConfig& config, int& maxColumns, TaskStateInfo& ti);
    
    static QStringList parseLineIntoTokens(const QString& line, const CSVParsingConfig& config, TaskStateInfo& ti, int lineNum = 1);

    static QString guessSeparatorString(const QString& text, const CSVParsingConfig& config);

    // script variable that holds line value
    static QString LINE_VAR;
    // script variable that holds parsed line numbers. Lines that skipped to not increment this value
    static QString LINE_NUM_VAR;

private:
    QString                         file;
    CSVParsingConfig                config;
    QList<SharedAnnotationData>     result;
};

} // namespace U2

#endif 
