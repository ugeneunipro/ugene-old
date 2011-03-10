#include "SWResultFilterRegistry.h"

#include <QtCore/QMutexLocker>


namespace U2 {

SWResultFilterRegistry::SWResultFilterRegistry(QObject* pOwn): 
    QObject(pOwn)
{
    registerFilter(new SWRF_EmptyFilter());
    
    SmithWatermanResultFilter* f = new SWRF_WithoutIntersect();
    registerFilter(f);
    defaultFilterId = f->getId();
}

SWResultFilterRegistry::~SWResultFilterRegistry() {
    QList<SmithWatermanResultFilter*> swFilters = filters.values();
    foreach (SmithWatermanResultFilter* swFilter, swFilters) {
        delete swFilter;
    }
}

QStringList SWResultFilterRegistry::getFiltersIds() const {
    return filters.keys();
}

SmithWatermanResultFilter* SWResultFilterRegistry::getFilter(const QString& id) {
    return filters.value(id, 0);
}

bool SWResultFilterRegistry::isRegistered(const QString& id) const {
    return filters.contains(id);
}

bool SWResultFilterRegistry::registerFilter( SmithWatermanResultFilter* filter ) {
    QMutexLocker locker(&mutex);
    QString id = filter->getId();
    if (filters.contains(id)) {
        return false;
    }
    filters[id] = filter;
    return true;
}

} // namespace
