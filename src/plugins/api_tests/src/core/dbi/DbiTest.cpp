#include "DbiTest.h"

#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/AppContext.h>

#include <QtCore/QDir>


namespace U2 {

const QString BaseDbiTest::DB_URL("db_url");

void BaseDbiTest::SetUp() {
    testData = GetParam();

    U2DbiFactory *factory = AppContext::getDbiRegistry()->getDbiFactoryById("SQLiteDbi");
    ASSERT_NE((U2DbiFactory *)NULL, factory);

    dbi.reset(factory->createDbi());
    ASSERT_NE((U2Dbi *)NULL, dbi.get());

    QString originalFile = testData.getValue<QString>(DB_URL);
    QString tmpFile = QDir::temp().absoluteFilePath(QFileInfo(originalFile).fileName());

    if(QFile::exists(tmpFile)) {
        QFile::remove(tmpFile);
    }

    if (QFile::exists(originalFile)) {
        ASSERT_TRUE(QFile::copy(originalFile, tmpFile));
    } else {
        properties[U2_DBI_OPTION_CREATE] = U2_DBI_VALUE_ON;
    }
    properties["url"] = tmpFile;

    QVariantMap persistentData;
    U2OpStatusImpl opStatus;
    dbi->init(properties, persistentData, opStatus);
    ASSERT_FALSE(opStatus.hasError());
}

void BaseDbiTest::TearDown() {
    U2OpStatusImpl opStatus;
    dbi->shutdown(opStatus);
    ASSERT_FALSE(opStatus.hasError());
    dbi.reset();
}

} //namespace
