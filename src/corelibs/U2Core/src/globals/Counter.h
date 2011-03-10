#ifndef _U2_COUNTER_H_
#define _U2_COUNTER_H_

#include "global.h"

#include <QtCore/QList>

namespace U2 {

class U2CORE_EXPORT GCounter : public QObject {
    Q_OBJECT
public:
    GCounter(const QString& name, const QString& suffix, double scale = 1);
    virtual ~GCounter();

    static const QList<GCounter*>& allCounters() {return getCounters();}

    QString name;
    QString suffix;
    qint64  totalCount;
    double  counterScale;
    bool    dynamicCounter; //true if created and deleted dynamically at application runtime (not static init time)

    double scaledTotal() const {return totalCount / counterScale;}
    
protected:
    
    static QList<GCounter*>& getCounters();
};

//Marks that counter will be reported by Shtirlitz
//TODO: implement GPerformanceCounter for plugin_perf_monitor?
class U2CORE_EXPORT GReportableCounter : public GCounter {
    Q_OBJECT
public:
    GReportableCounter(const QString& name, const QString& suffix, double scale = 1);
};


class U2CORE_EXPORT SimpleEventCounter {
public:
    SimpleEventCounter(GCounter* tc) : totalCounter(tc), eventCount(1){ assert(tc!=NULL);}
    virtual ~SimpleEventCounter() {totalCounter->totalCount+=eventCount;}
private:
    GCounter*   totalCounter;
    qint64      eventCount;
};

#define GCOUNTER(cvar, tvar, name) \
    static GReportableCounter cvar(name, "", 1); \
    SimpleEventCounter tvar(&cvar)


} //namespace

#endif
