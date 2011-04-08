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

#ifndef _U2_SQLITE_ASSEMBLY_PACK_ALGORITHM_DBI_H_
#define _U2_SQLITE_ASSEMBLY_PACK_ALGORITHM_DBI_H_

#include "SQLiteDbi.h"
#include <U2Core/U2SqlHelpers.h>

namespace U2 {

class PackAlgorithmData {
public:
    PackAlgorithmData() : leftmostPos(0), effectiveLen(0){}

    U2DataId readId;
    qint64   leftmostPos;
    qint64   effectiveLen;
};

class PackAlgorithmAdapter {
public:
    virtual U2DbiIterator<PackAlgorithmData>* selectAllReads(U2OpStatus& os) = 0;
    virtual void assignProw(const U2DataId& readId, qint64 prow, U2OpStatus& os) = 0;
};
    
class AssemblyPackAlgorithm {
public:
    static void pack(PackAlgorithmAdapter& adapter, U2OpStatus& os);
};


} //namespace

#endif
