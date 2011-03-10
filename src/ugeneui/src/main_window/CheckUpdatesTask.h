#ifndef _U2_CHECK_UPDATES_TASKS_H_
#define _U2_CHECK_UPDATES_TASKS_H_

#include <U2Core/Task.h>

namespace U2 {

class CheckUpdatesTask : public Task {
    Q_OBJECT

public:
    CheckUpdatesTask();
    void run();
    ReportResult report();

    QString siteVersion;
};


}//namespace

#endif
