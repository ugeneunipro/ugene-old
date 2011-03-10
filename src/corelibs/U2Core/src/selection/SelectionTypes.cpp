#include "SelectionTypes.h"

// These are included to export symbols
#include "TextSelection.h"
#include "DNASequenceSelection.h"

namespace U2 {

const GSelectionType GSelectionTypes::DOCUMENTS("selection_documents");

const GSelectionType GSelectionTypes::GOBJECTS("selection_gobjects");

const GSelectionType GSelectionTypes::GOBJECT_VIEWS("selection_gobject_views");

const GSelectionType GSelectionTypes::TEXT("selection_text");

const GSelectionType GSelectionTypes::DNASEQUENCE("selection_dnasequence");

const GSelectionType GSelectionTypes::ANNOTATIONS("selection_annotations");

const GSelectionType GSelectionTypes::ANNOTATION_GROUPS("selection_annotation_groups");

}//namespace
