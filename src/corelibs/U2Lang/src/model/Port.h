#ifndef _U2_WORKFLOW_PORT_H_
#define _U2_WORKFLOW_PORT_H_

#include <U2Lang/Peer.h>
#include <U2Lang/Datatype.h>
#include <U2Lang/Configuration.h>

namespace U2 {

namespace Workflow {

class Actor;
class Link;

/**
* identity data of a port
*/
class U2LANG_EXPORT PortDescriptor : public Descriptor {
public:
    PortDescriptor(const Descriptor& desc, DataTypePtr type, bool input, bool multi = false, uint f = 0);
    virtual ~PortDescriptor() {}

    bool isInput() const;
    bool isOutput() const;
    bool isMulti() const;
    uint getFlags() const;

    virtual DataTypePtr getType() const;
    DataTypePtr getOutputType() const;

protected:
    // type of data that this port contains
    DataTypePtr type;
    // port can be input (receives data) or output (sends data)
    bool input;
    // port can get data from many sources
    bool multi;
    // for user purposes
    // see usage in implementations e.g. IntegralBusModel.cpp
    uint flags;

}; // PortDescriptor

/**
* Port is a place where communication channels meet actors
* actually, Port itself does not need to be a configuration
* nowadays, BusPort uses 1 attribute BUS_MAP to store StrStr map of values that come to port
* see BusPort for details
* Configuration is chosen for extensibility
*
* Peer needs in realizations to store CommunicationChannel that goes throw a port. see BaseWorker::BaseWorker
*/
class U2LANG_EXPORT Port : public QObject, public PortDescriptor, public Configuration, public Peer {
    Q_OBJECT
public:
    Port(const PortDescriptor& d, Actor* p);
    virtual ~Port() {}

    Actor* owner() const;

    QMap<Port*,Link*> getLinks() const;
    virtual void addLink(Link*);
    virtual void removeLink(Link*);

    // how many links goes throw this port
    int getWidth() const;

    // reimplemented Configuration::setParameter
    virtual void setParameter(const QString& name, const QVariant& val);

    // simple check if one port can be binded to other
    // port types doesn't check here
    virtual bool canBind(const Port* other) const;

    // reimplemented Configuration::remap
    // empty implementation
    virtual void remap(const QMap<ActorId, ActorId>&);

signals:
    // emitted when link is added or removed from bindings
    void bindingChanged();

protected:
    // owner of this port
    Actor* proc;
    // links with other ports
    QMap<Port*,Link*> bindings;

}; // Port

/**
* represents connection between 2 ports
* as if ports are binded to actors, link represents connection between 2 actors
*
* Peer needs to store CommunicationChannel. See LocalDomainFactory::createConnection as example
*/
class U2LANG_EXPORT Link : public Peer {
public:
    Link();
    Link(Port* p1, Port* p2);
    virtual ~Link() {}

    // adds this link to p1 and p2 bindings
    void connect(Port* p1, Port* p2);

    Port* source() const;
    Port* destination() const;

private:
    // output port of some actor
    Port *src;
    // input port of some actor
    Port *dest;

}; // Link

}

} // U2

#endif // _U2_WORKFLOW_PORT_H_
