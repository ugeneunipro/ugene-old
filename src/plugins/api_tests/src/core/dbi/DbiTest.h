#ifndef DBITEST_H
#define DBITEST_H

#include <U2Core/U2Dbi.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/AppContext.h>

#include <U2Test/TestRunnerSettings.h>

#include <gtest/gtest.h>

#include <memory>

#include <QtCore/QDir>


namespace U2 {

class BaseDbiTest : public ::testing::TestWithParam<APITestData> {
public:
    static const QString DATA_PATH;

protected:
    virtual void SetUp() {
        testData = GetParam();

        U2DbiFactory *factory = AppContext::getDbiRegistry()->getDbiFactoryById("SQLiteDbi");
        ASSERT_NE((U2DbiFactory *)NULL, factory);

        dbi.reset(factory->createDbi());
        ASSERT_NE((U2Dbi *)NULL, dbi.get());

        {
            QString originalFile = testData.getValue<QString>(DATA_PATH);
            ASSERT_TRUE(QFileInfo(originalFile).exists());

            /*QFileInfo fi(originalFile);
            QString tmpFile = QDir::temp().absoluteFilePath(fi.fileName());
            if(QFile::exists(tmpFile)) {
                QFile::remove(tmpFile);
            }
            ASSERT_TRUE(QFile::copy(originalFile, tmpFile));*/
            properties["url"] = originalFile;
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
    APITestData testData;
};

const QString BaseDbiTest::DATA_PATH("data_path");

} // namespace U2

#endif // DBITEST_H
