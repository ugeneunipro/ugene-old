#ifndef __U2_WORKFLOW_BASE_PORTS_H_
#define __U2_WORKFLOW_BASE_PORTS_H_

#include <QtCore/QString>
#include <U2Core/global.h>

namespace U2 {
namespace Workflow {

class U2LANG_EXPORT BasePorts {
public:
    static const QString OUT_MSA_PORT_ID();
    static const QString IN_MSA_PORT_ID();
    static const QString OUT_SEQ_PORT_ID();
    static const QString IN_SEQ_PORT_ID();
    static const QString OUT_ANNOTATIONS_PORT_ID();
    static const QString IN_ANNOTATIONS_PORT_ID();
    static const QString OUT_TEXT_PORT_ID();
    static const QString IN_TEXT_PORT_ID();
    
}; // BasePorts

} // Workflow
} // U2

#endif // __U2_WORKFLOW_BASE_PORTS_H_
