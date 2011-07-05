#ifndef _U2_CONVERT_TO_DB_TASK_H_
#define _U2_CONVERT_TO_DB_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/DNASequence.h>
#include <U2Core/U2Dbi.h>
#include <memory>


namespace U2 {

class LoadDocumentTask;

class ConvertToSQLiteTask : public Task {
    Q_OBJECT
public:
    ConvertToSQLiteTask(const QString& file, const QString& dbUrl);
    ~ConvertToSQLiteTask();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
private:
    LoadDocumentTask* loadTask;
    std::auto_ptr<U2Dbi> dbi;
};

class GObject;

class FillDbTask : public Task {
    Q_OBJECT
public:
    FillDbTask(U2Dbi* dbi, const QList<GObject*>& objects);
    virtual void run();
private:
    void addSequence(U2Sequence& seq, const QByteArray& data);
private:
    U2Dbi* dbi;
    QList<GObject*> objects;
};

} //namespace

#endif
