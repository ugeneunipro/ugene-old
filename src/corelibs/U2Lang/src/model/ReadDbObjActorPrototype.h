/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_READ_DB_OBJ_ACTOR_PROTOTYPE_H_
#define _U2_READ_DB_OBJ_ACTOR_PROTOTYPE_H_

#include <U2Lang/IntegralBusModel.h>

namespace U2 {

namespace Workflow {

class U2LANG_EXPORT ReadDbObjActorPrototype : public IntegralBusActorPrototype {
public:
    ReadDbObjActorPrototype(const Descriptor &desc, const QList<PortDescriptor *> &ports = QList<PortDescriptor *>(),
        const QList<Attribute *> &attrs = QList<Attribute *>());

    // set @types as a compatible types for a URL attribute
    virtual void setCompatibleDbObjectTypes(const QSet<GObjectType> &types);
};

} // Workflow

} // U2

#endif // _U2_READ_DB_OBJ_ACTOR_PROTOTYPE_H_
