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

#include <U2Lang/ActorPrototypeRegistry.h>

namespace U2 {
namespace Workflow {
void ActorPrototypeRegistry::registerProto(const Descriptor& group, ActorPrototype* proto) {
    // debug check for proto name
    QString id = proto->getId(); Q_UNUSED(id);
    assert(!id.contains("."));
    
    groups[group].append(proto);
    emit si_registryModified();
}

ActorPrototype* ActorPrototypeRegistry::unregisterProto(const QString& id) {
    QMap<Descriptor, QList<ActorPrototype*> >::iterator it;
    for (it = groups.begin(); it != groups.end(); ++it)
    {
        QList<ActorPrototype*>& l = it.value();
        foreach(ActorPrototype* p, l) {
            if (p->getId() == id) {
                l.removeAll(p);
                if (l.isEmpty()) {
                    groups.remove(it.key());
                }
                emit si_registryModified();
                return p;
            }
        }
    }
    return NULL;
}

ActorPrototype* ActorPrototypeRegistry::getProto(const QString& id) const {
    foreach(QList<ActorPrototype*> l, groups.values()) {
        foreach(ActorPrototype* p, l) {
            if (p->getId() == id) {
                return p;
            }
        }
    }
    return NULL;
}

ActorPrototypeRegistry::~ActorPrototypeRegistry()
{
    foreach(QList<ActorPrototype*> l, groups) {
        qDeleteAll(l);
    }
    groups.clear();
}
}//namespace Workflow
}//namespace U2
