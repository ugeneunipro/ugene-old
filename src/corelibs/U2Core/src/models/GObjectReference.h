#ifndef _U2_GOBJECT_REFERENCE_H_
#define _U2_GOBJECT_REFERENCE_H_

#include <U2Core/global.h>

#include <QtCore/QDataStream>

namespace U2 {

class GObject;

class U2CORE_EXPORT GObjectReference {
public:
    GObjectReference(){}

    GObjectReference(const QString& _docUrl, const QString& _objName, const GObjectType& _type) 
        : docUrl(_docUrl), objName(_objName), objType(_type){}

    GObjectReference(const GObject* obj, bool deriveLoadedType = true);

    bool isValid() const {return !docUrl.isEmpty() && !objName.isEmpty() && !objType.isEmpty();}

    bool operator ==(const GObjectReference& o) const;

    /** GObject reference keeps only string path of the document url.
        This must be enough to find document in the project
        while allows not to keep a complete url data here (for example username/password, etc...)
    */
    QString        docUrl;

    /** The name of the object */
    QString     objName;

    /** The type of the object */
    GObjectType objType;

private:
    static bool registerMeta;
};

//TODO: add constraints on relation roles

class U2CORE_EXPORT GObjectRelation {
public:
    GObjectRelation(){}

    GObjectRelation(const GObjectReference& _ref, const QString& _role, const QString& _data = QString()) 
        : ref(_ref), role(_role), data(_data){}

    bool isValid() const {return ref.isValid() && !role.isEmpty();}

    bool operator ==(const GObjectRelation& o) const;

    const QString& getDocURL() const {return ref.docUrl;}

    GObjectReference ref;
    QString         role;
    QString         data;
private:
    static bool registerMeta;
};



QDataStream &operator<<(QDataStream &out, const GObjectReference &myObj);
QDataStream &operator>>(QDataStream &in, GObjectReference &myObj);
QDataStream &operator<<(QDataStream &out, const GObjectRelation &myObj);
QDataStream &operator>>(QDataStream &in, GObjectRelation &myObj);
    

}//namespace

Q_DECLARE_METATYPE( U2::GObjectReference )
Q_DECLARE_METATYPE( U2::GObjectRelation )
Q_DECLARE_METATYPE( QList< U2::GObjectReference > )
Q_DECLARE_METATYPE( QList< U2::GObjectRelation > )

#endif
