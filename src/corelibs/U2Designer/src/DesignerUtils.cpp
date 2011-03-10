#include "DesignerUtils.h"

#include <U2Lang/WorkflowUtils.h>
#include <U2Misc/DialogUtils.h>

//TODO FIX translator
namespace U2 {

QString DesignerUtils::getSchemaFileFilter(bool any, bool addOldExt) {
    QStringList exts(WorkflowUtils::WD_FILE_EXTENSIONS);
    if(addOldExt) {
        exts << WorkflowUtils::WD_XML_FORMAT_EXTENSION;
    }
    return DialogUtils::prepareFileFilter(WorkflowUtils::tr("UGENE workflow schemes"), exts, any);
}

} //ns