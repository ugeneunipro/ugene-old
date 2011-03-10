#include "DNAQuality.h"
#include "DNAChromatogram.h"

namespace U2 {

static const QString SANGER("Sanger");
static const QString ILLUMINA("Illumina 1.3+");
static const QString SOLEXA("Solexa/Illumina 1.0");

DNAQuality::DNAQuality( const QByteArray& qualScore, DNAQualityType t /*= DNAQualityType_Sanger*/ )
: qualCodes(qualScore), type(t)
{

}

int DNAQuality::getValue( int pos ) const
{
    assert(pos >=0 && pos < qualCodes.count());
    return  ( (int)qualCodes.at(pos) - 33 );
}

char DNAQuality::encode( int val, DNAQualityType type )
{
    if (type == DNAQualityType_Sanger ) {
        return (char) ( (val <= 93 ? val : 93) + 33 );
    } else {
        return (char) ( (val <= 62 ? val : 62) + 64 );
    } 
}

QString DNAQuality::getDNAQualityNameByType( DNAQualityType t )
{
    switch(t){
        case DnaQualityType_Solexa:
            return SOLEXA;
        case DNAQualityType_Illumina:
            return ILLUMINA;
        default:
            return SANGER;
    }
}

DNAQualityType DNAQuality::getDNAQualityTypeByName( const QString& name )
{
    if ( name == SOLEXA) {
        return DNAQualityType_Illumina;
    } else if (name == ILLUMINA) {
        return DNAQualityType_Illumina;
    } else {
        return DNAQualityType_Sanger;
    }
}

QStringList DNAQuality::getDNAQualityTypeNames()
{
    QStringList res;
    res << SANGER << ILLUMINA << SOLEXA;
    return res;
}

} // U2
