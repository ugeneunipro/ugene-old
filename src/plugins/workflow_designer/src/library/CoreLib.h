#ifndef _U2_WORKFLOW_CORE_LIB_LIB_H_
#define _U2_WORKFLOW_CORE_LIB_LIB_H_

#include <U2Lang/Datatype.h>

namespace U2 {

namespace Workflow {

class CoreLib : public QObject {
    Q_OBJECT
public:
    static void init();
    static void initUsersWorkers();
};

} // Workflow namespace
} // U2 namespace

#endif
