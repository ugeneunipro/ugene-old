/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "WorkflowEnvImpl.h"

#include "U2Lang/ActorPrototypeRegistry.h"
#include <U2Lang/Datatype.h>
#include <U2Lang/WorkflowManager.h>
#include <U2Lang/Schema.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowMonitor.h>
#include <U2Lang/WorkflowTasksRegistry.h>

#include "model/actor/ActorValidator.h"

namespace U2 {
namespace Workflow {

DataTypeRegistry * WorkflowEnvImpl::initDataRegistry() {
    qRegisterMetaTypeStreamOperators<QStrStrMap>("QStrStrMap");
    qRegisterMetaTypeStreamOperators<CfgMap>("CfgMap");
    qRegisterMetaTypeStreamOperators<IterationCfg>("IterationCfg");
    qRegisterMetaType<U2::Workflow::Monitor::FileInfo>( "U2::Workflow::Monitor::FileInfo" );
    qRegisterMetaType<Problem>( "Problem" );
    qRegisterMetaType<U2::Workflow::Monitor::WorkerInfo>( "U2::Workflow::Monitor::WorkerInfo" );
    qRegisterMetaType<U2::Workflow::Monitor::LogEntry>( "U2::Workflow::Monitor::LogEntry" );
    qRegisterMetaType<U2::ActorId>( "U2::ActorId" );

    DataTypeRegistry *r = new DataTypeRegistry();

    return r;
}

DomainFactoryRegistry * WorkflowEnvImpl::initDomainRegistry() {
    DomainFactoryRegistry *r = new DomainFactoryRegistry();
    return r;
}

ActorPrototypeRegistry * WorkflowEnvImpl::initProtoRegistry() {
    ActorPrototypeRegistry *r = new ActorPrototypeRegistry();
    return r;
}

DataTypeValueFactoryRegistry * WorkflowEnvImpl::initDataTypeValueFactoryRegistry() {
    DataTypeValueFactoryRegistry *ret = new DataTypeValueFactoryRegistry();
    ret->registerEntry( new StringTypeValueFactory() );
    ret->registerEntry( new BoolTypeValueFactory() );
    ret->registerEntry( new NumTypeValueFactory() );
    ret->registerEntry( new UrlTypeValueFactory() );
    return ret;
}

WorkflowEnvImpl::~WorkflowEnvImpl()
{
    delete domain;
    delete proto;
    delete data;
    delete dvfReg;
    delete actorValidatorRegistry;
}

ExternalToolCfgRegistry * WorkflowEnvImpl::initExternalToolCfgRegistry() {
    ExternalToolCfgRegistry *ecfgReg = new ExternalToolCfgRegistry();
    return ecfgReg;
}

SchemaActorsRegistry * WorkflowEnvImpl::initSchemaActorsRegistry() {
    SchemaActorsRegistry *actorsRegistry = new SchemaActorsRegistry();
    return actorsRegistry;
}

WorkflowTasksRegistry * WorkflowEnvImpl::initWorkflowTasksRegistry() {
    WorkflowTasksRegistry *workflowTasksRegistry = new WorkflowTasksRegistry();
    return workflowTasksRegistry;
}

ActorValidatorRegistry * WorkflowEnvImpl::initActorValidatorRegistry() {
    ActorValidatorRegistry *actorValidatorRegistry = new ActorValidatorRegistry();
    return actorValidatorRegistry;
}

}//namespace Workflow
}//namespace U2
