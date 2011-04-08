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

#include "AssemblyModel.h"

#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2OpStatus.h>

#include <memory>

namespace U2 {

//==============================================================================
// AssemblyModel
//==============================================================================

AssemblyModel::AssemblyModel(const DbiHandle & dbiHandle_) : 
cachedModelLength(NO_VAL), cachedModelHeight(NO_VAL), referenceDbi(0), dbiHandle(dbiHandle_) 
{
}


bool AssemblyModel::isEmpty() const {
    assert(assemblies.isEmpty() == assemblyDbis.isEmpty());
    return assemblies.isEmpty() || assemblyDbis.isEmpty(); 
}

QList<U2AssemblyRead> AssemblyModel::getReadsFromAssembly(int assIdx, const U2Region & r, qint64 minRow, qint64 maxRow, U2OpStatus & os) {
    std::auto_ptr< U2DbiIterator<U2AssemblyRead> > it(assemblyDbis.at(assIdx)->getReadsByRow(assemblies.at(assIdx).id, r, minRow, maxRow, os));
    //TODO: handle error
    assert(!os.hasError());
    return U2DbiUtils::toList(it.get());
}

qint64 AssemblyModel::countReadsInAssembly(int assIdx, const U2Region & r, U2OpStatus & os) {
    return assemblyDbis.at(assIdx)->countReads(assemblies.at(assIdx).id, r, os);
}

qint64 AssemblyModel::getModelLength(U2OpStatus & os) {
    if(NO_VAL == cachedModelLength) {
        //TODO: length must be calculated as length of the reference sequence or
        //if there is no reference, as maximum of all assembly lengths in the model
        qint64 refLen = hasReference() ? reference.length : 0;
        qint64 assLen = assemblyDbis.at(0)->getMaxEndPos(assemblies.at(0).id, os);
        cachedModelLength = qMax(refLen, assLen);
    }
    return cachedModelLength;
}

qint64 AssemblyModel::getModelHeight(U2OpStatus & os) {
    if(NO_VAL == cachedModelHeight) {
        // TODO: get rid of this? Use predefined max value?
        qint64 zeroAsmLen = assemblyDbis.at(0)->getMaxEndPos(assemblies.at(0).id, os); 
        //TODO: model height should be calculated as sum of all assemblies ? 
        //Or consider refactoring to getHeightOfAssembly(int assIdx, ...)
        cachedModelHeight = assemblyDbis.at(0)->getMaxPackedRow(assemblies.at(0).id, U2Region(0, zeroAsmLen), os);
        return cachedModelHeight;
    }
    return cachedModelHeight;
}

void AssemblyModel::addAssembly(U2AssemblyDbi * dbi, const U2Assembly & assm) {
    //TODO: == operator for U2Assembly
    assert(!assemblyDbis.contains(dbi));
    //assert(!assemblies.contains(assm));
    assemblyDbis.push_back(dbi);
    assemblies.push_back(assm);
}

bool AssemblyModel::hasReference() const {
    return (bool)referenceDbi;
}

void AssemblyModel::setReference(U2SequenceDbi * dbi, const U2Sequence & seq) {
    //TODO emit signal ??
    reference = seq;
    referenceDbi = dbi;
}

QByteArray AssemblyModel::getReferenceRegion(const U2Region& region, U2OpStatus& os) {
    return referenceDbi->getSequenceData(reference.id, region, os);
}

} // U2