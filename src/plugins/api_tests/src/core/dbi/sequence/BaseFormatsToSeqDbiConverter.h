#ifndef _U2_BASE_FORMATS_TO_SEQDBI_CONVERTER_H_
#define _U2_BASE_FORMATS_TO_SEQDBI_CONVERTER_H_

#include <U2Core/Task.h>
#include <U2Core/DNASequence.h>
#include <U2Core/U2Dbi.h>
#include <memory>


namespace U2 {

class FillDbTask : public Task {
    Q_OBJECT
public:
    FillDbTask(const QList<DNASequence>& sequences, const QString& dbUrl);
    virtual void run();
private:
    std::auto_ptr<U2Dbi> dbi;
    QList<DNASequence> sequences;
};

class LoadDocumentTask;

class ConvertBaseFormatToDbTask : public Task {
    Q_OBJECT
public:
    ConvertBaseFormatToDbTask(const QString& srcDoc, const QString& dbUrl);
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
private:
    LoadDocumentTask* loadTask;
    QString dbUrl;
};

} //namespace

#endif
