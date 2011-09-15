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

#ifndef _REMOTE_SERVICE_PING_TASK_H_
#define _REMOTE_SERVICE_PING_TASK_H_

#include <U2Core/Task.h>
#include <memory>

namespace U2 {

 class RemoteServiceMachine;
 class RemoteServiceMachineFactory;

class RemoteServicePingTask : public Task {
	Q_OBJECT
public:
    RemoteServicePingTask(const QString& url);
    virtual void prepare();
    virtual void run();
private:
    QString machinePath;
    std::auto_ptr<RemoteServiceMachine> machine;
    std::auto_ptr<RemoteServiceMachineFactory> machineFactory;
};
  

} // namespace U2

#endif // _REMOTE_SERVICE_PING_TASK_H_
