#ifndef _REMOTE_SERVICE_UTIL_TASKS_H_
#define _REMOTE_SERVICE_UTIL_TASKS_H_

#include <U2Core/Task.h>
#include <memory>

namespace U2 {

class RemoteServiceMachine;

struct RemoteTaskInfo {
    QString taskId;
    QString date;
    QString taskState;
    QString result;
};

 
class GetUserTasksInfoTask: public Task {
public:
    GetUserTasksInfoTask(RemoteServiceMachine* machine);
    virtual void run();
    QList<RemoteTaskInfo>& getUserTasksInfo() { return infoList; }
private:
    RemoteServiceMachine* machine;
    QMap<QString,QString> properties;
    QList<RemoteTaskInfo> infoList;
};

class FetchRemoteTaskResultTask : public Task {
public:
    FetchRemoteTaskResultTask(RemoteServiceMachine* machine, const QStringList& resultUrls, qint64 taskId);
    virtual void run();
private:
    RemoteServiceMachine* machine;
    QStringList resultUrls;
    qint64 taskId;
};
  

class DeleteRemoteDataTask : public Task {
public:
    DeleteRemoteDataTask(RemoteServiceMachine* machine, qint64 taskId);
    virtual void run();
private:
    RemoteServiceMachine* machine;
    qint64 taskId;
};


} // namespace U2

#endif // _REMOTE_SERVICE_UTIL_TASKS_H_
