#include "SelectionModel.h"


namespace U2 {

void MultiGSelection::addSelection(const GSelection* s) {
    assert(!selections.contains(s));
    selections.push_back(s);
}

void MultiGSelection::removeSelection(const GSelection* s) {
    assert(selections.contains(s));
    selections.removeAll(s);
}


const GSelection* MultiGSelection::findSelectionByType(GSelectionType t) const {
    foreach(const GSelection* s, selections) {
        if (s->getSelectionType() == t) {
            return s;
        }
    }
    return NULL;
}



}//namespace
