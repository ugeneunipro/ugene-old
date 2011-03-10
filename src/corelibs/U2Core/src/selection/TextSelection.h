#ifndef _U2_TEXT_SELECTION_H_
#define _U2_TEXT_SELECTION_H_

#include "LRegionsSelection.h"
#include "SelectionTypes.h"

namespace U2 {

class TextObject;

class  U2CORE_EXPORT TextSelection : public LRegionsSelection {
public:
    TextSelection(TextObject* _obj, QObject* p = NULL) : LRegionsSelection(GSelectionTypes::TEXT, p), obj(_obj) {}

private:
    TextObject* obj;
};

}//namespace

#endif
