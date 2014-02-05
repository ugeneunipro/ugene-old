/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "DNAQuality.h"
#include "DNAChromatogram.h"

namespace U2 {

static const QString SANGER("Sanger");
static const QString ILLUMINA("Illumina 1.3+");
static const QString SOLEXA("Solexa/Illumina 1.0");

const QString DNAQuality::QUAL_FORMAT("PHRED");
const QString DNAQuality::ENCODED("Encoded");

DNAQuality::DNAQuality( const QByteArray& qualScore, DNAQualityType t /* = DNAQualityType_Sanger*/ )
: qualCodes(qualScore), type(t)
{

}

qint64 DNAQuality::memoryHint() const {
    qint64 m = sizeof(*this);
    m += qualCodes.capacity();

    return m;
}

int DNAQuality::getValue( int pos ) const
{
    assert(pos >=0 && pos < qualCodes.count());
    return  type == DNAQualityType_Sanger ? 
        ( (int)qualCodes.at(pos) - 33 ) : ( (int)qualCodes.at(pos) - 64 );
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
