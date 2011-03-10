#ifndef __U2_WORKFLOW_BASE_ATTRIBUTES_H_
#define __U2_WORKFLOW_BASE_ATTRIBUTES_H_

#include <U2Lang/Descriptor.h>

namespace U2 {
namespace Workflow {

class U2LANG_EXPORT BaseAttributes : public QObject {
    Q_OBJECT
public:
    static const Descriptor URL_IN_ATTRIBUTE();
    static const Descriptor URL_OUT_ATTRIBUTE();
    static const Descriptor URL_LOCATION_ATTRIBUTE();
    static const Descriptor DOCUMENT_FORMAT_ATTRIBUTE();
    static const Descriptor ACCUMULATE_OBJS_ATTRIBUTE();
    static const Descriptor READ_BY_LINES_ATTRIBUTE();
    static const Descriptor FILE_MODE_ATTRIBUTE();
    
    static const Descriptor STRAND_ATTRIBUTE();
    static const QString STRAND_BOTH();
    static const QString STRAND_DIRECT();
    static const QString STRAND_COMPLEMENTARY();
    static const QVariantMap STRAND_ATTRIBUTE_VALUES_MAP();
    
}; // BaseAttributes

} // Workflow
} // U2

#endif // __U2_WORKFLOW_BASE_ATTRIBUTES_H_
