#ifndef DBITEST_H
#define DBITEST_H

#include <U2Core/U2Dbi.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/AppContext.h>

#include <gtest/gtest.h>

#include <QtCore/QDir>

#include <memory>


namespace U2 {

class DbiTestData {
public:
    QString filePath;
    // maps function(test) name to expected/incoming value
    QMap<QString, QVariant> expectedValues;
    QMap<QString, QVariant> inValues;
};

class BaseDbiTest : public ::testing::TestWithParam<DbiTestData> {
protected:
    virtual void SetUp() {
        testData = GetParam();

        U2DbiFactory *factory = AppContext::getDbiRegistry()->getDbiFactoryById("SQLiteDbi");
        ASSERT_NE((U2DbiFactory *)NULL, factory);

        dbi.reset(factory->createDbi());
        ASSERT_NE((U2Dbi *)NULL, dbi.get());

        {
            QString originalFile = testData.filePath;
            ASSERT_TRUE(QFileInfo(originalFile).exists());

            QString tmpFile = QDir::temp().absoluteFilePath("example-alignment.bam.ugenedb");
            if(QFile::exists(tmpFile)) {
                QFile::remove(tmpFile);
            }
            ASSERT_TRUE(QFile::copy(originalFile, tmpFile));
            properties["url"] = tmpFile;
        }

        QVariantMap persistentData;
        U2OpStatusImpl opStatus;
        dbi->init(properties, persistentData, opStatus);
        ASSERT_FALSE(opStatus.hasError());
    }

    virtual void TearDown() {
        U2OpStatusImpl opStatus;
        dbi->shutdown(opStatus);
        ASSERT_FALSE(opStatus.hasError());
        dbi.reset();
    }

protected:
    std::auto_ptr<U2Dbi> dbi;
    QHash<QString, QString> properties;
    DbiTestData testData;
};

} // namespace U2

#endif // DBITEST_H
