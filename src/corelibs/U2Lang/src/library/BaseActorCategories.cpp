#include <U2Lang/WorkflowEnv.h>

#include "LocalDomain.h"
#include "BaseActorCategories.h"

namespace U2 {
namespace Workflow {

const Descriptor BaseActorCategories::CATEGORY_DATASRC() {
    return Descriptor("a", tr("Data sources"), "");
}

const Descriptor BaseActorCategories::CATEGORY_DATASINK() {
    return Descriptor("b", tr("Data sinks"), "");
}

const Descriptor BaseActorCategories::CATEGORY_CONVERTERS() {
    return Descriptor("bc", tr("Data converters"), "");
}

const Descriptor BaseActorCategories::CATEGORY_BASIC() { 
    return Descriptor("c", tr("Basic analysis"), "");
}

const Descriptor BaseActorCategories::CATEGORY_ALIGNMENT() { 
    return Descriptor("d", tr("Multiple sequence alignment"), "");
}

const Descriptor BaseActorCategories::CATEGORY_ASSEMBLY() {
    return Descriptor("asm", tr("DNA assembly"), ""); 
}

const Descriptor BaseActorCategories::CATEGORY_TRANSCRIPTION() {
    return Descriptor("tr", tr("Transcription factor"), ""); 
}

const Descriptor BaseActorCategories::CATEGORY_SCRIPT() {
    return Descriptor("sa", tr("Custom elements"), "");
}

}//Workflow namespace
}//GB2 namespace
