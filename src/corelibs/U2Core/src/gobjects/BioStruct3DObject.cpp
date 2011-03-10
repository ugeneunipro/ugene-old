#include "BioStruct3DObject.h"
#include "U2Core/DNASequence.h"
#include "GObjectTypes.h"

namespace U2 { 

BioStruct3DObject::BioStruct3DObject(const BioStruct3D& struct3D, const QString& objectName, const QVariantMap& hintsMap)
: GObject(GObjectTypes::BIOSTRUCTURE_3D, objectName, hintsMap), bioStruct3D(struct3D)
{
}

GObject* BioStruct3DObject::clone() const {
    BioStruct3DObject* cln = new BioStruct3DObject(bioStruct3D,getGObjectName(), getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    return cln;
}


} //namespace

