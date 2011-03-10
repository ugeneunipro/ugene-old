#ifndef _U2_TEXT_OBJECT_H_
#define _U2_TEXT_OBJECT_H_

#include <U2Core/GObject.h>
#include "GObjectTypes.h"

namespace U2 {

class U2CORE_EXPORT TextObject: public GObject {
    Q_OBJECT
public:
    TextObject(const QString& _text, const QString& objectName, const QVariantMap& hintsMap = QVariantMap()) 
        : GObject(GObjectTypes::TEXT, objectName, hintsMap), text(_text){};

    virtual const QString& getText() const {return text;}

    virtual void setText(const QString& newText);

    virtual GObject* clone() const;

protected:
    QString text;
};

}//namespace


#endif
