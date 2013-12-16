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

#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2ObjectDbi.h>

#include <QtCore/QFile>

namespace U2 {

#define DB_ID_OFFSET    0
#define TYPE_OFFSET     8
#define DB_EXTRA_OFFSET 10
#define DATAID_MIN_LEN  10

static U2DataId emptyId;

void U2DbiUtils::logNotSupported(U2DbiFeature f, U2Dbi* dbi, U2OpStatus& os) {
    QString msg = tr("Feature is not supported: %1, dbi: %2").arg(int(f)).arg(dbi == NULL ? QString("<unknown>") : dbi->getDbiId());
    coreLog.error(msg);
    if (!os.hasError()) {
        os.setError(msg);
    }

#ifdef _DEBUG
    if (dbi->getFeatures().contains(f)) {
        coreLog.error("Calling not-supported method while features is listed in supported features list!");
    }
    assert(0);
#endif
}

U2DbiRef U2DbiUtils::toRef(U2Dbi* dbi) {
    if (dbi == NULL) {
        return U2DbiRef();
    }
    return U2DbiRef(dbi->getFactoryId(), dbi->getDbiId());
}

void U2DbiUtils::addLimit(QString& sql, qint64 offset, qint64 count) {
    if (count == -1) {
        return;
    }
    sql = sql + QString(" LIMIT %1, %2").arg(offset).arg(count).toLatin1();
}

U2DataId U2DbiUtils::toU2DataId(qint64 id, U2DataType type, const QByteArray& dbExtra) {
    if (id == 0) {
        return emptyId;
    }
    assert(sizeof(U2DataType) == 2);
    int extraLen = dbExtra.size();
    int len = DATAID_MIN_LEN + extraLen;
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
    QByteArray res(len, Qt::Uninitialized);
#else
    QByteArray res(len, (char)0);
#endif
    char* data = res.data();
    ((qint64*)(data + DB_ID_OFFSET))[0] = id;
    ((U2DataType*)(data + TYPE_OFFSET))[0] = type;
    if (extraLen > 0) {
        memcpy(data + DB_EXTRA_OFFSET, dbExtra.constData(), dbExtra.size());
    }
    return res;
}

quint64 U2DbiUtils::toDbiId(const U2DataId& id) {
    if (id.size() < DATAID_MIN_LEN) {
        return 0;
    }
    return *(qint64*)(id.constData() + DB_ID_OFFSET);
}

U2DataType U2DbiUtils::toType(const U2DataId& id) {
    if (id.size() < DATAID_MIN_LEN) {
        return 0;
    }
    return *(U2DataType*)(id.constData() + TYPE_OFFSET);
}

QByteArray U2DbiUtils::toDbExtra(const U2DataId& id) {
    if (id.size() < DATAID_MIN_LEN) {
        return emptyId;
    }
    return QByteArray(id.constData() + DB_EXTRA_OFFSET, id.length() - DB_EXTRA_OFFSET);
}

QString U2DbiUtils::text(const U2DataId& id) {
    QString res = QString("[Id: %1, Type: %2, Extra: %3]").arg(toDbiId(id)).arg(int(toType(id))).arg(toDbExtra(id).constData());
    return res;
}

//////////////////////////////////////////////////////////////////////////
// DbiConnection
DbiConnection::DbiConnection(const U2DbiRef& ref,  U2OpStatus& os) : dbi(NULL) {
    open(ref, os);    
}

DbiConnection::DbiConnection(const U2DbiRef& ref,  bool create, U2OpStatus& os) : dbi(NULL) {
    open(ref, create, os);
}

DbiConnection::DbiConnection(const DbiConnection& dbiConnection) : dbi(dbiConnection.dbi) {
    if (dbiConnection.dbi != NULL) {
        U2OpStatus2Log os;
        AppContext::getDbiRegistry()->getGlobalDbiPool()->addRef(dbi, os);
    }
}

DbiConnection::~DbiConnection() {
    U2OpStatus2Log os;
    close(os);
}

void DbiConnection::open(const U2DbiRef& ref,  U2OpStatus& os)  {
    open(ref, false, os);
}

void DbiConnection::open(const U2DbiRef& ref,  bool create, U2OpStatus& os)  {
    SAFE_POINT_EXT(!isOpen(), os.setError(QString("Connection is already opened! %1").arg(dbi->getDbiId())), );
    dbi = AppContext::getDbiRegistry()->getGlobalDbiPool()->openDbi(ref, create, os);
}

void DbiConnection::close(U2OpStatus& os) {
    if (dbi != NULL) {
        AppContext::getDbiRegistry()->getGlobalDbiPool()->releaseDbi(dbi, os);
        dbi = NULL;
    }
}

DbiConnection::DbiConnection() : dbi(NULL) {
}

//////////////////////////////////////////////////////////////////////////
// TmpDbiHandle

TmpDbiHandle::TmpDbiHandle() {
}

TmpDbiHandle::TmpDbiHandle(const QString& _alias, U2OpStatus& os)
    : alias(_alias)
{
    dbiRef = AppContext::getDbiRegistry()->attachTmpDbi(alias, os);
}

TmpDbiHandle::TmpDbiHandle(const TmpDbiHandle& dbiHandle) {
    if (dbiHandle.isValid()) {
        alias = dbiHandle.getAlias();
        dbiRef = dbiHandle.getDbiRef();

        U2OpStatus2Log os;
        AppContext::getDbiRegistry()->attachTmpDbi(dbiHandle.getAlias(), os);
    }
}

TmpDbiHandle& TmpDbiHandle::operator=(const TmpDbiHandle& dbiHandle) {
    if (this != &dbiHandle) {
        if (dbiHandle.isValid()) {
            alias = dbiHandle.getAlias();
            dbiRef = dbiHandle.getDbiRef();

            U2OpStatus2Log os;
            AppContext::getDbiRegistry()->attachTmpDbi(dbiHandle.getAlias(), os);
        }
    }

    return *this;
}

TmpDbiHandle::~TmpDbiHandle () {
    if (isValid())
    {
        U2OpStatus2Log os;
        AppContext::getDbiRegistry()->detachTmpDbi(alias, os);
    }
}

//////////////////////////////////////////////////////////////////////////
// TmpDbiObjects
TmpDbiObjects::~TmpDbiObjects() {
    if (os.isCoR()) {
        foreach(const U2DataId& id, objects) {
            if (!id.isEmpty()) {
                U2OpStatus2Log os2log;
                DbiConnection con(dbiRef, os2log);
                con.dbi->getObjectDbi()->removeObject(id, os2log);
            }
        }
    }
}

DbiOperationsBlock::DbiOperationsBlock(const U2DbiRef &_dbiRef, U2OpStatus &os) {
    dbiRef = _dbiRef;
    connection = new DbiConnection(dbiRef, os);
    CHECK_OP(os, );
    connection->dbi->startOperationsBlock(os);
}

DbiOperationsBlock::~DbiOperationsBlock() {
    connection->dbi->stopOperationBlock();
    delete connection;
}


} //namespace
