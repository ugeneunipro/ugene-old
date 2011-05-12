#include <QtCore/QDir>
#include <QtCore/QFile>
#include "DbiTest.h"

namespace U2 {

// BAMDbiType

const char *BAMDbiType::ID = "BAMDbi";

// FileDbiType

const char *FileDbiType::ID = "FileDbi";

// SQLiteDbiType

const char *SQLiteDbiType::ID = "SQLiteDbi";

// UninitializedDbiTest<BAMDbiType>

void UninitializedDbiTest<BAMDbiType>::SetUp() {
    ASSERT_NO_FATAL_FAILURE(BaseDbiTest<DBI_TYPE>::SetUp());
    {
        /*QString originalName = "";
        fileName = QDir::temp().absoluteFilePath(QFileInfo(originalName).fileName());
        if(QFile::exists(fileName)) {
            QFile::remove(fileName);
        }
        if(QFile::exists(fileName + ".sqlite")) {
            QFile::remove(fileName + ".sqlite");
        }
        ASSERT_TRUE(QFile::copy(originalName, fileName));
        properties["url"] = "file://" + fileName;*/
    }
}

void UninitializedDbiTest<BAMDbiType>::TearDown() {
    /*if(QFile::exists(fileName)) {
        QFile::remove(fileName);
    }
    if(QFile::exists(fileName + ".sqlite")) {
        QFile::remove(fileName + ".sqlite");
    }*/
    EXPECT_NO_FATAL_FAILURE(BaseDbiTest<DBI_TYPE>::TearDown());
}

// UninitializedDbiTest<FileDbiType>

void UninitializedDbiTest<FileDbiType>::SetUp() {
    ASSERT_NO_FATAL_FAILURE(BaseDbiTest<DBI_TYPE>::SetUp());
    {
        /*QString originalName = "";
        fileName = QDir::temp().absoluteFilePath(QFileInfo(originalName).fileName());
        if(QFile::exists(fileName)) {
            QFile::remove(fileName);
        }
        ASSERT_TRUE(QFile::copy(originalName, fileName));
        properties["url"] = "file://" + fileName;*/
    }
}

void UninitializedDbiTest<FileDbiType>::TearDown() {
    /*if(QFile::exists(fileName)) {
        QFile::remove(fileName);
    }*/
    EXPECT_NO_FATAL_FAILURE(BaseDbiTest<DBI_TYPE>::TearDown());
}

// UninitializedDbiTest<SQLiteDbiType>

void UninitializedDbiTest<SQLiteDbiType>::SetUp() {
    ASSERT_NO_FATAL_FAILURE(BaseDbiTest<DBI_TYPE>::SetUp());
    {
        properties["url"] = ":memory:";
        properties["create"] = "1";
        properties["sqlite-assembly-reads-elen-method"] = "50-100-700-U";
    }
}

void UninitializedDbiTest<SQLiteDbiType>::TearDown() {
    EXPECT_NO_FATAL_FAILURE(BaseDbiTest<DBI_TYPE>::TearDown());
}

// InitializedDbiTest<BAMDbiType>

void InitializedDbiTest<BAMDbiType>::SetUp() {
    ASSERT_NO_FATAL_FAILURE(UninitializedDbiTest<DBI_TYPE>::SetUp());
    QVariantMap persistentData;
    {
        U2OpStatusImpl opStatus;
        dbi->init(properties, persistentData, opStatus);
        ASSERT_FALSE(opStatus.hasError());
    }
}

void InitializedDbiTest<BAMDbiType>::TearDown() {
    {
        U2OpStatusImpl opStatus;
        dbi->shutdown(opStatus);
        EXPECT_FALSE(opStatus.hasError());
    }
    EXPECT_NO_FATAL_FAILURE(UninitializedDbiTest<DBI_TYPE>::TearDown());
}

// InitializedDbiTest<FileDbiType>

void InitializedDbiTest<FileDbiType>::SetUp() {
    ASSERT_NO_FATAL_FAILURE(UninitializedDbiTest<DBI_TYPE>::SetUp());
    QVariantMap persistentData;
    {
        U2OpStatusImpl opStatus;
        dbi->init(properties, persistentData, opStatus);
        ASSERT_FALSE(opStatus.hasError());
    }
}

void InitializedDbiTest<FileDbiType>::TearDown() {
    {
        U2OpStatusImpl opStatus;
        dbi->shutdown(opStatus);
        EXPECT_FALSE(opStatus.hasError());
    }
    EXPECT_NO_FATAL_FAILURE(UninitializedDbiTest<DBI_TYPE>::TearDown());
}

// InitializedDbiTest<SQLiteDbiType>

void InitializedDbiTest<SQLiteDbiType>::SetUp() {
    ASSERT_NO_FATAL_FAILURE(UninitializedDbiTest<DBI_TYPE>::SetUp());
    QVariantMap persistentData;
    {
        U2OpStatusImpl opStatus;
        dbi->init(properties, persistentData, opStatus);
        
        ASSERT_FALSE(opStatus.hasError());
    }
}

void InitializedDbiTest<SQLiteDbiType>::TearDown() {
    {
        U2OpStatusImpl opStatus;
        dbi->shutdown(opStatus);
        EXPECT_FALSE(opStatus.hasError());
    }
    EXPECT_NO_FATAL_FAILURE(UninitializedDbiTest<DBI_TYPE>::TearDown());
}

} // namespace U2
