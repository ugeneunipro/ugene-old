#ifndef _U2_HELP_DUMPER_
#define _U2_HELP_DUMPER_

#include <U2Core/Task.h>

namespace U2 {

    class DumpHelpTask : public Task {
        Q_OBJECT
    public:
        static void dumpHelp();
        // for debug uses only
        static void dumpParameters();

    public:
        DumpHelpTask();
        virtual void prepare();

    protected:
        /// Called by scheduler when subtask is finished.
        virtual QList<Task*> onSubTaskFinished(Task* subTask);
    };

} //namespace U2

#endif
