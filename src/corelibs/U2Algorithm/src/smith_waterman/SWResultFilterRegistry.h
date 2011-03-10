#ifndef _U2_SW_RESULT_FILTER_REGISTRY_H_
#define _U2_SW_RESULT_FILTER_REGISTRY_H_

#include <QtCore/QStringList>
#include <QtCore/QMutex>

#include <U2Algorithm/SmithWatermanResultFilters.h>


namespace U2 {

class U2ALGORITHM_EXPORT SWResultFilterRegistry: public QObject {
    Q_OBJECT
public:
    SWResultFilterRegistry(QObject* pOwn = 0);
    ~SWResultFilterRegistry();
    
    QStringList getFiltersIds() const;
    
    SmithWatermanResultFilter* getFilter(const QString& id);
    
    bool isRegistered(const QString& id) const;
    
    bool registerFilter( SmithWatermanResultFilter* filter );

    const QString& getDefaultFilterId() {return defaultFilterId;}

private:
    QMutex mutex;
    QHash<QString, SmithWatermanResultFilter*> filters;   
    QString defaultFilterId;
};

} // namespace

#endif
