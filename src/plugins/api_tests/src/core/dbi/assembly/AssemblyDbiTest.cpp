#include <U2Core/U2OpStatusUtils.h>
#include "AssemblyDbiTest.h"

#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/AppSettings.h>

#include <U2Test/GTestFrameworkComponents.h>
#include <U2Test/TestRunnerSettings.h>


namespace U2 {

DbiTestData createTestData() {
    QString dataDirPath = AppContext::getAppSettings()->getTestRunnerSettings()->getVar("AssemblyDbiTest");
    QDir dataDir(dataDirPath);
    DbiTestData d;
    d.filePath = dataDir.absoluteFilePath("example-alignment.bam.ugenedb");
    d.expectedValues["countReads"] = 4971;
    d.expectedValues["getMaxEndPos"] = 9996;

    d.inValues["getReads"] = qVariantFromValue(U2Region(1000, 200));
    d.expectedValues["getReads"] = qVariantFromValue(U2Region(995,31));

    return d;
}

DbiTestData createTestData2() {
    QString dataDirPath = AppContext::getAppSettings()->getTestRunnerSettings()->getVar("AssemblyDbiTest");
    QDir dataDir(dataDirPath);
    DbiTestData d;
    d.filePath = dataDir.absoluteFilePath("c_elegans_test.bam.ugenedb");
    d.expectedValues["countReads"] = 81756;
    d.expectedValues["getMaxEndPos"] = 14874353;

    d.inValues["getReads"] = qVariantFromValue(U2Region(1000, 200));
    d.expectedValues["getReads"] = qVariantFromValue(U2Region(971,30));

    d.inValues["getReadsByRow"] = qVariantFromValue(U2Region());
    d.expectedValues["getReadsByRow"] = qVariantFromValue(U2Region());

    return d;
}

void AssemblyDbiTest::SetUp() {
    ASSERT_NO_FATAL_FAILURE(BaseDbiTest::SetUp());
    
    U2ObjectDbi *objectDbi = this->dbi->getObjectDbi();
    ASSERT_NE((U2ObjectDbi *)NULL, objectDbi);

    U2OpStatusImpl opStatus;
    assemblyIds = objectDbi->getObjects(U2Type::Assembly, 0, U2_DBI_NO_LIMIT, opStatus);
    ASSERT_FALSE(opStatus.hasError());

    assemblyDbi = this->dbi->getAssemblyDbi();
    ASSERT_NE((U2AssemblyDbi *)NULL, this->assemblyDbi);
}

TEST_P(AssemblyDbiTest, getAssemblyObject) {
    U2OpStatusImpl opStatus;
    U2DataId id = this->assemblyIds.first();
    U2Assembly a = this->assemblyDbi->getAssemblyObject(id, opStatus);
    ASSERT_FALSE(opStatus.hasError()) << opStatus.getError().toStdString();
}

TEST_P(AssemblyDbiTest, countReads) {
    U2OpStatusImpl os;
    qint64 res = this->assemblyDbi->countReads(this->assemblyIds.first(), U2_ASSEMBLY_REGION_MAX, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(testData.expectedValues["countReads"], res);
}

TEST_P(AssemblyDbiTest, getMaxEndPos) {
    U2OpStatusImpl os;
    qint64 res = this->assemblyDbi->getMaxEndPos(this->assemblyIds.first(), os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(testData.expectedValues["getMaxEndPos"], res);
}

TEST_P(AssemblyDbiTest, getReads) {
    U2OpStatusImpl os;
    U2Region r = qVariantValue<U2Region>(testData.inValues["getReads"]);
    U2DbiIterator<U2AssemblyRead>* iter = this->assemblyDbi->getReads(this->assemblyIds.first(), r, os);
    U2AssemblyRead read = iter->next();
    U2Region res(read->leftmostPos, read->effectiveLen);
    delete iter;
    U2Region expected = qVariantValue<U2Region>(testData.expectedValues["getReads"]);
    EXPECT_EQ(res, expected);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
}

INSTANTIATE_TEST_CASE_P(
                        AssemblyDbiTestInstance,
                        AssemblyDbiTest,
                        ::testing::Values(createTestData(), createTestData2()));

} // namespace U2
