#ifndef _U2_RPS_BLAST_SUPPORT_TASK_H_
#define _U2_RPS_BLAST_SUPPORT_TASK_H_

#include "BlastPlusSupportCommonTask.h"

#include <U2Algorithm/CDSearchTaskFactory.h>


namespace U2 {

class RPSBlastSupportTask : public BlastPlusSupportCommonTask {
    Q_OBJECT
public:
    RPSBlastSupportTask(const BlastTaskSettings& settings) : BlastPlusSupportCommonTask(settings) {}
    virtual ExternalToolRunTask* createBlastPlusTask();
};

class LocalCDSearch : public CDSearchResultListener {
public:
    LocalCDSearch(const CDSearchSettings& settings);
    virtual Task* getTask() const { return task; }
    virtual QList<SharedAnnotationData> getCDSResults() const;
private:
    RPSBlastSupportTask* task;
};

class CDSearchLocalTaskFactory : public CDSearchFactory {
public:
    virtual CDSearchResultListener* createCDSearch(const CDSearchSettings& settings) const { return new LocalCDSearch(settings); }
};

} //namespace



#endif
