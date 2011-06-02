#include "AssemblyDbiTest.h"

#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/AppSettings.h>

#include <U2Test/TestRunnerSettings.h>


namespace U2 {

static const QString& TOTAL_NUM_READS = "total_num_reads";
static const QString& NUM_READS_IN = "num_reads_in";
static const QString& NUM_READS_OUT = "num_reads_out";

static const QString& GET_READS_REGION = "get_reads_region";
static const QString& GET_READS_OUT = "get_reads_out";

static const QString& READS_BY_NAME_IN = "reads_by_name_in";
static const QString& READS_BY_NAME_OUT = "reads_by_name_out";

static const QString& MAX_END_POS = "max_end_pos";

static const QString& COVERAGE_REGION_IN = "calc_coverage_region_in";
static const QString& COVERAGE_REGION_OUT = "calc_coverage_region_out";

APITestData createTestData() {
    APITestData d;

    QString dataDirPath = AppContext::getAppSettings()->getTestRunnerSettings()->getVar("AssemblyDbiTest");
    QDir dataDir(dataDirPath);
    QString dataPath = dataDir.absoluteFilePath("test-alignment1.ugenedb");
    d.addValue(BaseDbiTest::DATA_PATH, dataPath);

    // countReads
    d.addValue(TOTAL_NUM_READS, 48);
    d.addValue(NUM_READS_IN, U2Region(10,10));
    d.addValue(NUM_READS_OUT, 6);

    //getReads
    d.addValue(GET_READS_REGION, U2Region(140, 145));
    QVariantList reads;

    ReadData read1;
    read1.name = "2797 Example sequence FW - secondary sequence 5464";
    read1.leftmostPos = 93;
    read1.effectiveLen = 49;
    read1.packedViewRow = 15;
    read1.readSequence = "AAGATCCTCATGTTATATCGGCAGTGGGTTGATCAATCCACGTGGATAG";
    read1.flags = None;
    
    ReadData read2;
    read2.name = "4940 Example sequence FW - secondary sequence 5466";
    read2.leftmostPos = 95;
    read2.effectiveLen = 49;
    read2.packedViewRow = 0;
    read2.readSequence = "GATCCTCATGTTATATCGGCAGTGGGTTGATCAATCCACGTGGATAGAT";
    read2.flags = None;

    ReadData read3;
    read3.name = "2378 Example sequence RV - secondary sequence 5468";
    read3.leftmostPos = 97;
    read3.effectiveLen = 49;
    read3.packedViewRow = 16;
    read3.readSequence = "TCCTCATGTTATATCGGCAGTGGGTTGATCAATCCACGTGGATAGATGC";
    read3.flags = None;

    reads.append(qVariantFromValue(read1));
    reads.append(qVariantFromValue(read2));
    reads.append(qVariantFromValue(read3));
    
    d.addValue(GET_READS_OUT, reads);

    // getReadsByName
    d.addValue(READS_BY_NAME_IN, QByteArray("146615444"));
    QVariantList readsByName;
    readsByName.append(qVariantFromValue(read1));
    d.addValue(READS_BY_NAME_OUT, readsByName);

    // getMaxEndPos
    d.addValue(MAX_END_POS, 146);

    // calculateCoverage
    d.addValue(COVERAGE_REGION_IN, U2Region(20, 1));
    d.addValue(COVERAGE_REGION_OUT, 7);

    return d;
}

TEST_P(AssemblyDbiTest, getAssemblyObject) {
    U2OpStatusImpl opStatus;
    U2DataId id = this->assemblyIds.first();
    this->assemblyDbi->getAssemblyObject(id, opStatus);
    ASSERT_FALSE(opStatus.hasError()) << opStatus.getError().toStdString();
}

TEST_P(AssemblyDbiTest, countReads) {
    U2OpStatusImpl os;
    U2DataId id = this->assemblyIds.first();

    qint64 numReads = this->assemblyDbi->countReads(id, U2_ASSEMBLY_REGION_MAX, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(testData.getValue<int>(TOTAL_NUM_READS), numReads);

    const U2Region& testRegion = testData.getValue<U2Region>(NUM_READS_IN);
    numReads = this->assemblyDbi->countReads(id, testRegion, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(testData.getValue<int>(NUM_READS_OUT), numReads);
}

bool findRead(const U2AssemblyRead& subj, QVariantList& reads) {
    for (int i=0, n = reads.size(); i<n; i++) {
        const ReadData& curRead = qVariantValue<ReadData>(reads.at(i));
        if (curRead == subj) {
            reads.removeAt(i);
            return true;
        }
    }
    return false;
}

TEST_P(AssemblyDbiTest, getReads) {
    U2OpStatusImpl os;
    const U2Region& region = testData.getValue<U2Region>(GET_READS_REGION);
    const U2DataId& id = this->assemblyIds.first();

    U2DbiIterator<U2AssemblyRead>* iter = this->assemblyDbi->getReads(id, region, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    QVariantList expectedReads = testData.getValue<QVariantList>(GET_READS_OUT);
    while (iter->hasNext()) {
        const U2AssemblyRead& read = iter->next();
        ASSERT_TRUE(findRead(read, expectedReads));
    }
    delete iter;
}

TEST_P(AssemblyDbiTest, getReadsByName) {
    U2OpStatusImpl os;
    const QByteArray& name = testData.getValue<QByteArray>(READS_BY_NAME_IN);
    const U2DataId& id = this->assemblyIds.first();

    U2DbiIterator<U2AssemblyRead>* iter = this->assemblyDbi->getReadsByName(id, name, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    QVariantList expectedReads = testData.getValue<QVariantList>(READS_BY_NAME_OUT);
    while (iter->hasNext()) {
        const U2AssemblyRead& read = iter->next();
        ASSERT_TRUE(findRead(read, expectedReads));
    }
    ASSERT_TRUE(expectedReads.isEmpty());

    delete iter;
}

TEST_P(AssemblyDbiTest, getMaxEndPos) {
    U2OpStatusImpl os;
    qint64 res = this->assemblyDbi->getMaxEndPos(this->assemblyIds.first(), os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(testData.getValue<int>(MAX_END_POS), res);
}

TEST_P(AssemblyDbiTest, calculateCoverage) {
    U2OpStatusImpl os;
    const U2DataId& id = this->assemblyIds.first();
    const U2Region& region = testData.getValue<U2Region>(COVERAGE_REGION_IN);
    U2AssemblyCoverageStat c;
    c.coverage.resize(1);
    this->assemblyDbi->calculateCoverage(id, region, c, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    int res = c.coverage.first().maxValue;
    ASSERT_EQ(testData.getValue<int>(COVERAGE_REGION_OUT), res);
}

INSTANTIATE_TEST_CASE_P(
                        AssemblyDbiTestInstance,
                        AssemblyDbiTest,
                        ::testing::Values(createTestData()));

} // namespace U2
