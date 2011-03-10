#ifndef _U2_WORKFLOW_CORE_LIB_CONSTANTS_H_
#define _U2_WORKFLOW_CORE_LIB_CONSTANTS_H_

#include <QtCore/QString>
#include <U2Core/global.h>

namespace U2 {

namespace Workflow {

class U2LANG_EXPORT CoreLibConstants {
public:
    static const QString WRITE_FASTA_PROTO_ID;
    static const QString READ_TEXT_PROTO_ID;
    static const QString WRITE_TEXT_PROTO_ID;
    static const QString WRITE_GENBANK_PROTO_ID;
    static const QString WRITE_FASTQ_PROTO_ID;
    static const QString GENERIC_READ_MA_PROTO_ID;
    static const QString GENERIC_READ_SEQ_PROTO_ID;
    static const QString WRITE_CLUSTAL_PROTO_ID;
    static const QString WRITE_STOCKHOLM_PROTO_ID;
    static const QString WRITE_MSA_PROTO_ID;
    static const QString WRITE_SEQ_PROTO_ID;
    
    static const QString TEXT_TYPESET_ID;
    
    static const QString WORKFLOW_ON_CLOUD_TASK_ID;
    static const QString WORKFLOW_SCHEMA_ATTR;
    static const QString DATA_IN_ATTR;
    static const QString DATA_OUT_ATTR;
    
}; // CoreLibConstants

} // Workflow

} // U2

#endif // _U2_WORKFLOW_CORE_LIB_CONSTANTS_H_
