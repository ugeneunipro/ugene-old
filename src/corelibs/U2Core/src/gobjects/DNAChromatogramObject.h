#ifndef _U2_DNA_CHROMATOGRAM_OBJECT_H_
#define _U2_DNA_CHROMATOGRAM_OBJECT_H_

#include <U2Core/GObject.h>
#include <U2Core/DNAChromatogram.h>

namespace U2 {

class  U2CORE_EXPORT DNAChromatogramObject: public GObject {
    Q_OBJECT
public:
    DNAChromatogramObject(const DNAChromatogram& chrom, const QString& objectName, const QVariantMap& hints = QVariantMap());

    const DNAChromatogram& getChromatogram() const {return chrom;}

    virtual GObject* clone() const;

protected:
    DNAChromatogram chrom;
};

}//namespace


#endif
