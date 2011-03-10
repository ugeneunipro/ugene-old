#ifndef _U2_COREAPI_ID_REGISTRY_H_
#define _U2_COREAPI_ID_REGISTRY_H_

namespace U2 {

/*************************************
 * template class for default registry
 *************************************/
template <class T> class IdRegistry {
public:
    virtual T* getById(const QString& id) {return registry.value(id);}
    virtual bool registerEntry(T* t) {
        if (registry.contains(t->getId())) {
            return false;
        } else {
            registry.insert(t->getId(), t);
            return true;
        } 
    }
    virtual T* unregisterEntry(const QString& id) {return registry.take(id);}
    virtual ~IdRegistry() { qDeleteAll(registry.values());}

    virtual QList<T*> getAllEntries() const {return registry.values();}
    virtual QList<QString> getAllIds() const {return registry.uniqueKeys();}

protected:
    QMap<QString, T*> registry;
    
}; // IdRegistry

} // U2

#endif // _U2_COREAPI_ID_REGISTRY_H_
