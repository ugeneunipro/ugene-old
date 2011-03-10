#ifndef _U2_QUERY_DESIGNER_REGISTRY_H_
#define _U2_QUERY_DESIGNER_REGISTRY_H_

#include "QDScheme.h"

#include <U2Core/IdRegistry.h>


namespace U2 {
    
class U2LANG_EXPORT QDActorPrototypeRegistry : public QObject, public IdRegistry<QDActorPrototype> {
    Q_OBJECT
public:
    void registerProto(QDActorPrototype* _factory);
    QDActorPrototype* unregisterProto(const QString& id);
    QList<QDActorPrototype*> getProtos() { return IdRegistry<QDActorPrototype>::getAllEntries(); }
    QDActorPrototype* getProto(const QString& id) { return IdRegistry<QDActorPrototype>::getById(id); }
signals:
    void si_registryModified();
};

}//namespace

#endif
