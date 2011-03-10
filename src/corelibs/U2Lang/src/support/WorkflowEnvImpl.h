#ifndef _U2_WORKFLOW_ENVIMPL_H_
#define _U2_WORKFLOW_ENVIMPL_H_

#include <U2Lang/WorkflowEnv.h>

namespace U2 {

namespace Workflow {

/**
 * Workflow environment container implementation
 */
class U2LANG_EXPORT WorkflowEnvImpl : public WorkflowEnv {
protected:
    virtual ~WorkflowEnvImpl();
    
    virtual DataTypeRegistry* initDataRegistry();
    virtual ActorPrototypeRegistry* initProtoRegistry();
    virtual DomainFactoryRegistry* initDomainRegistry();
    virtual DataTypeValueFactoryRegistry* initDataTypeValueFactoryRegistry();
    
}; // WorkflowEnvImpl

} //namespace Workflow

} //namespace U2

#endif
