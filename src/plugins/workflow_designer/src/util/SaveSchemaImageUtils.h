#ifndef _U2_WORKFLOW_SAVE_SCHEMA_IMAGE_UTILS_H_
#define _U2_WORKFLOW_SAVE_SCHEMA_IMAGE_UTILS_H_

#include <QtGui/QPixmap>
#include <QtXml/QDomDocument>

#include <U2Core/Task.h>
#include <U2Lang/Schema.h>

namespace U2 {

using namespace Workflow;

class SaveSchemaImageUtils : public QObject {
    Q_OBJECT
public:
    static QPixmap generateSchemaSnapshot(const QString & data);
    static QString saveSchemaImageToFile(const QString & schemaName, const QString & imagePath);
};

class GoogleChartImage {
public:
    static const QString    CHART_TYPE_OPTION;
    static const QString    GRAPH_OPTION;
    static const QString    CHART_SIZE_OPTION;
    static const QString    GRAPH_VIZ_CHART_TYPE;
    static const QSize      CHART_SIZE_DEFAULT;
    static const QString    GOOGLE_CHART_BASE_URL;
    
public:
    GoogleChartImage(Schema * sc, const Metadata& meta);
    QString getImageUrl() const;
    
private:
    QString getUrlArguments() const;
    
private:
    QSize chartSize;
    Schema * schema;
    Metadata meta;
};

/**
 * Produces link to googlechart api of produced image
 */
class ProduceSchemaImageLinkTask : public Task {
    Q_OBJECT
public:
    ProduceSchemaImageLinkTask(const QString & schemaName);
    ~ProduceSchemaImageLinkTask();
    
    virtual void prepare();
    virtual ReportResult report();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    
    QString getImageLink() const;

private:
    ProduceSchemaImageLinkTask();
    ProduceSchemaImageLinkTask(const ProduceSchemaImageLinkTask &);
    ProduceSchemaImageLinkTask & operator=(const ProduceSchemaImageLinkTask &);
    
private:
    QString schemaPath;
    Schema * schema;
    Metadata meta;
    QString imageLink;
    
}; // ProduceSchemaImageLinkTask

} // U2

#endif // _U2_WORKFLOW_SAVE_SCHEMA_IMAGE_UTILS_H_
