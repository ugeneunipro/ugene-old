/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <U2Remote/RemoteMachine.h>
#include <U2Remote/SerializeUtils.h>

#include "PingRemoteServiceTests.h"

namespace U2 {

#define MACHINE_CFG_PATH "cfg-path"

void GTest_PingRemoteServiceTask::init(XMLTestFormat*, const QDomElement& el) {    
    pingTask = NULL;
    machineCfgPath = el.attribute(MACHINE_CFG_PATH);
    if (machineCfgPath.isEmpty()) {
        failMissingValue(MACHINE_CFG_PATH);
        return;
    }
    machineCfgPath.prepend(env->getVar("COMMON_DATA_DIR")+"/");
}

void GTest_PingRemoteServiceTask::prepare()
{
    RemoteMachineSettingsPtr rMachine = SerializeUtils::deserializeRemoteMachineSettingsFromFile(machineCfgPath);
    if(rMachine == NULL) {
        setError(tr("Can not parse remote server settings file %1").arg(machineCfgPath));
        return;
    }
    pingTask = new RetrieveRemoteMachineInfoTask(rMachine);
    addSubTask(pingTask);
}

Task::ReportResult GTest_PingRemoteServiceTask::report() {
    if (hasError() || isCanceled()) {
        return ReportResult_Finished;
    }
    
    if (pingTask->hasError()) {
        setError(pingTask->getError());
    }

    return ReportResult_Finished;
}

void GTest_PingRemoteServiceTask::cleanup()
{
}

///////////////////////////////////////////////////////////////////////////////////////////

QList<XMLTestFactory*> PingRemoteServiceTests::createTestFactories()
{
    QList<XMLTestFactory*> res;
    res.append(GTest_PingRemoteServiceTask::createFactory());
    
    return res;
}

} // U2
