#include "WorkflowTransport.h"

namespace U2 {

namespace Workflow {

Message::Message(DataTypePtr _t, const QVariant& d) : id(nextid()), t(_t), data(d) {
}

int Message::nextid() {
    static QAtomicInt id(0);
    return id.fetchAndAddRelaxed(1); //memory model??
}

int Message::getId() const {
    return id;
}

DataTypePtr Message::getType() const {
    return t;
}

QVariant Message::getData() const {
    return data;
}

} //Workflow namespace

} //GB2 namespace
