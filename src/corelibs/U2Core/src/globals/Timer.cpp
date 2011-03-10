#include "Timer.h"

namespace U2 {


QDateTime GTimer::createDateTime(qint64 micros, Qt::TimeSpec spec) {
    QDateTime res(QDate(1970, 1, 1), QTime(0, 0), Qt::UTC);
    res = res.addMSecs(micros/1000);
    assert(res.isValid());
    return res.toTimeSpec(spec);
}

static double getFrequency() {
    double frequency = 0;
#if defined(Q_OS_WIN) && defined(WIN_USE_PERF_COUNTER)
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    frequency = (double)freq.QuadPart;
#else 
    frequency = 1000*1000; //microseconds
#endif    
    return frequency;

}

static qint64 getCorrection() {
    GCounter totalCounter("timer correction", "ticks", 1);
    totalCounter.dynamicCounter = true;
    
    TimeCounter tc(&totalCounter, false);
    tc.start(); tc.stop();
    tc.start(); tc.stop();
    tc.start(); tc.stop();
    tc.start(); tc.stop();
    
    qint64 correction = totalCounter.totalCount / 4;
    return correction;
}

qint64 TimeCounter::correction = getCorrection();
double TimeCounter::frequency = getFrequency();

bool TimeCounter::enabled = true;
QString  TimeCounter::timeSuffix("seconds");
} //namespace
