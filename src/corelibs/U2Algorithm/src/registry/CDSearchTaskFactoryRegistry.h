#ifndef _U2_CDS_FACTORY_REGISTRY_H_
#define _U2_CDS_FACTORY_REGISTRY_H_

#include <U2Algorithm/CDSearchTaskFactory.h>


namespace U2 {

class U2ALGORITHM_EXPORT CDSearchFactoryRegistry : public QObject {
    Q_OBJECT
public:
    CDSearchFactoryRegistry() : localSearchFactory(NULL), remoteSearchFactory(NULL) {}
    ~CDSearchFactoryRegistry() {
        delete localSearchFactory;
        delete remoteSearchFactory;
    }

    enum SearchType { LocalSearch, RemoteSearch };

    void registerFactory(CDSearchFactory* factory, SearchType type) {
        if (type == LocalSearch) {
            assert(localSearchFactory == NULL);
            localSearchFactory = factory;
        } else if (type == RemoteSearch) {
            assert(remoteSearchFactory == NULL);
            remoteSearchFactory = factory;
        } else {
            assert(0);
        }
    }

    CDSearchFactory* getFactory(SearchType type) const {
        if (type == LocalSearch) {
            return localSearchFactory;
        } else if (type == RemoteSearch) {
            return remoteSearchFactory;
        } else {
            assert(0);
            return NULL;
        }
    }

private:
    CDSearchFactory* localSearchFactory;
    CDSearchFactory* remoteSearchFactory;
};

} //namespace

#endif
