#include "DataBaseRegistry.h"

namespace U2 {

DataBaseRegistry::DataBaseRegistry(QObject *o):QObject(o) {
}

DataBaseRegistry::~DataBaseRegistry() {
    foreach(const DataBaseFactory *dbf, factories) {
        delete dbf;
    }
}

bool DataBaseRegistry::registerDataBase(DataBaseFactory *f, const QString &id) {
    if(!isRegistered(id)) {
        factories[id] = f;
        return true;
    }
    else {
        return false;
    }
}

bool DataBaseRegistry::isRegistered(const QString& id) {
    if(factories.contains(id)) {
        return true;
    }
    else {
        return false;
    }
}

DataBaseFactory *DataBaseRegistry::getFactoryById(const QString &id) {
    if(isRegistered(id)) {
        return factories[id];
    }
    else {
        return NULL;
    }
}

}