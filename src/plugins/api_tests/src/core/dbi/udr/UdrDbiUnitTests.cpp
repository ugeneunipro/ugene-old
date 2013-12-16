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

#include <U2Core/AppContext.h>
#include <U2Core/UdrSchemaRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/UdrSchemaRegistry.h>
#include <U2Core/UdrDbi.h>

#include "UdrDbiUnitTests.h"

namespace U2 {

namespace {
    const UdrSchemaId TEST_SCHEMA_ID("UnitTest");
    const UdrSchemaId TEST_SCHEMA_ID_2("UnitTest_2");
    const int INT_FIELD = 0;
    const int DOUBLE_FIELD = 1;
    const int STRING_FIELD = 2;
    const int BLOB_FIELD = 3;

    QList<UdrValue> getData(qint64 iv, double dv, const QString &sv) {
        QList<UdrValue> data;
        data << iv;
        data << dv;
        data << sv;
        data << UdrValue();
        return data;
    }

    UdrRecordId writeDataSchema2(QList<QByteArray> data, U2OpStatus &os) {
        UdrDbi *dbi = UdrTestData::getUdrDbi();
        QList<UdrValue> rec; rec<< QString("url") << QString("");
        UdrRecordId id = dbi->addRecord(TEST_SCHEMA_ID_2, rec, os);
        CHECK_OP(os, id);

        int length = 0;
        foreach (const QByteArray &bytes, data) {
            length += bytes.length();
        }

        OutputStream *oStream = dbi->createOutputStream(id, 1, length, os);
        CHECK_OP(os, id);

        foreach (const QByteArray &bytes, data) {
            oStream->write(bytes.constData(), bytes.size(), os);
        }
        delete oStream;

        return id;
    }

    void checkWrittenDataSchema2(const UdrRecordId &id, const QByteArray &srcData, U2OpStatus &os) {
        UdrDbi *dbi = UdrTestData::getUdrDbi();
        QScopedPointer<InputStream> iStream(dbi->createInputStream(id, 1, os));
        CHECK_OP(os, );
        CHECK_EXT(srcData.size() == iStream->available(), os.setError("wrong stream size"), );
        QByteArray dstData(iStream->available(), 0);
        int read = iStream->read(dstData.data(), dstData.size(), os);
        CHECK_OP(os, );
        CHECK_EXT(read == dstData.size(), os.setError("wrong read size"), );
        CHECK_EXT(srcData == dstData, os.setError("wrong data"), );
    }
}

TestDbiProvider UdrTestData::dbiProvider = TestDbiProvider();
const QString & UdrTestData::UDR_DB_URL("udr-dbi.ugenedb");
UdrDbi * UdrTestData::udrDbi = NULL;
U2DataId UdrTestData::id1("");
U2DataId UdrTestData::id2("");
U2DataId UdrTestData::id_2("");
QByteArray UdrTestData::dataSchema2("");

void UdrTestData::init() {
    initTestUdr();
    bool ok = dbiProvider.init(UDR_DB_URL, false);
    SAFE_POINT(ok, "dbi provider failed to initialize",);

    udrDbi = dbiProvider.getDbi()->getUdrDbi();
    SAFE_POINT(NULL != udrDbi, "udr database not loaded",);

    initTestData();
}

void UdrTestData::shutdown() {
    if (udrDbi != NULL) {
        U2OpStatusImpl os;
        dbiProvider.close();
        udrDbi = NULL;
        SAFE_POINT_OP(os, );
    }
}

void UdrTestData::initTestUdr() {
    UdrSchemaRegistry *reg = AppContext::getUdrSchemaRegistry();
    SAFE_POINT(NULL != reg, "NULL reg", );

    if (NULL != reg->getSchemaById(TEST_SCHEMA_ID)){
        return;
    }

    U2OpStatusImpl os;
    { // init test schema
        UdrSchema *schema = new UdrSchema(TEST_SCHEMA_ID);
        schema->addField(UdrSchema::FieldDesc("int", UdrSchema::INTEGER, UdrSchema::INDEXED), os);
        schema->addField(UdrSchema::FieldDesc("double", UdrSchema::DOUBLE), os);
        schema->addField(UdrSchema::FieldDesc("string", UdrSchema::STRING), os);
        schema->addField(UdrSchema::FieldDesc("blob", UdrSchema::BLOB), os);
        SAFE_POINT_OP(os, );

        schema->addMultiIndex(QList<int>() << 0 << 2, os);
        SAFE_POINT_OP(os, );

        reg->registerSchema(schema, os);
        SAFE_POINT_OP(os, );
    }
    { // init test schema 2
        UdrSchema *schema = new UdrSchema(TEST_SCHEMA_ID_2);
        schema->addField(UdrSchema::FieldDesc("url", UdrSchema::STRING), os);
        schema->addField(UdrSchema::FieldDesc("data", UdrSchema::BLOB, UdrSchema::NOT_INDEXED), os);
        SAFE_POINT_OP(os, );

        reg->registerSchema(schema, os);
        SAFE_POINT_OP(os, );
    }
}

void UdrTestData::initTestData() {
    UdrDbi *dbi = UdrTestData::getUdrDbi();
    SAFE_POINT(NULL != dbi, "NULL dbi", );

    U2OpStatusImpl os;
    { // schema 1
        id1 = dbi->addRecord(TEST_SCHEMA_ID, getData(20, 30.0, "test str"), os).recordId;
        SAFE_POINT_OP(os, );
        id2 = dbi->addRecord(TEST_SCHEMA_ID, getData(48, 37.0, "test str 2"), os).recordId;
        SAFE_POINT_OP(os, );
    }

    { // schema 2
        dataSchema2 = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                      "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
                      "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"
                      "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT";
        QList<QByteArray> data; data << dataSchema2;
        id_2 = writeDataSchema2(data, os).recordId;
        SAFE_POINT_OP(os, );
    }
}

UdrDbi * UdrTestData::getUdrDbi() {
    if (udrDbi == NULL) {
        UdrTestData::init();
    }
    return udrDbi;
}

IMPLEMENT_TEST(UdrDbiUnitTests, getRecord) {
    UdrDbi *dbi = UdrTestData::getUdrDbi();
    U2OpStatusImpl os;
    UdrRecord record1 = dbi->getRecord(UdrRecordId(TEST_SCHEMA_ID, UdrTestData::id1), os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(20 == record1.getInt(INT_FIELD, os), "int field 1");
    CHECK_TRUE(30.0 == record1.getDouble(DOUBLE_FIELD, os), "double field 1");
    CHECK_TRUE("test str" == record1.getString(STRING_FIELD, os), "string field 1");
    CHECK_NO_ERROR(os);

    UdrRecord record2 = dbi->getRecord(UdrRecordId(TEST_SCHEMA_ID, UdrTestData::id2), os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(48 == record2.getInt(INT_FIELD, os), "int field 2");
    CHECK_TRUE(37.0 == record2.getDouble(DOUBLE_FIELD, os), "double field 2");
    CHECK_TRUE("test str 2" == record2.getString(STRING_FIELD, os), "string field 2");
    CHECK_NO_ERROR(os);
}

IMPLEMENT_TEST(UdrDbiUnitTests, addRecord_1) {
    U2OpStatusImpl os;
    UdrDbi *dbi = UdrTestData::getUdrDbi();

    QList<UdrValue> data = getData(51, 102.0, "fifty one");

    UdrRecordId id = dbi->addRecord(TEST_SCHEMA_ID, data, os);
    CHECK_NO_ERROR(os);

    UdrRecord record = dbi->getRecord(id, os);
    CHECK_NO_ERROR(os);

    CHECK_TRUE(51 == record.getInt(INT_FIELD, os), "int field");
    CHECK_TRUE(102.0 == record.getDouble(DOUBLE_FIELD, os), "double field");
    CHECK_TRUE("fifty one" == record.getString(STRING_FIELD, os), "string field");
    CHECK_NO_ERROR(os);
}

IMPLEMENT_TEST(UdrDbiUnitTests, addRecord_2) {
    U2OpStatusImpl os;
    UdrDbi *dbi = UdrTestData::getUdrDbi();

    QList<UdrValue> data1 = getData(5, 2.0, "who? I'm");
    QList<UdrValue> data2 = getData(42, 51.0, "UGENE is the best");

    UdrRecordId id1 = dbi->addRecord(TEST_SCHEMA_ID, data1, os);
    CHECK_NO_ERROR(os);
    UdrRecordId id2 = dbi->addRecord(TEST_SCHEMA_ID, data2, os);
    CHECK_NO_ERROR(os);

    UdrRecord record1 = dbi->getRecord(id1, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(5 == record1.getInt(INT_FIELD, os), "int field 1");
    CHECK_TRUE(2.0 == record1.getDouble(DOUBLE_FIELD, os), "double field 1");
    CHECK_TRUE("who? I'm" == record1.getString(STRING_FIELD, os), "string field 1");
    CHECK_NO_ERROR(os);

    UdrRecord record2 = dbi->getRecord(id2, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(42 == record2.getInt(INT_FIELD, os), "int field 2");
    CHECK_TRUE(51.0 == record2.getDouble(DOUBLE_FIELD, os), "double field 2");
    CHECK_TRUE("UGENE is the best" == record2.getString(STRING_FIELD, os), "string field 2");
    CHECK_NO_ERROR(os);
}

IMPLEMENT_TEST(UdrDbiUnitTests, addRecord_with_adapter) {
    U2OpStatusImpl os;
    UdrDbi *dbi = UdrTestData::getUdrDbi();

    QList<UdrValue> data; data << QString("url") << QString("");

    UdrRecordId id = dbi->addRecord(TEST_SCHEMA_ID_2, data, os);
    CHECK_NO_ERROR(os);
    UdrRecord record = dbi->getRecord(id, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE("url" == record.getString(0, os), "url field");
    CHECK_NO_ERROR(os);
}

IMPLEMENT_TEST(UdrDbiUnitTests, removeRecord) {
    U2OpStatusImpl os;
    UdrDbi *dbi = UdrTestData::getUdrDbi();

    UdrRecordId id = dbi->addRecord(TEST_SCHEMA_ID, getData(1, 2, "three"), os);
    CHECK_NO_ERROR(os);

    dbi->getRecord(id, os);
    CHECK_NO_ERROR(os);

    dbi->removeRecord(id, os);
    CHECK_NO_ERROR(os);

    dbi->getRecord(id, os);
    CHECK_TRUE(os.hasError(), "no error");
}

IMPLEMENT_TEST(UdrDbiUnitTests, OutputStream_write) {
    U2OpStatusImpl os;

    QByteArray bytes("test data");
    QList<QByteArray> data; data << bytes;
    UdrRecordId id = writeDataSchema2(data, os);
    CHECK_NO_ERROR(os);

    checkWrittenDataSchema2(id, bytes, os);
    CHECK_NO_ERROR(os);
}

IMPLEMENT_TEST(UdrDbiUnitTests, OutputStream_write_2) {
    U2OpStatusImpl os;

    QByteArray bytes1("test1");
    QByteArray bytes2("test2");
    QList<QByteArray> data; data << bytes1 << bytes2;
    UdrRecordId id = writeDataSchema2(data, os);
    CHECK_NO_ERROR(os);

    checkWrittenDataSchema2(id, bytes1+bytes2, os);
    CHECK_NO_ERROR(os);
}

IMPLEMENT_TEST(UdrDbiUnitTests, InputStream_read) {
    U2OpStatusImpl os;
    UdrDbi *dbi = UdrTestData::getUdrDbi();

    QScopedPointer<InputStream> iStream(dbi->createInputStream(UdrRecordId(TEST_SCHEMA_ID_2, UdrTestData::id_2), 1, os));
    CHECK_NO_ERROR(os);

    QByteArray dstData;
    int read = 0;
    do {
        QByteArray data(23, 0);
        read = iStream->read(data.data(), data.size(), os);
        CHECK_NO_ERROR(os);
        if (read > 0) {
            dstData.append(data.constData(), read);
        }
    } while (-1 != read);
    CHECK_TRUE(UdrTestData::dataSchema2 == dstData, "wrong data");
}

IMPLEMENT_TEST(UdrDbiUnitTests, InputStream_skip_OutOfRange_1) {
    U2OpStatusImpl os;
    UdrDbi *dbi = UdrTestData::getUdrDbi();

    QScopedPointer<InputStream> iStream(dbi->createInputStream(UdrRecordId(TEST_SCHEMA_ID_2, UdrTestData::id_2), 1, os));
    CHECK_NO_ERROR(os);

    qint64 skipped = iStream->skip(UdrTestData::dataSchema2.size() + 1, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(UdrTestData::dataSchema2.size() == skipped, "wrong skipped");

    QByteArray data(200, 0);
    int read = iStream->read(data.data(), data.size(), os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(-1 == read, "wrong read");
}

IMPLEMENT_TEST(UdrDbiUnitTests, InputStream_skip_OutOfRange_2_negative) {
    U2OpStatusImpl os;
    UdrDbi *dbi = UdrTestData::getUdrDbi();

    QScopedPointer<InputStream> iStream(dbi->createInputStream(UdrRecordId(TEST_SCHEMA_ID_2, UdrTestData::id_2), 1, os));
    CHECK_NO_ERROR(os);

    qint64 skipped = iStream->skip(100, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(100 == skipped, "wrong skipped 1");

    skipped = iStream->skip(-200, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(-100 == skipped, "wrong skipped 2");

    QByteArray data(UdrTestData::dataSchema2.size(), 0);
    int read = iStream->read(data.data(), data.size(), os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(200 == read, "wrong read");

    CHECK_TRUE(UdrTestData::dataSchema2 == data, "wrong data");
}

IMPLEMENT_TEST(UdrDbiUnitTests, InputStream_skip_Range_1) {
    U2OpStatusImpl os;
    UdrDbi *dbi = UdrTestData::getUdrDbi();

    QScopedPointer<InputStream> iStream(dbi->createInputStream(UdrRecordId(TEST_SCHEMA_ID_2, UdrTestData::id_2), 1, os));
    CHECK_NO_ERROR(os);

    qint64 skipped = iStream->skip(50, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(50 == skipped, "wrong skipped");

    QByteArray data(200, 0);
    int read = iStream->read(data.data(), data.size(), os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(150 == read, "wrong read");

    CHECK_TRUE(UdrTestData::dataSchema2.mid(50) == data.left(read), "wrong data");
}

IMPLEMENT_TEST(UdrDbiUnitTests, InputStream_skip_Range_2_negative) {
    U2OpStatusImpl os;
    UdrDbi *dbi = UdrTestData::getUdrDbi();

    QScopedPointer<InputStream> iStream(dbi->createInputStream(UdrRecordId(TEST_SCHEMA_ID_2, UdrTestData::id_2), 1, os));
    CHECK_NO_ERROR(os);

    QByteArray data(200, 0);
    int read = iStream->read(data.data(), data.size(), os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(200 == read, "wrong read 1");
    CHECK_TRUE(UdrTestData::dataSchema2 == data, "wrong data 1");

    qint64 skipped = iStream->skip(-100, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(-100 == skipped, "wrong skipped");

    read = iStream->read(data.data(), data.size(), os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(100 == read, "wrong read 2");
    CHECK_TRUE(UdrTestData::dataSchema2.mid(100) == data.left(read), "wrong data 2");
}

} // U2
