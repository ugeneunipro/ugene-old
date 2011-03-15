/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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
