#ifndef _U2_BLASTPPLUS_SUPPORT_TASK_H
#define _U2_BLASTPPLUS_SUPPORT_TASK_H

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DocumentModel.h>

#include <U2Core/AnnotationData.h>


#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include "utils/ExportTasks.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>

#include "ExternalToolRunTask.h"
#include "BlastPlusSupportCommonTask.h"

#include <QtXml/QDomNode>
#include <QtXml/QDomDocument>

namespace U2 {

class BlastPPlusSupportTask : public BlastPlusSupportCommonTask {
    Q_OBJECT
public:
    BlastPPlusSupportTask(const BlastTaskSettings& settings):BlastPlusSupportCommonTask(settings){}
    virtual ExternalToolRunTask* createBlastPlusTask();
};

}//namespace
#endif // _U2_BLASTPPLUS_SUPPORT_TASK_H
