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

