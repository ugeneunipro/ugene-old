#ifndef _U2_BIOSTRUCT3D_OBJECT_H_
#define _U2_BIOSTRUCT3D_OBJECT_H_

#include <U2Core/GObject.h>
#include <U2Core/U2Region.h>
#include <U2Core/BioStruct3D.h>

namespace U2 {

class  U2CORE_EXPORT BioStruct3DObject: public GObject {
    Q_OBJECT
public:
    BioStruct3DObject(const BioStruct3D& bioStruct, const QString& objectName, const QVariantMap& hintsMap = QVariantMap());
    const BioStruct3D& getBioStruct3D() const { return bioStruct3D;}
    virtual GObject* clone() const;

protected:
    BioStruct3D     bioStruct3D;

};

} //namespace


#endif //_U2_BIOSTRUCT3D_OBJECT_H_
