#ifndef _U2_WORKFLOW_BASE_LIB_H_
#define _U2_WORKFLOW_BASE_LIB_H_

#include <U2Lang/Datatype.h>

namespace U2 {
namespace Workflow {

class U2LANG_EXPORT BaseActorCategories : public QObject {
    Q_OBJECT
public:
    static const Descriptor CATEGORY_DATASRC();
    static const Descriptor CATEGORY_DATASINK();
    static const Descriptor CATEGORY_CONVERTERS();
    static const Descriptor CATEGORY_BASIC();
    static const Descriptor CATEGORY_ALIGNMENT();
    static const Descriptor CATEGORY_ASSEMBLY();
    static const Descriptor CATEGORY_TRANSCRIPTION();
    static const Descriptor CATEGORY_SCRIPT();
    
}; // BaseActorCategories

}//Workflow namespace
}//GB2 namespace

#endif
