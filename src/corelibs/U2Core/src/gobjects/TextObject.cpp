#include "TextObject.h"

namespace U2 {

void TextObject::setText(const QString& newText) {
    text = newText;
    setModified(true);
}

GObject* TextObject::clone() const {
    TextObject* cln = new TextObject(text, getGObjectName(), getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    return cln;
}


}//namespace


