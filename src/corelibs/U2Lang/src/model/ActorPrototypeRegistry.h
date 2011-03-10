#ifndef _U2_WORKFLOW_REG_H_
#define _U2_WORKFLOW_REG_H_

#include <U2Lang/ActorModel.h>

namespace U2 {
namespace Workflow {

/**
 * registry of actor prototypes
 * register proto to one of groups - computational categories
 * 
 * Base categories are listed in BioActorLibrary
 */
class U2LANG_EXPORT ActorPrototypeRegistry: public QObject {
    Q_OBJECT
public:
    virtual ~ActorPrototypeRegistry();
    
    // standard registry
    void registerProto(const Descriptor& group, ActorPrototype* proto);
    ActorPrototype* unregisterProto(const QString& id);
    const QMap<Descriptor, QList<ActorPrototype*> >& getProtos() const {return groups;}
    ActorPrototype* getProto(const QString& id) const;
    
signals:
    void si_registryModified();
    
private:
     QMap<Descriptor, QList<ActorPrototype*> > groups;
    
}; // ActorPrototypeRegistry

}//namespace Workflow

}//namespace U2

#endif // _U2_WORKFLOW_REG_H_
