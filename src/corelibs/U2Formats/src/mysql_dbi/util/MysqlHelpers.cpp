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

#include <QtSql/QSqlError>

#include <U2Core/Log.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SafePoints.h>

#include "MysqlHelpers.h"

namespace U2 {

static U2DataId     emptyId;
static QByteArray   emptyBlob;
static QString      emptyString;

MysqlDbRef::MysqlDbRef() :
    mutex(QMutex::Recursive)
{
}

qint64 MysqlUtils::remove(const QString& table, const QString& field, const U2DataId& id, qint64 expectedRows, MysqlDbRef* db, U2OpStatus& os) {
    CHECK_OP(os, -1);

    static const QString queryString = "DELETE FROM %1 WHERE %2 = :id";
    U2SqlQuery q(queryString.arg(table).arg(field), db, os);
    q.bindDataId("id", id);
    const qint64 changedRows = q.update();
    SAFE_POINT( -1 == expectedRows || changedRows == expectedRows,
                "Unexpected changed row count",
                changedRows );

    return changedRows;
}

bool MysqlUtils::isTableExists(const QString& tableName, MysqlDbRef* db, U2OpStatus& os) {
    static const QString queryString = "show tables like '%1'";
    U2SqlQuery q(queryString.arg(tableName), db, os);
    return q.step();
}

//////////////////////////////////////////////////////////////////////////
// U2SqlQuery

U2SqlQuery::U2SqlQuery(const QString& sql, MysqlDbRef* db, U2OpStatus& _os) :
    db(db),
    os(&_os),
    query(NULL == db ? QSqlDatabase() : db->handle)
{
    QMutexLocker locker(&db->mutex);

    query.setForwardOnly(true);
    query.prepare(sql);
}

U2SqlQuery::U2SqlQuery(const QString& sql, qint64 offset, qint64 count, MysqlDbRef* db, U2OpStatus& _os) :
    db(db),
    os(&_os),
    query(NULL == db ? QSqlDatabase() : db->handle)
{
    QMutexLocker locker(&db->mutex);
    QString sqlString = sql;
    U2DbiUtils::addLimit(sqlString, offset, count);
    query.setForwardOnly(true);
    query.prepare(sqlString);
}

U2SqlQuery::~U2SqlQuery() {
    QMutexLocker locker(&db->mutex);
    query.clear();
}

void U2SqlQuery::setError(const QString& err) const {
    ioLog.trace("SQL: error: " + err + " in query: " + query.lastQuery());
    if (!os->hasError()) {
        os->setError(err);
    }
}

bool U2SqlQuery::step() {
    CHECK(!hasError(), false);

    if (!query.isActive()) {
        execute();
        CHECK(!hasError(), false);
    }

    return query.next();
}

void U2SqlQuery::ensureDone() {
    CHECK(!hasError(), );

    bool done = !step();
    if (U2_UNLIKELY(!done && !hasError())) {
        setError(U2DbiL10n::tooManyResults());
    }
}

int U2SqlQuery::getInt32(int column) const {
    CHECK(!hasError(), -1);

    bool ok = false;
    int result = query.value(column).toInt(&ok);
    CHECK_EXT(ok, setError("Can't convert value to int"), -1);

    return result;
}

qint64 U2SqlQuery::getInt64(int column) const {
    CHECK(!hasError(), -1);

    bool ok = false;
    qint64 result = query.value(column).toLongLong(&ok);
    CHECK_EXT(ok, setError("Can't convert value to int64"), -1);

    return result;
}

double U2SqlQuery::getDouble(int column) const {
    CHECK(!hasError(), -1.0);

    bool ok = false;
    qint64 result = query.value(column).toDouble(&ok);
    CHECK_EXT(ok, setError("Can't convert value to double"), -1.0);

    return result;
}

U2DataId U2SqlQuery::getDataId(int column, U2DataType type, const QByteArray& dbExtra) const {
    CHECK(!hasError(), emptyId);

    if (query.isNull(column)) {
        return emptyId;
    }

    U2DataId result = U2DbiUtils::toU2DataId(getInt64(column), type, dbExtra);

    return result;
}

U2DataId U2SqlQuery::getDataIdExt(int column) const {
    CHECK(!hasError(), emptyId);

    U2DataType type = getInt32(column + 1);
    CHECK(!hasError(), emptyId);
    if (U2_UNLIKELY(type == U2Type::Unknown)) {
        return emptyId;
    }

    QByteArray dbExtra = getBlob(column + 2);
    CHECK(!hasError(), emptyId);

    U2DataId result = U2DbiUtils::toU2DataId(getInt64(column), type, dbExtra);

    return result;
}


U2DataType U2SqlQuery::getDataType(int column) const {
    CHECK(!hasError(), U2Type::Unknown);

    bool ok = false;
    U2DataType result = (U2DataType)query.value(column).toInt(&ok);
    CHECK_EXT(ok, setError("Can't convert value to data type"), U2Type::Unknown);

    return result;
}

QString U2SqlQuery::getString(int column) const {
    CHECK(!hasError(), emptyString);

    return query.value(column).toString();
}

QByteArray U2SqlQuery::getCString(int column) const {
    CHECK(!hasError(), emptyBlob);

    return query.value(column).toByteArray();
}

QByteArray U2SqlQuery::getBlob(int column) const {
    CHECK(!hasError(), emptyBlob);

    return query.value(column).toByteArray();
}

bool U2SqlQuery::getBool(int column) const {
    CHECK(!hasError(), false);

    return query.value(column).toBool();
}

void U2SqlQuery::bindDataId(const QString& placeholder, const U2DataId& val) {
    CHECK(!hasError(), );

    quint64 dbiId = U2DbiUtils::toDbiId(val);
    if (0 != dbiId) {
        query.bindValue(placeholder, dbiId);
    } else {
        bindNull(placeholder);
    }
}

void U2SqlQuery::bindNull(const QString& placeholder) {
    CHECK(!hasError(), );

    query.bindValue(placeholder, QVariant(QVariant::Int));
}

void U2SqlQuery::bindType(const QString& placeholder, U2DataType type) {
    bindInt64(placeholder, type);
}

void U2SqlQuery::bindString(const QString& placeholder, const QString& val) {
    CHECK(!hasError(), );
    query.bindValue(placeholder, val);
}

void U2SqlQuery::bindInt32(const QString& placeholder, qint32 val) {
    CHECK(!hasError(), );
    query.bindValue(placeholder, val);
}

void U2SqlQuery::bindDouble(const QString& placeholder, double val) {
    CHECK(!hasError(), );
    query.bindValue(placeholder, val);
}

void U2SqlQuery::bindInt64(const QString& placeholder, qint64 val) {
    CHECK(!hasError(), );
    query.bindValue(placeholder, val);
}

void U2SqlQuery::bindBool(const QString& placeholder, bool val) {
    CHECK(!hasError(), );
    query.bindValue(placeholder, val);
}

void U2SqlQuery::bindBlob(const QString& placeholder, const QByteArray& blob) {
    CHECK(!hasError(), );
    query.bindValue(placeholder, blob);
}

void U2SqlQuery::bindZeroBlob(const QString& placeholder, int reservedSize) {
    CHECK(!hasError(), );
    QByteArray blob(reservedSize, 0);
    bindBlob(placeholder, blob);
}

void U2SqlQuery::execute() {
    CHECK(!hasError(), );
    QMutexLocker locker(&db->mutex);

    query.exec();
    if (U2_UNLIKELY(query.lastError().isValid())) {
        setError("Error: " + query.lastError().text() +
                 "; Query: '" + query.lastQuery() + "' "
                 + getBoundValues());
    }
}

qint64 U2SqlQuery::update() {
    CHECK(!hasError(), -1);

    execute();
    CHECK(!hasError(), -1);

    return (query.isSelect() ? query.size() : query.numRowsAffected());
}

qint64 U2SqlQuery::insert() {
    CHECK(!hasError(), -1);

    execute();
    CHECK(!hasError(), -1);

    bool ok = false;
    QVariant v = query.lastInsertId();
    qint64 result = v.toLongLong(&ok);

    return result;
}

U2DataId U2SqlQuery::insert(U2DataType type, const QByteArray& dbExtra) {
    CHECK(!hasError(), emptyId);

    qint64 lastRowId = insert();
    CHECK(!hasError(), emptyId);

    return U2DbiUtils::toU2DataId(lastRowId, type, dbExtra);
}

qint64 U2SqlQuery::selectInt64() {
    CHECK(!hasError(), -1);

    execute();
    CHECK(!hasError(), -1);

    if (step()) {
        return getInt64(0);
    }

    return -1;
}

qint64 U2SqlQuery::selectInt64(qint64 defaultValue) {
    CHECK(!hasError(), defaultValue);

    execute();
    CHECK(!hasError(), defaultValue);

    if (step()) {
        return getInt64(0);
    }

    return defaultValue;
}

QList<U2DataId> U2SqlQuery::selectDataIds(U2DataType type, const QByteArray& dbExtra) {
    QList<U2DataId> res;

    execute();
    CHECK(!hasError(), res);

    while(step()) {
        U2DataId id = getDataId(0, type, dbExtra);
        res.append(id);
    }
    return res;
}

QList<U2DataId> U2SqlQuery::selectDataIdsExt() {
    QList<U2DataId> res;

    execute();
    CHECK(!hasError(), res);

    while(step()) {
        U2DataId id = getDataIdExt(0);
        res.append(id);
    }
    return res;
}

QStringList U2SqlQuery::selectStrings() {
    QStringList res;

    execute();
    CHECK(!hasError(), res);

    while (step()) {
        QString text = getString(0);
        res.append(text);
    }
    return res;
}

QString U2SqlQuery::getQueryText() const {
    return query.lastQuery();
}

bool U2SqlQuery::hasError() const {
     return (os != NULL) ? os->hasError() : true;
}

void U2SqlQuery::setOpStatus(U2OpStatus& os) {
    this->os = &os;
}

U2OpStatus& U2SqlQuery::getOpStatus() {
    return *os;
}

const MysqlDbRef* U2SqlQuery::getDb() const {
    return db;
}

QString U2SqlQuery::getBoundValues() const {
    QString result = U2DbiL10n::tr("Bound values: ");
    const QMap<QString, QVariant> boundValues = query.boundValues();
    foreach (const QString placeholder, boundValues.keys()) {
        result += placeholder + " = " + boundValues.value(placeholder).toString() + "; ";
    }
    return result;
}


//////////////////////////////////////////////////////////////////////////
// Mysql transaction helper
MysqlTransaction::MysqlTransaction(MysqlDbRef* db, U2OpStatus& os) :
    db(db),
    os(os),
    started(false)
{
    QMutexLocker locker(&db->mutex);

    if (db->transactionStack.isEmpty()) {
        db->mutex.lock();
        if (!db->handle.transaction()) {
            db->mutex.unlock();
            os.setError(U2DbiL10n::tr("Cannot start a transaction"));
            return;
        }
    }

    db->transactionStack << this;
    started = true;
}

MysqlTransaction::~MysqlTransaction() {
    QMutexLocker locker(&db->mutex);
    CHECK(started, );
    SAFE_POINT(!db->transactionStack.isEmpty(), "Empty transaction stack", );
    SAFE_POINT(db->transactionStack.last() == this, "Wrong transaction in stack", );

    db->transactionStack.pop_back();

    if (db->transactionStack.isEmpty()) {
        if (os.isCoR()) {
            db->handle.rollback();
            db->mutex.unlock();
            return;
        }
        if (!db->handle.commit()) {
            os.setError(db->handle.lastError().text());
        }
        db->mutex.unlock();
    }
}

}   // namespace U2
