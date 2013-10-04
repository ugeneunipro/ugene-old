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


#ifndef _U2_KNOWN_MUTATIONS_UTILS_
#define _U2_KNOWN_MUTATIONS_UTILS_

#include <U2Core/KnownMutations.h>

#include <U2Core/U2Type.h>
#include <U2Core/U2OpStatusUtils.h>

namespace U2 {

class U2SequenceDbi;
class KnownMutationsDbi;

class U2CORE_EXPORT KnownMutationsUtils{
public:
    //return -1 if cannot convert
    static int getNumberByChrName(const QString& chrName);    

    static void referenceStatistics (const U2DataId& knownTrackId, KnownMutationsDbi* knownDbi, const U2DataId& seqId, U2SequenceDbi* seqDbi);
};
    
} //namespace

#endif //_U2_KNOWN_MUTATIONS_UTILS_
