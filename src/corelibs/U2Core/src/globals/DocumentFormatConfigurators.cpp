#include "DocumentFormatConfigurators.h"

namespace U2 {

DocumentFormatConfigurators::~DocumentFormatConfigurators() {
    foreach(DocumentFormatConfigurator* c, configs.values()) {
        delete c;
    }
    configs.clear();
}


void DocumentFormatConfigurators::registerConfigurator(DocumentFormatConfigurator* c) {
    assert(!configs.contains(c->getFormatId()));
    configs[c->getFormatId()] = c;
}

void DocumentFormatConfigurators::unregisterConfigurator(const DocumentFormatId& id) {
    DocumentFormatConfigurator* c = configs.value(id);
    assert(c!=NULL);
    configs.remove(id);
    delete c;
}

DocumentFormatConfigurator* DocumentFormatConfigurators::findConfigurator(const DocumentFormatId& id) const {
    DocumentFormatConfigurator* c = configs.value(id);
    return c;
}


} //namespace
