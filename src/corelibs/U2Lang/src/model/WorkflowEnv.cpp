#include <U2Lang/SchemaSerializer.h>
#include "WorkflowEnv.h"

namespace U2 {
namespace Workflow {

WorkflowEnv* WorkflowEnv::instance;

bool WorkflowEnv::init(WorkflowEnv* env) {
    if (instance) {
        assert(0);// Duplicate initialization;
        return false;
    }
    
    env->data = env->initDataRegistry();
    env->proto = env->initProtoRegistry();
    env->domain = env->initDomainRegistry();
    instance = env;
    env->dvfReg = env->initDataTypeValueFactoryRegistry();
    return true;
}

}// namespace Workflow
}//namespace U2
