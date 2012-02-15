/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_DISTRIBUTED_COMPUTING_UTIL_H
#define _U2_DISTRIBUTED_COMPUTING_UTIL_H

#include <QtCore/QObject>

#include <U2Remote/ProtocolInfo.h>
#include <U2Remote/RemoteMachineMonitor.h>

namespace U2 {

class PingTask;

class U2REMOTE_EXPORT DistributedComputingUtil : public QObject {
    Q_OBJECT
public:
    DistributedComputingUtil();
    ~DistributedComputingUtil();
        
    /* remote machines has some services that are only for internal usage. this function filters them */
    static QStringList filterRemoteMachineServices( const QStringList & services );
    
private slots:
    void sl_showRemoteMachinesMonitor();
    
private:
    /* pointers here to manage object creation order */
    ProtocolInfoRegistry *        pir;
    RemoteMachineMonitor *        rmm;
    
}; // DistributedComputingUtil


/* Checks for active tasks and adds them to TaskScheduler 
 */
class UpdateActiveTasks : public Task {
    Q_OBJECT

public:
    UpdateActiveTasks( const RemoteMachineSettingsPtr& s);
    ~UpdateActiveTasks();
    virtual void run();
    const RemoteMachineSettingsPtr& getSettings() { return settings; }
   
private:
    RemoteMachineSettingsPtr settings;
    RemoteMachine* machine;
    void addTaskToScheduler(qint64 taskid);

}; // UpdateActiveTasks

} // U2

#endif // _U2_DISTRIBUTED_COMPUTING_UTIL_H
