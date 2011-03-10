#include "BaseSlots.h"

static const QString SEQ_SLOT_ID("sequence");
static const QString MA_SLOT_ID("msa");
static const QString FEATURE_TABLE_SLOT_ID("annotations");
static const QString TEXT_SLOT_ID("text");
static const QString URL_SLOT_ID("url");
static const QString FASTA_HEADER_SLOT_ID("fasta-header");

namespace U2 {
namespace Workflow {

const Descriptor BaseSlots::DNA_SEQUENCE_SLOT() {
    return Descriptor(SEQ_SLOT_ID, tr("Sequence"), tr("A biological sequence"));
}

const Descriptor BaseSlots::MULTIPLE_ALIGNMENT_SLOT() {
    return Descriptor(MA_SLOT_ID, tr("MSA"), tr("A multiple sequence alignment"));
}

const Descriptor BaseSlots::ANNOTATION_TABLE_SLOT() {
    return Descriptor(FEATURE_TABLE_SLOT_ID, tr("Set of annotations"), tr("A set of annotated regions"));
}

const Descriptor BaseSlots::TEXT_SLOT() {
    return Descriptor(TEXT_SLOT_ID, tr("Plain text"), tr("Plain text reading or splitting to strings."));
}

const Descriptor BaseSlots::URL_SLOT() {
    return Descriptor(URL_SLOT_ID, tr("Source URL"), tr("Location of a corresponding input file."));
}

const Descriptor BaseSlots::FASTA_HEADER_SLOT() {
    return Descriptor(FASTA_HEADER_SLOT_ID, tr("FASTA header"), tr("A header line for the FASTA record."));
}

} // Workflow
} // U2
