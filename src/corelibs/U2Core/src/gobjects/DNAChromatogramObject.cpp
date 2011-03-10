#include "DNAChromatogramObject.h"

#include "GObjectTypes.h"

namespace U2 {

DNAChromatogramObject::DNAChromatogramObject(const DNAChromatogram& c, const QString& objectName,const QVariantMap& hints) 
: GObject(GObjectTypes::CHROMATOGRAM, objectName, hints), chrom(c)
{

}

GObject* DNAChromatogramObject::clone() const {
    DNAChromatogramObject* cln = new DNAChromatogramObject(chrom, getGObjectName(), getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    return cln;
}

}//namespace


