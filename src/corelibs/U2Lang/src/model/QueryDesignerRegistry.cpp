#include "QueryDesignerRegistry.h"


namespace U2 {

void QDActorPrototypeRegistry::registerProto(QDActorPrototype* _factory) {
    IdRegistry<QDActorPrototype>::registerEntry(_factory);
    emit si_registryModified();
}

QDActorPrototype* QDActorPrototypeRegistry::unregisterProto(const QString& id) {
    QDActorPrototype* removed = IdRegistry<QDActorPrototype>::unregisterEntry(id);
    if(removed) {
        emit si_registryModified();
    }
    return removed;
}

}//namespace
