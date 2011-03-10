#ifndef _U2_DNA_QUALITY_H_
#define _U2_DNA_QUALITY_H_

#include <U2Core/global.h>

#include <QtCore/QByteArray>
#include <QtCore/QStringList>

namespace U2 {

enum DNAQualityType {
    DNAQualityType_Sanger,
    DNAQualityType_Illumina,
    DnaQualityType_Solexa
};

class U2CORE_EXPORT DNAQuality {
public:
    DNAQuality() : type (DNAQualityType_Sanger) {}
    DNAQuality(const QByteArray& qualScore, DNAQualityType type = DNAQualityType_Sanger);
    QByteArray qualCodes;
    DNAQualityType type;
    bool isEmpty() const { return qualCodes.isEmpty(); }
    int getValue(int pos) const;
    static char encode(int val, DNAQualityType type);
    
    static QString getDNAQualityNameByType(DNAQualityType t);
    static DNAQualityType getDNAQualityTypeByName(const QString& name);
    static QStringList getDNAQualityTypeNames();

};

}//namespace


#endif
