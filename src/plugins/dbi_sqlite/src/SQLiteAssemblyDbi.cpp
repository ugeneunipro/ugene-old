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

#include "SQLiteAssemblyDbi.h"
#include "SQLiteObjectDbi.h"
#include "assembly/SingleTableAssemblyAdapter.h"
#include "assembly/RTreeAssemblyAdapter.h"

#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2SqlHelpers.h>

#include <QtCore/QVarLengthArray>

#include <U2Core/Timer.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>

#include <memory>

namespace U2 {

SQLiteAssemblyDbi::SQLiteAssemblyDbi(SQLiteDbi* dbi) : U2AssemblyDbi(dbi), SQLiteChildDBICommon(dbi) {
}

AssemblyAdapter* SQLiteAssemblyDbi::getAdapter(const U2DataId& assemblyId, U2OpStatus& os) {
    SQLiteQuery q("SELECT elen_method, compression_method FROM Assembly WHERE object = ?1", db, os);
    q.bindDataId(1, assemblyId);
    if (!q.step()) {
        return NULL;
    }
    QString elen = q.getString(0);
    assert(!elen.isEmpty());
    //TODO    QString comp = q.getString(1);


    AssemblyAdapter* res = NULL;
    if (elen == SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_SINGLE_TABLE) {
        res = new SingleTableAssemblyAdapter(dbi, assemblyId, NULL, db, os);
//    } else if (elen == SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_MULTITABLE_4) {
//        res = new MultitableAssemblyAdapter(assemblyId, NULL, QVector<int>() << 50 << 200 << 700 << U2_DBI_NO_LIMIT, db, os);
    } else if (elen == SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_RTREE) {
        res = new RTreeAssemblyAdapter(dbi, assemblyId, NULL, db, os);
    } else {
        os.setError(SQLiteL10N::tr("Unsupported reads storage type: %1").arg(elen));
        return NULL;
    }
    return res;
}


U2Assembly SQLiteAssemblyDbi::getAssemblyObject(const U2DataId& assemblyId, U2OpStatus& os) {
    U2Assembly res(assemblyId, dbi->getDbiId(), 0);
    SQLiteQuery q("SELECT Assembly.reference, Object.name, Object.version FROM Assembly, Object "
                " WHERE Object.id = ?1 AND Assembly.object = Object.id", db, os);
    q.bindDataId(1, assemblyId);
    if (q.step())  {
        res.referenceId = q.getDataId(0, U2Type::Assembly);
        res.visualName = q.getString(1);
        res.version = q.getInt64(2);
        q.ensureDone();
    } 
    return res;
}

qint64 SQLiteAssemblyDbi::countReads(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::countReadsAt -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::countReadsAt");
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    return a->countReads(r, os);
}


U2DbiIterator<U2AssemblyRead>* SQLiteAssemblyDbi::getReads(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::getReadsAt -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::getReadsAt");
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    return a->getReads(r, os);
}

U2DbiIterator<U2AssemblyRead>* SQLiteAssemblyDbi::getReadsByRow(const U2DataId& assemblyId, const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::getReadsAt -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::getReadsAt");
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    return a->getReadsByRow(r, minRow, maxRow, os);
}

qint64 SQLiteAssemblyDbi::getMaxPackedRow(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) {
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    return a->getMaxPackedRow(r, os);
}



quint64 SQLiteAssemblyDbi::getMaxEndPos(const U2DataId& assemblyId, U2OpStatus& os) {
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    return a->getMaxEndPos(os);
}



void SQLiteAssemblyDbi::createAssemblyObject(U2Assembly& assembly, const QString& folder,  U2DbiIterator<U2AssemblyRead>* it, U2OpStatus& os) {
    assembly.id = SQLiteObjectDbi::createObject(U2Type::Assembly, folder, assembly.visualName, db, os);
    if (os.hasError()) {
        return;
    }
    
    SQLiteQuery q("INSERT INTO Assembly(object, reference, elen_method, compression_method) VALUES(?1, ?2, ?3, ?4)", db, os);
    q.bindDataId(1, assembly.id);
    q.bindDataId(2, assembly.referenceId);
    q.bindText(3, dbi->getProperty(SQLITE_DBI_ASSEMBLY_READ_COMPRESSION_METHOD_KEY, SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_SINGLE_TABLE, os));
    q.bindText(4, SQLITE_DBI_ASSEMBLY_READ_COMPRESSION_METHOD_NO_COMPRESSION);
    q.execute();


    std::auto_ptr<AssemblyAdapter> a(getAdapter(assembly.id, os));
    if (os.hasError()) {
        return;
    }

    a->createReadsTables(os);
    if (os.hasError()) {
        return;
    }

    int insertGroupSize = a->getInsertGroupSize();
    if (it != NULL) {
        do {
            QList<U2AssemblyRead>  reads;
            for (int i = 0; i < insertGroupSize && it->hasNext() && !os.hasError(); i++) {
                reads.append(it->next());
            }
            if (!reads.isEmpty()) {
                a->addReads(reads, os);
            }
        } while (it->hasNext() && !os.hasError());
    }

    a->createReadsIndexes(os);
}
    
void SQLiteAssemblyDbi::removeReads(const U2DataId& assemblyId, const QList<U2DataId>& rowIds, U2OpStatus& os){
    SQLiteObjectDbi* objDbi = dbi->getSQLiteObjectDbi();
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    a->removeReads(rowIds, os);
}

void SQLiteAssemblyDbi::addReads(const U2DataId& assemblyId, QList<U2AssemblyRead>& rows, U2OpStatus& os) {
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    a->addReads(rows, os);
}


/**  Packs assembly rows: assigns packedViewRow value for every read in assembly */
void SQLiteAssemblyDbi::pack(const U2DataId& assemblyId, U2OpStatus& os) {
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    a->pack(os);
}

//////////////////////////////////////////////////////////////////////////
// AssemblyAdapter

AssemblyAdapter::AssemblyAdapter(const U2DataId& _assemblyId, const AssemblyCompressor* _compressor, DbRef* _db, U2OpStatus& _os) 
:assemblyId(_assemblyId), compressor(_compressor), db(_db), os(_os)
{
}

} //namespace
