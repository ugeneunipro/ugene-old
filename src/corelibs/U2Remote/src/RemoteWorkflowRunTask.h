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

#ifndef _U2_REMOTE_WORKFLOW_RUN_TASK_H_
#define _U2_REMOTE_WORKFLOW_RUN_TASK_H_

#include <U2Core/VirtualFileSystem.h>
#include <U2Remote/RemoteMachine.h>

#include <U2Lang/Schema.h>

#include <QtCore/QEventLoop>


namespace U2 {

using namespace Workflow;

class U2REMOTE_EXPORT RemoteWorkflowRunTask : public Task {
    Q_OBJECT
public:
    RemoteWorkflowRunTask( const RemoteMachineSettingsPtr& m, const Schema & sc, const QList<Iteration> & its );
    RemoteWorkflowRunTask( const RemoteMachineSettingsPtr& m, qint64 remoteTaskId);
    ~RemoteWorkflowRunTask();

    static const int TIMER_UPDATE_TIME = 2000; /* 2 seconds */
    static const int REMOTE_TASK_TIMEOUT = 30000;

    virtual void prepare();
    virtual void run();
    QString generateReport() const;
    virtual ReportResult report();
    qint64 getRemoteTaskId() { return taskId; }

private:
    void preprocessSchema();
    static void dumpSchema(const QString& fileName, const QByteArray& schema);

private slots:
    void sl_remoteTaskTimerUpdate();

private:
    RemoteMachineSettingsPtr  machineSettings;
    RemoteMachine *         machine;
    Schema                  schema;
    QList<Iteration>        iterations;
    qint64                  taskId;
    QEventLoop*             eventLoop;
    QStringList             outputUrls;
    QVariantMap             taskSettings;
    // Task is running or enqueued on remote machine already
    bool                    taskIsActive;

}; // RemoteWorkflowRunTask

} // U2

#endif // _U2_REMOTE_WORKFLOW_RUN_TASK_H_
