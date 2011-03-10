#ifndef _U2_DNASEQUENCE_SELECTION_H_
#define _U2_DNASEQUENCE_SELECTION_H_

#include "LRegionsSelection.h"
#include "SelectionTypes.h"

namespace U2 {

class DNASequenceObject;

class U2CORE_EXPORT DNASequenceSelection : public LRegionsSelection {
    
    Q_OBJECT

public:
    DNASequenceSelection(DNASequenceObject* _obj, QObject* p = NULL) : LRegionsSelection(GSelectionTypes::DNASEQUENCE, p), obj(_obj) {}
    const DNASequenceObject* getSequenceObject() const { return obj; }

private:
    DNASequenceObject* obj;
};

}//namespace

#endif
