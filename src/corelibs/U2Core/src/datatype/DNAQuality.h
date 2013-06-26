/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

typedef QString DNAQualityFormat;

class U2CORE_EXPORT DNAQuality {
public:
    DNAQuality() : type (DNAQualityType_Sanger) {}
    DNAQuality(const QByteArray& qualScore, DNAQualityType type = DNAQualityType_Sanger);
    
    bool isEmpty() const { return qualCodes.isEmpty(); }
    int getValue(int pos) const;
    static char encode(int val, DNAQualityType type);
        
    static QString getDNAQualityNameByType(DNAQualityType t);
    static DNAQualityType getDNAQualityTypeByName(const QString& name);
    static QStringList getDNAQualityTypeNames();

    qint64 memoryHint() const;

    QByteArray      qualCodes;
    DNAQualityType  type;
    
    static const DNAQualityFormat QUAL_FORMAT;
    static const DNAQualityFormat ENCODED;
};

}//namespace


#endif
