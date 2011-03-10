#include "Counter.h"

namespace U2 {

QList<GCounter*>& GCounter::getCounters() {
    static QList<GCounter*> counters;
    return counters;
}

GCounter::GCounter(const QString& _name, const QString& s, double scale) : name(_name), suffix(s), totalCount(0), counterScale(scale) {
    assert(counterScale > 0);
    getCounters().append(this);
    dynamicCounter = false;
}

GCounter::~GCounter() {
    if (dynamicCounter) {
        getCounters().removeOne(this);
    }
}

GReportableCounter::GReportableCounter(const QString& name, const QString& suffix, double scale /* = 1 */) :
GCounter(name, suffix, scale) {
}

} //namespace

