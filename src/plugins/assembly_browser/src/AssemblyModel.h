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

#ifndef __U2_ASSEMBLY_BROWSER_MODEL_H__
#define __U2_ASSEMBLY_BROWSER_MODEL_H__

#include <U2Core/U2DbiUtils.h>

namespace U2 {

class AssemblyModel {
public:
    //TODO refactor 
    AssemblyModel(const DbiHandle & dbiHandle);

    bool isEmpty() const;

    QList<U2AssemblyRead> getReadsFromAssembly(int assIdx, const U2Region & r, qint64 minRow, qint64 maxRow, U2OpStatus & os);

    qint64 countReadsInAssembly(int assIdx, const U2Region & r, U2OpStatus & os);

    qint64 getModelLength(U2OpStatus & os);

    qint64 getModelHeight(U2OpStatus & os);

    void addAssembly(U2AssemblyDbi * dbi, const U2Assembly & assm);

    bool hasReference() const;

    void setReference(U2SequenceDbi * dbi, const U2Sequence & seq);

    QByteArray getReferenceRegion(const U2Region& region, U2OpStatus& os);

    const DbiHandle & getDbiHandle() const {return dbiHandle;}

private:
    const static qint64 NO_VAL = -1;
    //TODO: track model changes and invalidate caches accordingly
    qint64 cachedModelLength;
    qint64 cachedModelHeight;

    U2Sequence reference;
    U2SequenceDbi * referenceDbi;

    QList<U2Assembly> assemblies;
    QList<U2AssemblyDbi *> assemblyDbis;

    DbiHandle dbiHandle; 
}; // AssemblyModel

} // U2

#endif // __U2_ASSEMBLY_BROWSER_MODEL_H__
