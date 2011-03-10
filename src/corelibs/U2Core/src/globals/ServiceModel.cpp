#include "PluginModel.h"
#include "ServiceModel.h"

#include <algorithm>

namespace U2 {


Service::Service(ServiceType t, const QString& _name, const QString& _desc, const QList<ServiceType>& _parentServices, ServiceFlags f)
: type(t), name(_name), description(_desc), parentServices(_parentServices), state(ServiceState_Disabled_New), flags(f)
{
}

void ServiceRegistry::_setServiceState(Service* s, ServiceState state)  
{
    assert(s->state!=state);

    ServiceState oldState = s->state;
    bool enabledBefore = s->isEnabled();
    s->state = state;
    bool enabledAfter = s->isEnabled();
    s->serviceStateChangedCallback(oldState, enabledBefore!=enabledAfter);
    emit si_serviceStateChanged(s, oldState);
}

}//namespace

