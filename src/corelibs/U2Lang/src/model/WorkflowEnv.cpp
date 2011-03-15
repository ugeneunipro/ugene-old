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
