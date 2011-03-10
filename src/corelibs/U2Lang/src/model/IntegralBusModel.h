#ifndef _U2_WORKFLOW_BUS_MODEL_H_
#define _U2_WORKFLOW_BUS_MODEL_H_

#include <U2Lang/ActorModel.h>

namespace U2 {
namespace Workflow {

/**
 * To support passing data that is not produced by linked actor-neighbour
 * but produced by actor connected in transitive closure of schema graph
 * information of such data is saved in busmap - StrStr map
 * saved as attribute with BUS_MAP_ATTR_ID id
 * 
 * in such cases port need to know who will produce needed data
 * thats why we need new Port
 * 
 */
class U2LANG_EXPORT IntegralBusPort : public Port {
    Q_OBJECT
public:
    // attribute for busmap
    static const QString BUS_MAP_ATTR_ID;
    static const uint BLIND_INPUT = 1 << 16;
    
public:
    IntegralBusPort(const PortDescriptor& d, Actor* p);
    
    virtual DataTypePtr getType() const;
    DataTypePtr getOwnType() const {return type;}
    
    // slot is list pairs (actorId:attrId);(actorId:attrId);(actorId:attrId)...
    Actor* getProducer(const QString& slot);
    QList<Actor*> getProducers(const QString& slot);
    Actor* getLinkedActorById(ActorId id) const;
    
    // find matching data and assign it
    void setupBusMap();
    
    virtual bool validate(QStringList&) const;
    // reimplemented from Configuration
    virtual void remap(const QMap<ActorId, ActorId>&);
    
    // used when loading schema
    void setBusMapValue(const QString & slotId, const QString & value);
    
protected:
    virtual DataTypePtr getBusType() const;
    //bool getNearestData(const Descriptor & key, QStringList candidates);
    //
    mutable bool recursing;
    
}; // IntegralBusPort

/**
 * as if ActorPrototype creates ports for actor
 * we need special actor prototype for IntegralBusPort
 */
class U2LANG_EXPORT IntegralBusActorPrototype : public ActorPrototype {
public:
    IntegralBusActorPrototype(const Descriptor& desc, 
        const QList<PortDescriptor*>& ports = QList<PortDescriptor*>(), 
        const QList<Attribute*>& attrs = QList<Attribute*>()) : ActorPrototype(desc, ports, attrs) {}
    
    virtual Port* createPort(const PortDescriptor& d, Actor* p) {return new IntegralBusPort(d, p);}
    
}; // IntegralBusActorPrototype

/**
 * validates if binding in busmap to some actor's data is correct
 */
class U2LANG_EXPORT ScreenedSlotValidator : public ConfigurationValidator {
public:
    ScreenedSlotValidator(const QString& slot): screenedSlots(slot) {}
    static bool validate(const QStringList& screenedSlots, const IntegralBusPort*, QStringList& output);
    virtual bool validate(const Configuration*, QStringList& output) const;
        
protected:
    QStringList screenedSlots;
    
}; // ScreenedSlotValidator

/**
 * When attribute can be set by user or can be chosen from integral bus
 * this validator checks that only one must be chosen
 */
class U2LANG_EXPORT ScreenedParamValidator : public ConfigurationValidator {
public:
    ScreenedParamValidator(const QString& id, const QString& port, const QString& slot);
    virtual ~ScreenedParamValidator() {}
    
    virtual bool validate(const Configuration*, QStringList& output) const;
    QString validate(const Configuration * cfg) const;
    
    QString getId() const {return id;}
    QString getPort() const {return port;}
    QString getSlot() const {return slot;}
    
protected:
    QString id;
    QString port;
    QString slot;
    
}; // ScreenedParamValidator

}//namespace Workflow
}//namespace U2

typedef QMap<QString, QString> QStrStrMap;
Q_DECLARE_METATYPE(QStrStrMap)

#endif
