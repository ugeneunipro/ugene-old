#include "AssemblyDbiTest.h"

#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2DbiUtils.h>

#include <U2Test/TestRunnerSettings.h>


namespace U2 {

static const QString& TOTAL_NUM_READS = "total_num_reads";
static const QString& NUM_READS_IN = "num_reads_in";
static const QString& NUM_READS_OUT = "num_reads_out";

static const QString& GET_READS_IN = "get_reads_region";
static const QString& GET_READS_OUT = "get_reads_out";

static const QString& GET_READS_BY_ROW_REGION = "get_reads_by_row_region";
static const QString& GET_READS_BY_ROW_BEGIN = "get_reads_by_row_begin";
static const QString& GET_READS_BY_ROW_END = "get_reads_by_row_end";
static const QString& GET_READS_BY_ROW_OUT = "get_reads_by_row_out";

static const QString& READS_BY_NAME_IN = "reads_by_name_in";
static const QString& READS_BY_NAME_OUT = "reads_by_name_out";

static const QString& MAX_END_POS = "max_end_pos";

static const QString& ADD_READ = "add_read";

static const QString& REMOVE_READS_IN = "remove_reads_in";

static const QString& MAX_PACKED_ROW_IN = "max_packed_row_in";
static const QString& MAX_PACKED_ROW_OUT = "max_packed_row_out";

static const QString& COVERAGE_REGION_IN = "calc_coverage_region_in";
static const QString& COVERAGE_REGION_OUT = "calc_coverage_region_out";

APITestData createTestData() {
    APITestData d;

    QString dataDirPath = AppContext::getAppSettings()->getTestRunnerSettings()->getVar("AssemblyDbiTest");
    QDir dataDir(dataDirPath);
    QString dataPath = dataDir.absoluteFilePath("test-alignment1.ugenedb");
    d.addValue(BaseDbiTest::DATA_PATH, dataPath);

    // countReads
    {
        d.addValue(TOTAL_NUM_READS, 48);
        d.addValue(NUM_READS_IN, U2Region(10,10));
        d.addValue(NUM_READS_OUT, 6);
    }

    //getReads
    {
        d.addValue(GET_READS_IN, U2Region(140, 145));

        U2AssemblyRead read1(new U2AssemblyReadData());
        read1->name = "2797 Example sequence FW - secondary sequence 5464";
        read1->leftmostPos = 93;
        read1->effectiveLen = 49;
        read1->packedViewRow = 15;
        read1->readSequence = "AAGATCCTCATGTTATATCGGCAGTGGGTTGATCAATCCACGTGGATAG";
        read1->cigar.append(U2CigarToken(U2CigarOp_M, 49));
        read1->flags = None;

        U2AssemblyRead read2(new U2AssemblyReadData());
        read2->name = "4940 Example sequence FW - secondary sequence 5466";
        read2->leftmostPos = 95;
        read2->effectiveLen = 49;
        read2->packedViewRow = 0;
        read2->readSequence = "GATCCTCATGTTATATCGGCAGTGGGTTGATCAATCCACGTGGATAGAT";
        read2->cigar.append(U2CigarToken(U2CigarOp_M, 49));
        read2->flags = None;

        U2AssemblyRead read3(new U2AssemblyReadData());
        read3->name = "2378 Example sequence RV - secondary sequence 5468";
        read3->leftmostPos = 97;
        read3->effectiveLen = 49;
        read3->packedViewRow = 16;
        read3->readSequence = "TCCTCATGTTATATCGGCAGTGGGTTGATCAATCCACGTGGATAGATGC";
        read3->cigar.append(U2CigarToken(U2CigarOp_M, 49));
        read3->flags = None;

        QVariantList reads;
        reads << qVariantFromValue(read1) << qVariantFromValue(read2) << qVariantFromValue(read3);
        d.addValue(GET_READS_OUT, reads);
    }

    // getReadsByRow
    {
        U2AssemblyRead read1(new U2AssemblyReadData());
        read1->name = "4924 Example sequence RV -E(46=3|) secondary sequence 5400";
        read1->leftmostPos = 29;
        read1->effectiveLen = 49;
        read1->packedViewRow = 10;
        read1->readSequence = "TGACGCAGCGAGCCGCCTTCATACTTTGTTAGCACGGCTCCACACATAT";
        read1->flags = None;
        read1->cigar.append(U2CigarToken(U2CigarOp_M, 49));

        U2AssemblyRead read2(new U2AssemblyReadData());
        read2->name = "2728 Example sequence FW - secondary sequence 5440";
        read2->leftmostPos = 69;
        read2->effectiveLen = 49;
        read2->packedViewRow = 5;
        read2->readSequence = "CACACATATGGTCCGACTCTAATGAAGATCCTCATGTTATATCGGCAGT";
        read2->flags = None;
        read2->cigar.append(U2CigarToken(U2CigarOp_M, 49));

        QVariantList reads;
        reads << qVariantFromValue(read1) << qVariantFromValue(read2);

        d.addValue(GET_READS_BY_ROW_REGION, U2Region(74, 3));
        d.addValue(GET_READS_BY_ROW_BEGIN, 5);
        d.addValue(GET_READS_BY_ROW_END, 11);
        d.addValue(GET_READS_BY_ROW_OUT, reads);
    }

    // getReadsByName
    {
        U2AssemblyRead read(new U2AssemblyReadData());
        read->name = "2797 Example sequence FW - secondary sequence 5464";
        read->leftmostPos = 93;
        read->effectiveLen = 49;
        read->packedViewRow = 15;
        read->readSequence = "AAGATCCTCATGTTATATCGGCAGTGGGTTGATCAATCCACGTGGATAG";
        read->cigar.append(U2CigarToken(U2CigarOp_M, 49));
        read->flags = None;

        QVariantList readsByName;
        readsByName.append(qVariantFromValue(read));
        d.addValue(READS_BY_NAME_OUT, read->name);
    }

    // getMaxPackedRow
    {
        d.addValue(MAX_PACKED_ROW_IN, U2Region(10,6));
        d.addValue(MAX_PACKED_ROW_OUT, 3);
    }

    // getMaxEndPos
    {
        d.addValue(MAX_END_POS, qint64(146));
    }

    // addReads
    {
        U2AssemblyRead read2add(new U2AssemblyReadData());
        read2add->name = "Test read";
        read2add->leftmostPos = 90;
        read2add->readSequence = "GTTATATCGGCAGTGGGTTGATC";
        read2add->effectiveLen = read2add->readSequence.length();
        read2add->flags = None;
        QVariantList reads2add;
        reads2add.append(qVariantFromValue(read2add));
        d.addValue(ADD_READ, reads2add);
    }

    // removeReads
    {
        d.addValue(REMOVE_READS_IN, U2Region(10, 10));
    }
    
    // calculateCoverage
    {
        d.addValue(COVERAGE_REGION_IN, U2Region(20, 1));
        d.addValue(COVERAGE_REGION_OUT, 7);
    }

    return d;
}

bool compareCigar(const QList<U2CigarToken>& c1, const QList<U2CigarToken>& c2) {
    if (c1.size() != c2.size()) {
        return false;
    }
    for (int i=0; i<c1.size(); i++) {
        U2CigarToken tok1 = c1.at(i);
        U2CigarToken tok2 = c2.at(i);
        if (tok1.count != tok2.count || tok1.op != tok2.op) {
            return false;
        }
    }
    return true;
}

bool compareReads(const U2AssemblyRead& r1, const U2AssemblyRead& r2) {
    if (r1->name != r2->name) {
        return false;
    }
    if (r1->leftmostPos != r2->leftmostPos) {
        return false;
    }
    if (r1->effectiveLen != r2->effectiveLen) {
        return false;
    }
    if (r1->packedViewRow != r2->packedViewRow) {
        return false;
    }
    if (r1->readSequence != r2->readSequence) {
        return false;
    }
    if (r1->quality != r2->quality) {
        return false;
    }
    if (r1->mappingQuality != r2->mappingQuality) {
        return false;
    }
    if (r1->flags != r2->flags) {
        return false;
    }
    if (!compareCigar(r1->cigar, r2->cigar)) {
        return false;
    }
    return true;
}

bool findRead(const U2AssemblyRead& subj, QVariantList& reads) {
    for (qint64 i=0, n = reads.size(); i<n; i++) {
        const U2AssemblyRead& curRead = qVariantValue<U2AssemblyRead>(reads.at(i));
        if (compareReads(subj, curRead)) {
            reads.removeAt(i);
            return true;
        }
    }
    return false;
}

bool containsRead(const U2AssemblyRead& subj, U2DbiIterator<U2AssemblyRead>* iter) {
    while (iter->hasNext()) {
        const U2AssemblyRead& r = iter->next();
        if (compareReads(r, subj)) {
            return true;
        }
    }
    return false;
}

TEST_P(AssemblyDbiTest, getAssemblyObject) {
    U2OpStatusImpl opStatus;
    U2DataId id = this->assemblyIds.first();
    U2Assembly assembly = this->assemblyDbi->getAssemblyObject(id, opStatus);
    ASSERT_FALSE(opStatus.hasError()) << opStatus.getError().toStdString();
}

TEST_P(AssemblyDbiTest, countReads) {
    U2OpStatusImpl os;
    U2DataId id = this->assemblyIds.first();

    qint64 numReads = this->assemblyDbi->countReads(id, U2_ASSEMBLY_REGION_MAX, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(testData.getValue<qint64>(TOTAL_NUM_READS), numReads);

    const U2Region& testRegion = testData.getValue<U2Region>(NUM_READS_IN);
    numReads = this->assemblyDbi->countReads(id, testRegion, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(testData.getValue<qint64>(NUM_READS_OUT), numReads);
}

TEST_P(AssemblyDbiTest, getReads) {
    const U2DataId& id = this->assemblyIds.first();
    const U2Region& region = testData.getValue<U2Region>(GET_READS_IN);
    U2OpStatusImpl os;

    U2DbiIterator<U2AssemblyRead>* iter = this->assemblyDbi->getReads(id, region, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    QVariantList expectedReads = testData.getValue<QVariantList>(GET_READS_OUT);
    while (iter->hasNext()) {
        const U2AssemblyRead& read = iter->next();
        ASSERT_TRUE(findRead(read, expectedReads));
    }
    delete iter;
    ASSERT_TRUE(expectedReads.isEmpty());
}

TEST_P(AssemblyDbiTest, getReadsByRow) {
    const U2DataId& id = this->assemblyIds.first();
    const U2Region& region = testData.getValue<U2Region>(GET_READS_BY_ROW_REGION);
    U2OpStatusImpl os;

    qint64 begin = testData.getValue<qint64>(GET_READS_BY_ROW_BEGIN);
    qint64 end = testData.getValue<qint64>(GET_READS_BY_ROW_END);

    U2DbiIterator<U2AssemblyRead>* iter = this->assemblyDbi->getReadsByRow(id, region, begin, end, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    QVariantList expectedReads = testData.getValue<QVariantList>(GET_READS_BY_ROW_OUT);
    while (iter->hasNext()) {
        U2AssemblyRead actualRead = iter->next();
        ASSERT_TRUE(findRead(actualRead, expectedReads));
    }
    delete iter;
    ASSERT_TRUE(expectedReads.isEmpty());
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

TEST_P(AssemblyDbiTest, getMaxPackedRow) {
    const U2DataId& id = this->assemblyIds.first();
    U2OpStatusImpl os;

    qint64 actual = this->assemblyDbi->getMaxPackedRow(id, testData.getValue<U2Region>(MAX_PACKED_ROW_IN), os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(actual, testData.getValue<qint64>(MAX_PACKED_ROW_OUT));
}

TEST_P(AssemblyDbiTest, getMaxEndPos) {
    U2DataId id = this->assemblyIds.first();
    U2OpStatusImpl os;
    qint64 res = this->assemblyDbi->getMaxEndPos(id, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(testData.getValue<qint64>(MAX_END_POS), res);
}

TEST_P(AssemblyDbiTest, createAssemblyObject) {
    U2Assembly assembly;
    U2AssemblyReadsImportInfo importInfo;
    U2OpStatusImpl os;

    this->assemblyDbi->createAssemblyObject(assembly, "/", NULL, importInfo, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    U2Assembly actual = this->assemblyDbi->getAssemblyObject(assembly.id, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
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

TEST_P(AssemblyDbiTest, addReads) {
    QVariantList reads2add = testData.getValue<QVariantList>(ADD_READ);
    QList<U2AssemblyRead> reads;
    foreach(QVariant var, reads2add) {
        const U2AssemblyRead& read = qVariantValue<U2AssemblyRead>(var);
        reads.append(read);
    }

    BufferedDbiIterator<U2AssemblyRead> it(reads);

    U2OpStatusImpl os;
    const U2DataId& id = this->assemblyIds.first();

    this->assemblyDbi->addReads(id, &it, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    U2DbiIterator<U2AssemblyRead>* iter = this->assemblyDbi->getReads(id, U2_ASSEMBLY_REGION_MAX, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    foreach(QVariant var, reads2add) {
        const U2AssemblyRead& addedRead = qVariantValue<U2AssemblyRead>(var);
        ASSERT_TRUE(containsRead(addedRead, iter)) << "Read was not added";
    }
    delete iter;
}

TEST_P(AssemblyDbiTest, removeReads) {
    U2OpStatusImpl os;
    const U2DataId& id = this->assemblyIds.first();
    const U2Region& region = testData.getValue<U2Region>(REMOVE_READS_IN);

    U2DbiIterator<U2AssemblyRead>* iter = this->assemblyDbi->getReads(id, region, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    QList<U2DataId> readIds;
    while (iter->hasNext()) {
        const U2AssemblyRead& read = iter->next();
        readIds.append(read->id);
    }
    delete iter;

    this->assemblyDbi->removeReads(id, readIds, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    iter = this->assemblyDbi->getReads(id, region, os);
    ASSERT_FALSE(iter->hasNext()) << "Reads were not deleted";
}

INSTANTIATE_TEST_CASE_P(
                        AssemblyDbiTestInstance,
                        AssemblyDbiTest,
                        ::testing::Values(createTestData()));

} // namespace U2
