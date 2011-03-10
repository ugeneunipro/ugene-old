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
