/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_SNP_H_
#define _U2_SNP_H_

#include <U2Core/U2Type.h>

namespace U2 {

/** 
    Representation for set of SNPs. 
*/
class U2CORE_EXPORT U2SnpTrack : public U2Object {
public:
    U2SnpTrack() {}
    U2SnpTrack(const U2DataId& id, const QString& dbId, qint64 version) : U2Object(id, dbId, version){}
    
    /** Sequence id */
    U2DataId      sequence;
};

/** Single nucleotide polymorphisms database representation */
class U2CORE_EXPORT U2Snp : public U2Entity {
public:
    U2Snp() : pos (0), oldBase(0), newBase(0) {}

    qint64  pos;
    char    oldBase;
    char    newBase;

};

} // namespace

#endif
