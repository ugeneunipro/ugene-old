#include "Port.h"

namespace U2 {

namespace Workflow {

/**************************
* PortDescriptor
**************************/
PortDescriptor::PortDescriptor(const Descriptor& desc, DataTypePtr t, bool input, bool multi, uint f )
    : Descriptor(desc), type(t), input(input), multi(multi), flags(f) {
    assert(type->isMap());
    if(input) {
        assert(!multi && "currently wd model cannot handle multi input ports");
    }
}

bool PortDescriptor::isInput() const {
    return input;
}

bool PortDescriptor::isOutput() const {
    return !input;
}

bool PortDescriptor::isMulti() const {
    return multi;
}

uint PortDescriptor::getFlags() const {
    return flags;
}

DataTypePtr PortDescriptor::getType() const {
    return type;
}

DataTypePtr PortDescriptor::getOutputType() const {
    return type;
}

/**************************
* Port
**************************/
Port::Port(const PortDescriptor& d, Actor* p) : PortDescriptor(d), proc(p) {
}

Actor * Port::owner() const {
    return proc;
}

QMap<Port*, Link*> Port::getLinks() const {
    return bindings;
}

int Port::getWidth() const {
    return bindings.size();
}

void Port::setParameter(const QString& name, const QVariant& val) {
    Configuration::setParameter(name, val);
    emit bindingChanged();
}

void Port::remap(const QMap<ActorId, ActorId>&) {
}

bool Port::canBind(const Port* other) const {
    if (this == other || proc == other->proc || isInput() == other->isInput()) {
        return false;
    }
    if ((!isMulti() && getWidth() != 0) || (!other->isMulti() && other->getWidth() != 0)) {
        return false;
    }
    return !bindings.contains(const_cast<Port*>(other));
}

void Port::addLink(Link* b) {
    Port* peer = isInput()? b->source() : b->destination();
    assert(this == (isInput()? b->destination() : b->source()));
    //assert(canBind(peer));
    assert(!bindings.contains(peer));
    bindings[peer] = b;
    emit bindingChanged();
}

void Port::removeLink(Link* b) {
    Port* peer = isInput()? b->source() : b->destination();
    assert(this == (isInput()? b->destination() : b->source()));
    assert(bindings.contains(peer));
    bindings.remove(peer);
    emit bindingChanged();
}

/**************************
* Link
**************************/
Link::Link()  : src(NULL), dest(NULL) {
}

Link::Link(Port* p1, Port* p2) {
    connect(p1, p2);
}

Port * Link::source() const {
    return src;
}

Port * Link::destination() const {
    return dest;
}

void Link::connect(Port* p1, Port* p2) {
    assert(p1->canBind(p2));
    if (p1->isInput()) {
        dest = p1;
        src = p2;
    } else {
        dest = p2;
        src = p1;
    }
    p1->addLink(this);
    p2->addLink(this);
}

}

} // U2
