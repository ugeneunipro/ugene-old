#ifndef _U2_UGENECL_DUMP_LICENCE_TASK_H_
#define _U2_UGENECL_DUMP_LICENCE_TASK_H_

#include <U2Core/Task.h>

namespace U2 {

class DumpLicenseTask : public Task {
Q_OBJECT
public:
    static const QString LICENSE_CMDLINE_OPTION;
    static const QString LICENCE_FILENAME;
    static void initHelp();

public:
    DumpLicenseTask();
    virtual void run();
    
}; // DumpLicenseTask

}

#endif // _U2_UGENECL_DUMP_LICENCE_TASK_H_
