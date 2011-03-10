#include "WorkflowEnvImpl.h"

#include "U2Lang/ActorPrototypeRegistry.h"
#include <U2Lang/Datatype.h>
#include <U2Lang/WorkflowManager.h>
#include <U2Lang/Schema.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/BaseTypes.h>

namespace U2 {
namespace Workflow {

DataTypeRegistry* WorkflowEnvImpl::initDataRegistry() {
    qRegisterMetaTypeStreamOperators<QStrStrMap>("QStrStrMap");
    qRegisterMetaTypeStreamOperators<CfgMap>("CfgMap");
    qRegisterMetaTypeStreamOperators<IterationCfg>("IterationCfg");

    DataTypeRegistry * r = new DataTypeRegistry();

    return r;
}

DomainFactoryRegistry* WorkflowEnvImpl::initDomainRegistry() {
    DomainFactoryRegistry* r = new DomainFactoryRegistry();
    return r;
}

ActorPrototypeRegistry* WorkflowEnvImpl::initProtoRegistry() {

    ActorPrototypeRegistry* r = new ActorPrototypeRegistry();
    return r;
}

DataTypeValueFactoryRegistry* WorkflowEnvImpl::initDataTypeValueFactoryRegistry() {
    DataTypeValueFactoryRegistry* ret = new DataTypeValueFactoryRegistry();
    ret->registerEntry( new StringTypeValueFactory() );
    ret->registerEntry( new BoolTypeValueFactory() );
    ret->registerEntry( new NumTypeValueFactory() );
    return ret;
}

WorkflowEnvImpl::~WorkflowEnvImpl()
{
    delete domain;
    delete proto;
    delete data;
    delete dvfReg;
}

}//namespace Workflow
}//namespace U2
