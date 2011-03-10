#ifndef __U2_WORKFLOW_BASE_SLOTS_H_
#define __U2_WORKFLOW_BASE_SLOTS_H_

#include <U2Lang/Descriptor.h>

namespace U2 {
namespace Workflow {

class U2LANG_EXPORT BaseSlots : public QObject {
    Q_OBJECT
public:
    static const Descriptor DNA_SEQUENCE_SLOT();
    static const Descriptor MULTIPLE_ALIGNMENT_SLOT();
    static const Descriptor ANNOTATION_TABLE_SLOT();
    static const Descriptor TEXT_SLOT();
    static const Descriptor URL_SLOT();
    static const Descriptor FASTA_HEADER_SLOT();
    
}; // BaseSlots

} // Workflow
} // U2

#endif // __U2_WORKFLOW_BASE_SLOTS_H_
