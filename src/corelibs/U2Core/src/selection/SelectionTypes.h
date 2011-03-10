#ifndef _U2_SELECTION_TYPES_H_
#define _U2_SELECTION_TYPES_H_

#include <U2Core/SelectionModel.h>

namespace U2 {

class U2CORE_EXPORT GSelectionTypes {
public:
    static const GSelectionType DOCUMENTS;
    static const GSelectionType GOBJECTS;
    static const GSelectionType GOBJECT_VIEWS;
    static const GSelectionType TEXT;
    static const GSelectionType DNASEQUENCE;
    static const GSelectionType ANNOTATIONS;
    static const GSelectionType ANNOTATION_GROUPS;
};

}//namespace

#endif

