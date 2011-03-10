#ifndef _U2_INTEGRAL_BUS_TYPE_H_
#define _U2_INTEGRAL_BUS_TYPE_H_

#include <U2Lang/Datatype.h>
#include <U2Lang/IntegralBusModel.h>

namespace U2 {

namespace Workflow {

/**
 * IntegralPort can receive data from actors that are not connected with him directly
 * but connected in transitive closure of schema graph
 * 
 * for such purposes, we need IntegralBusType
 */
class U2LANG_EXPORT IntegralBusType : public MapDataType {
public:
    IntegralBusType(const Descriptor& d, const QMap<Descriptor, DataTypePtr>& m);
    
    // add port's type to map
    void addInputs(const Port* p);
    void addOutput(DataTypePtr, const Port* producer);
    
public:
    // in busmap (see IntegralBusPort) attributes of other actors saved as 'actorId:attrId'
    // these are utility functions to work with it
    static Descriptor assignSlotDesc(const Descriptor& elementDesc, const Port* producer);
    static ActorId parseSlotDesc(const QString& id);
    static QString parseAttributeIdFromSlotDesc(const QString & str);
    // when schema is deeply copied we need to remap actorIds in busmap
    static void remap(QStrStrMap& busMap, const QMap<ActorId, ActorId>&);
    
}; // IntegralBusType

} //namespace Workflow

} //namespace U2

#endif // _U2_INTEGRAL_BUS_TYPE_H_
